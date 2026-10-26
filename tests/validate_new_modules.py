#!/usr/bin/env python3
"""Compile actual new listings; test them and measure isolated time/RSS workloads."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import re
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
FILES = [
    'dp/string_dp.tex', 'dp/knapsack_variants.tex', 'dp/digit_dp.tex',
    'techniques/meet_in_the_middle.tex', 'graphs/tree_diameter.tex',
    'graphs/konig_vertex_cover.tex', 'math/xor_basis.tex',
]
LISTING = re.compile(r'\\begin\{lstlisting\}\s*(?:\[[^]]*\])?\s*\n(.*?)\\end\{lstlisting\}', re.S)
HEADER = r'''
#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
using namespace std;
using ll = long long;
using i128 = __int128_t;
'''
# These are measured scenarios, not universal ICPC limits. Official correspondence
# (or the absence of one) is explained in the accompanying benchmark report.
CASES = {
    'lcs_2000': (2.0, 256), 'lcs_5000': (2.0, 256), 'lcs_10000': (2.0, 256),
    'edit_3000': (1.0, 256), 'palindrome_3000': (1.0, 256),
    'unbounded': (2.0, 256), 'exact': (2.0, 256), 'bitset': (2.0, 256),
    'nine_packs_max': (2.0, 256),
    'bitset_many': (2.0, 256),
    'mitm_40': (2.0, 256), 'diameter_chain': (1.0, 256),
    'konig': (2.0, 256), 'digit': (2.0, 256),
    'xor_scalar': (2.0, 256), 'footxor_1500': (0.5, 1024),
}


def blocks(name):
    return LISTING.findall((ROOT / name).read_text())


def run_measured(executable, case, timeout):
    # A separate Python process gives each executable an independent RUSAGE_CHILDREN
    # peak, instead of inheriting the previous benchmark's maximum RSS.
    worker = r'''
import json,resource,subprocess,sys,time
start=time.perf_counter()
try:
 r=subprocess.run([sys.argv[1],sys.argv[2]],capture_output=True,text=True,timeout=float(sys.argv[3]))
 usage=resource.getrusage(resource.RUSAGE_CHILDREN)
 rss=usage.ru_maxrss/(1024*1024 if sys.platform=='darwin' else 1024)
 print(json.dumps(dict(case=sys.argv[2],wall_s=time.perf_counter()-start,
   peak_rss_mib=rss,exit_code=r.returncode,output=r.stdout.strip(),error=r.stderr.strip())))
except subprocess.TimeoutExpired:
 print(json.dumps(dict(case=sys.argv[2],timeout=True,wall_s=time.perf_counter()-start)))
'''
    result = subprocess.run([sys.executable, '-c', worker, str(executable), case, str(timeout)],
                            capture_output=True, text=True, check=True)
    return json.loads(result.stdout)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default='clang++')
    parser.add_argument('--cxxflag', action='append', default=[])
    parser.add_argument('--benchmark', action='store_true')
    parser.add_argument('--repeat', type=int, default=3)
    parser.add_argument('--case', action='append', choices=CASES)
    parser.add_argument('--json-output', type=Path)
    args = parser.parse_args()
    assert args.repeat >= 1
    code = HEADER
    code += next(b for b in blocks('graphs/matching.tex') if 'struct HopcroftKarp {' in b)
    for name in FILES:
        code += '\n'.join(blocks(name)) + '\n'
    code += (ROOT / 'tests/new_modules_test.cpp').read_text()
    results = []
    with tempfile.TemporaryDirectory(prefix='handbook-new-modules-') as tmp:
        source, executable = Path(tmp) / 'test.cpp', Path(tmp) / 'test'
        source.write_text(code)
        subprocess.run([args.compiler, '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
                        *args.cxxflag, str(source), '-o', str(executable)], check=True)
        subprocess.run([str(executable)], check=True)
        if args.benchmark or args.case:
            for case in args.case or CASES:
                limit_s, limit_mib = CASES[case]
                samples = [run_measured(executable, case, max(10.0, 3 * limit_s))
                           for _ in range(args.repeat)]
                result = sorted(samples, key=lambda r: r['wall_s'])[len(samples) // 2]
                result['runs'] = args.repeat
                result['all_runs_succeeded'] = all(
                    not sample.get('timeout') and sample['exit_code'] == 0 for sample in samples)
                if all('peak_rss_mib' in sample for sample in samples):
                    result['peak_rss_mib'] = max(sample['peak_rss_mib'] for sample in samples)
                result.update(time_budget_s=limit_s, memory_budget_mib=limit_mib)
                result['fits_local_budget'] = (
                    result['all_runs_succeeded'] and not result.get('timeout') and result['exit_code'] == 0
                    and result['wall_s'] <= limit_s and result['peak_rss_mib'] <= limit_mib)
                results.append(result)
                print(json.dumps(result), flush=True)
    lines = {name: sum(len(b.splitlines()) for b in blocks(name)) for name in FILES}
    print('C++ listing lines (including comments/blanks):', json.dumps(lines))
    if args.json_output:
        metadata = dict(compiler=args.compiler, cxxflags=args.cxxflag,
                        architecture=platform.machine(), system=platform.system(),
                        generated_source_sha256=hashlib.sha256(code.encode()).hexdigest())
        args.json_output.write_text(json.dumps(dict(metadata=metadata, results=results,
                                                   listing_lines=lines), indent=2) + '\n')


if __name__ == '__main__':
    main()
