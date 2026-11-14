#!/usr/bin/env python3
"""Verify SA/LCP byte compatibility, SCC ordering, and all 2-SAT helpers.

Optional before/after benchmarks use an immutable Git revision of the actual
listings. No third-party checkout or edits to the book are needed.
"""
import argparse
import hashlib
import json
from pathlib import Path
import statistics
import subprocess
import sys
import tempfile
from audit_book import HEADER, LISTING, ROOT

FILES=[('strings/suffix_array_lcp.tex',0),('graphs/dag_and_scc.tex',1),('graphs/two_sat.tex',0)]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--compiler',default='clang++')
    p.add_argument('--cxxflag',action='append',default=[])
    p.add_argument('--baseline-ref',default='e068fdf')
    p.add_argument('--benchmark',action='store_true')
    p.add_argument('--repeat',type=int,default=5)
    p.add_argument('--json-output',type=Path)
    a=p.parse_args();code=HEADER;lines={};hashes={}
    for version in ('before','after'):
        code+=f'\nnamespace {version} {{\n';lines[version]={}
        for name,index in FILES:
            raw=(subprocess.check_output(['git','show',a.baseline_ref+':'+name],cwd=ROOT,text=True)
                 if version=='before' else (ROOT/name).read_text())
            snippet=list(LISTING.finditer(raw))[index][2]
            code+=snippet+'\n'
            clean=__import__('re').sub(r'//[^\n]*','',snippet)
            lines[version][name]=sum(bool(x.strip()) for x in clean.splitlines())
            if version=='after':hashes[name]=hashlib.sha256(raw.encode()).hexdigest()
        code+='}\n'
    code+=(ROOT/'tests/sa_tarjan_test.cpp').read_text()
    report=dict(baseline_ref=a.baseline_ref,flags=['-std=c++17','-O2',*a.cxxflag],
                active_code_lines=lines,after_sha256=hashes,benchmarks=[])
    worker=r'''
import json,resource,subprocess,sys
r=subprocess.run(sys.argv[1:],capture_output=True,text=True,timeout=30,check=True)
data=json.loads(r.stdout)
data['peak_rss_mib']=resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss/(1024**2 if sys.platform=='darwin' else 1024)
print(json.dumps(data))
'''
    with tempfile.TemporaryDirectory(prefix='handbook-sa-tarjan-') as tmp:
        cpp,exe=Path(tmp)/'test.cpp',Path(tmp)/'test';cpp.write_text(code)
        subprocess.run([a.compiler,*report['flags'],str(cpp),'-o',str(exe)],check=True)
        r=subprocess.run([str(exe)],text=True,capture_output=True,check=True,timeout=60)
        print(r.stdout.strip(),flush=True);report['correctness']=r.stdout.strip()
        if a.benchmark:
            for case in ('suffix_random','suffix_repeated','scc_dag','scc_cycles'):
                samples={'before':[],'after':[]}
                for i in range(a.repeat):
                    for version in (('before','after') if i%2==0 else ('after','before')):
                        r=subprocess.run([sys.executable,'-c',worker,str(exe),case,version],text=True,capture_output=True,check=True)
                        samples[version].append(json.loads(r.stdout))
                row=dict(case=case,samples=samples)
                for version,values in samples.items():
                    row[version]=dict(median_ms=statistics.median(x['ms'] for x in values),
                                      peak_rss_mib=max(x['peak_rss_mib'] for x in values))
                assert len({x['checksum'] for values in samples.values() for x in values})==1
                report['benchmarks'].append(row)
                print(json.dumps({k:v for k,v in row.items() if k!='samples'}),flush=True)
    if a.json_output:a.json_output.write_text(json.dumps(report,indent=2)+'\n')
    print('Active C++ lines:',json.dumps(lines))


if __name__=='__main__':main()
