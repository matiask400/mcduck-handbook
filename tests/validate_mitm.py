#!/usr/bin/env python3
"""Validate the actual MITM listing and optionally measure n=40 time and peak RSS."""
import argparse
import json
from pathlib import Path
import platform
import re
import statistics
import subprocess
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
HEADER='\n#include <algorithm>\n#include <array>\n#include <bitset>\n#include <cassert>\n#include <chrono>\n#include <cstdint>\n#include <functional>\n#include <iomanip>\n#include <iostream>\n#include <limits>\n#include <numeric>\n#include <optional>\n#include <queue>\n#include <random>\n#include <set>\n#include <sstream>\n#include <string>\n#include <tuple>\n#include <vector>\nusing namespace std;\nusing ll = long long;\nusing i128 = __int128_t;\n'
WORKER=r"""
import json,resource,subprocess,sys,time
start=time.perf_counter()
r=subprocess.run([sys.argv[1],'benchmark'],capture_output=True,text=True,check=True)
rss=resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss
print(json.dumps(dict(wall_s=time.perf_counter()-start,
    peak_rss_mib=rss/(1024*1024 if sys.platform=='darwin' else 1024),output=r.stdout.strip())))
"""
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler',default='clang++')
    parser.add_argument('--cxxflag',action='append',default=[])
    parser.add_argument('--benchmark',action='store_true')
    parser.add_argument('--json-output',type=Path)
    args=parser.parse_args()
    tex=(ROOT/'techniques/meet_in_the_middle.tex').read_text()
    blocks=re.findall(r'\\begin\{lstlisting\}\s*(?:\[[^]]*\])?\s*\n(.*?)\\end\{lstlisting\}',tex,re.S)
    code=HEADER+'\n'.join(blocks)+(ROOT/'tests/mitm_test.cpp').read_text()
    with tempfile.TemporaryDirectory(prefix='handbook-mitm-') as tmp:
        source,exe=Path(tmp)/'test.cpp',Path(tmp)/'test';source.write_text(code)
        subprocess.run([args.compiler,'-std=c++17','-O2','-Wall','-Wextra','-Werror',*args.cxxflag,str(source),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
        if args.benchmark:
            samples=[]
            for _ in range(3):
                result=subprocess.check_output([sys.executable,'-c',WORKER,str(exe)],text=True)
                samples.append(json.loads(result))
            report=dict(compiler=args.compiler,cxxflags=args.cxxflag,architecture=platform.machine(),
                system=platform.system(),n=40,runs=3,median_wall_s=statistics.median(x['wall_s'] for x in samples),
                max_peak_rss_mib=max(x['peak_rss_mib'] for x in samples),samples=samples)
            print(json.dumps(report,indent=2))
            if args.json_output:args.json_output.write_text(json.dumps(report,indent=2)+'\n')
if __name__=='__main__':main()
