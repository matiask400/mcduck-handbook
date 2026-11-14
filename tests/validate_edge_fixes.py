#!/usr/bin/env python3
"""Validate actual handbook edge-case fixes; benchmark against a pinned baseline."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import re
import statistics
import subprocess
import sys
import tempfile
from audit_book import HEADER, LISTING, ROOT

FILES = {
    'odt': ('data_structures/disjoint_intervals.tex', '\nodt.assign('),
    'matrix': ('math/matrices.tex', None),
    'divide': ('dp/divide_conquer_optimization.tex', None),
    'hld': ('graphs/heavy_light_decomposition.tex', None),
    'lowlink': ('graphs/bridges_articulation.tex', '\nLowLink lowlink('),
    'reroot': ('graphs/rerooting_dp.tex', None),
    'euler': ('graphs/lca_euler_rmq.tex', '\nEulerRMQLCA lca('),
    'floyd': ('graphs/shortest_paths.tex', None),
    'binary': ('techniques/binary_search.tex', None),
}


def source(baseline):
    code=HEADER;metrics={};hashes={}
    for version in ['before','after']:
        code+='\nnamespace '+version+' {\n'
        for name,(path,stop) in FILES.items():
            raw=subprocess.check_output(['git','show',baseline+':'+path],cwd=ROOT,text=True) if version=='before' else (ROOT/path).read_text()
            if version=='after':hashes[path]=hashlib.sha256(raw.encode()).hexdigest()
            blocks=list(LISTING.finditer(raw))
            snippet=next(b[2] for b in blocks if 'void floyd_warshall(' in b[2]) if name=='floyd' else blocks[0][2]
            if stop:snippet=snippet[:snippet.index(stop)]
            clean=re.sub(r'/\*.*?\*/|//[^\n]*','',snippet,flags=re.S)
            metrics.setdefault(name,{})[version]=sum(bool(l.strip()) for l in clean.splitlines())
            if name=='binary':
                snippet='ll integer_search(int start,int end,ll threshold) {\nauto can=[&](int x){return x>=threshold;};\n'+snippet+'\nreturn l;\n}\n'
            code+=snippet+'\n'
        code+='}\n'
    code+=(ROOT/'tests/edge_fixes_test.cpp').read_text()
    return code,dict(baseline_ref=baseline,active_lines=metrics,file_sha256=hashes,
                     generated_cpp_sha256=hashlib.sha256(code.encode()).hexdigest())


WORKER=r'''
import json,resource,subprocess,sys
def limits():
 resource.setrlimit(resource.RLIMIT_STACK,(8*1024*1024,8*1024*1024))
 resource.setrlimit(resource.RLIMIT_CORE,(0,0))
try:
 r=subprocess.run(sys.argv[1:],capture_output=True,text=True,preexec_fn=limits,timeout=90)
 result=dict(exit_code=r.returncode,output=r.stdout.strip(),error=r.stderr[-1800:])
 if r.returncode==0 and r.stdout.startswith('{'):result.update(json.loads(r.stdout))
except subprocess.TimeoutExpired:result=dict(timeout=True)
result['peak_rss_mib']=resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss/(1024**2 if sys.platform=='darwin' else 1024)
print(json.dumps(result))
'''


def run(exe,*args):
    r=subprocess.run([sys.executable,'-c',WORKER,str(exe),*args],capture_output=True,text=True,check=True)
    return json.loads(r.stdout)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--compiler',default='clang++')
    p.add_argument('--cxxflag',action='append',default=[])
    p.add_argument('--baseline-ref',default='3a4aa53')
    p.add_argument('--benchmark',action='store_true')
    p.add_argument('--deep',action='store_true')
    p.add_argument('--repeat',type=int,default=5)
    p.add_argument('--json-output',type=Path)
    args=p.parse_args();code,report=source(args.baseline_ref)
    version=subprocess.check_output([args.compiler,'--version'],text=True).splitlines()[0]
    flags=['-std=c++17','-O2',*args.cxxflag]
    if 'clang' in version.lower():flags+=['-Wno-unqualified-std-cast-call']
    report.update(compiler=version,platform=platform.platform(),flags=flags,seed=20261028,stack_mib=8,benchmarks=[])
    with tempfile.TemporaryDirectory(prefix='handbook-edge-fixes-') as tmp:
        cpp,exe=Path(tmp)/'test.cpp',Path(tmp)/'test';cpp.write_text(code)
        subprocess.run([args.compiler,*flags,str(cpp),'-o',str(exe)],check=True)
        report['correctness']=run(exe);print(json.dumps(report['correctness']),flush=True)
        if report['correctness'].get('exit_code')!=0:raise RuntimeError(report['correctness'])
        if args.deep:
            report['deep_chains']={}
            for case in ['hld','lowlink','reroot','euler']:
                result=run(exe,case,'after','chain');report['deep_chains'][case]=result
                print(case,'chain',json.dumps(result),flush=True)
                if result.get('exit_code')!=0:raise RuntimeError(result)
        if args.benchmark:
            cases=[(name,'random') for name in FILES]+[(name,shape) for name in ['hld','lowlink','reroot','euler'] for shape in ['star','chain']]
            for name,shape in cases:
                samples={'before':[],'after':[]}
                for i in range(args.repeat):
                    for side in (['before','after'] if i%2==0 else ['after','before']):
                        # A reproduced old crash needs only one sample.
                        if samples[side] and samples[side][-1].get('exit_code')!=0:continue
                        samples[side].append(run(exe,name,side,shape))
                row=dict(case=name,shape=shape,samples=samples)
                for side,runs in samples.items():
                    good=all(r.get('exit_code')==0 for r in runs)
                    row[side]=dict(passed=good,peak_rss_mib=max(r['peak_rss_mib'] for r in runs))
                    if good:row[side]['ms']=statistics.median(r['ms'] for r in runs)
                if not row['after']['passed']:raise RuntimeError(row)
                if row['before']['passed']:assert len({r['checksum'] for runs in samples.values() for r in runs})==1
                report['benchmarks'].append(row)
                print(json.dumps({k:v for k,v in row.items() if k!='samples'}),flush=True)
    if args.json_output:args.json_output.write_text(json.dumps(report,indent=2)+'\n')


if __name__=='__main__':main()
