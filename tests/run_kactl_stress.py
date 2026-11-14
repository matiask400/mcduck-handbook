#!/usr/bin/env python3
"""Run every original KACTL stress .cpp, unchanged, with bounded per-test time.

Matches doc/scripts/run-all.sh flags, while using unique executables and
portable timing. Does not claim that tests which time out or fail have passed.
"""
import argparse
import concurrent.futures
import hashlib
import json
import os
from pathlib import Path
import platform
import resource
import subprocess
import tempfile
import time


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('kactl',type=Path)
    p.add_argument('--compiler',default='g++')
    p.add_argument('--cxxflag',action='append',default=[])
    p.add_argument('--timeout',type=int,default=180)
    p.add_argument('--jobs',type=int,default=2)
    p.add_argument('--json-output',type=Path,required=True)
    p.add_argument('--test',action='append',help='Explicit relative filenames; default all .cpp')
    a=p.parse_args();root=a.kactl.resolve()
    flags=['-Wall','-Wfatal-errors','-Wconversion','-std=c++17','-O2',*a.cxxflag]
    # Darwin refuses raising the main-thread stack in this environment. Keep
    # the actual inherited limit explicit instead of pretending CI's 512 MiB.
    stack=resource.getrlimit(resource.RLIMIT_STACK)[0]
    paths=sorted((root/'stress-tests').rglob('*.cpp'))
    if a.test:paths=[root/x for x in a.test]
    report=dict(kactl_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip(),
                compiler=subprocess.check_output([a.compiler,'--version'],text=True).splitlines()[0],
                flags=flags,platform=platform.platform(),stack_bytes=stack,
                official_stack_bytes=512*1024**2,timeout_per_test_s=a.timeout,
                total=len(paths),results=[])
    a.json_output.parent.mkdir(parents=True,exist_ok=True)
    def save():a.json_output.write_text(json.dumps(report,indent=2)+'\n')
    with tempfile.TemporaryDirectory(prefix='kactl-stress-bin-') as tmp:
        def one(item):
            i,path=item;name=str(path.relative_to(root));exe=Path(tmp)/str(i)
            row=dict(test=name,source_sha256=hashlib.sha256(path.read_bytes()).hexdigest())
            t=time.perf_counter()
            try:
                c=subprocess.run([a.compiler,*flags,str(path),'-o',str(exe)],cwd=root,
                                 capture_output=True,text=True,timeout=90)
                row.update(compile_s=time.perf_counter()-t,compile_exit=c.returncode,
                           compiler_output=c.stderr[-6000:])
                if c.returncode:row['status']='compile_failed';return row
                t=time.perf_counter()
                r=subprocess.run([str(exe)],cwd=root,capture_output=True,text=True,
                                 timeout=a.timeout)
                row.update(run_s=time.perf_counter()-t,run_exit=r.returncode,
                           output=r.stdout[-6000:],error=r.stderr[-4000:],
                           status=('skipped' if 'Skipped.' in r.stdout+r.stderr else 'passed')
                           if r.returncode==0 else 'failed')
            except subprocess.TimeoutExpired as e:
                row.update(status='timeout',elapsed_s=time.perf_counter()-t,
                           output=(e.stdout or b'')[-3000:].decode(errors='replace') if isinstance(e.stdout,bytes) else e.stdout)
            return row
        with concurrent.futures.ThreadPoolExecutor(max_workers=a.jobs) as pool:
            futures=[pool.submit(one,x) for x in enumerate(paths)]
            for future in concurrent.futures.as_completed(futures):
                row=future.result();report['results'].append(row);save()
                print(json.dumps({k:row[k] for k in ('test','status','compile_s','run_s') if k in row}),flush=True)
    report['summary']={s:sum(x['status']==s for x in report['results'])
                       for s in ('passed','skipped','failed','compile_failed','timeout')}
    report['all_original_stress_tests_selected']=not a.test
    save();print(json.dumps(report['summary']),flush=True)
    return 0 if all(r['status']=='passed' for r in report['results']) else 1


if __name__=='__main__':raise SystemExit(main())
