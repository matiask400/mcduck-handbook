#!/usr/bin/env python3
"""Audit actual handbook listings without editing them; failures remain visible.

The default suite checks supported inputs. Run --case findings for minimal
counterexamples outside the documented contract; --benchmark for isolated
time/RSS measurements. Unsupported counterexamples exit nonzero deliberately.
"""
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
LISTING = re.compile(r'\\begin\{lstlisting\}\s*(?:\[([^]]*)\])?\s*\n(.*?)\\end\{lstlisting\}', re.S)
HEADER = '''
#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
using namespace std;
using ll = long long;
using i128 = __int128_t;
const ll LINF = numeric_limits<ll>::max()/4;
const int INF = numeric_limits<int>::max()/4;
#define fore(i,a,b) for(int i=(a), audit_end=(b); i<audit_end; ++i)
#define SZ(a) ((int)(a).size())
'''
# Only remove example statements printed after a complete definition; no
# algorithm tokens are changed. Source #line preserves the book's line numbers.
SELECTION = [
 ('data_structures/dsu.tex',0,'\nDSU dsu('),
 *[(f'data_structures/{name}.tex',0,None) for name in (
     'segment_tree','lazy_segment_tree','persistent_segment_tree','sparse_table',
     'disjoint_sparse_table','monotonic_structures','fenwick_2d_offline',
     'wavelet_tree','implicit_treap','cht_li_chao')],
 ('data_structures/fenwick.tex',0,None),('data_structures/fenwick.tex',1,None),
 ('data_structures/disjoint_intervals.tex',0,'\nodt.assign('),
 ('graphs/bridges_articulation.tex',0,'\nLowLink lowlink('),
 ('graphs/biconnected_components.tex',0,None),
 ('graphs/dag_and_scc.tex',0,None),('graphs/dag_and_scc.tex',1,None),
 ('graphs/two_sat.tex',0,None),('graphs/eulerian_path.tex',0,None),
 ('graphs/flows.tex',0,None),('graphs/flows.tex',1,None),('graphs/flows.tex',3,None),
 ('graphs/matching.tex',0,None),('graphs/matching.tex',2,None),
 ('graphs/mst.tex',0,None),('graphs/mst.tex',1,None),
 ('graphs/trees_lca.tex',0,None),
 ('graphs/trees_lca.tex',1,'\nvector<vector<int>> tree(n);'),
 ('graphs/lca_euler_rmq.tex',0,'\nEulerRMQLCA lca('),
 ('graphs/heavy_light_decomposition.tex',0,None),
 ('graphs/centroid_decomposition.tex',0,None),('graphs/rerooting_dp.tex',0,None),
 ('graphs/virtual_tree.tex',0,'\n// Uso'),
 ('graphs/shortest_paths.tex',0,'\n// Ejemplo de uso:'),
 ('graphs/shortest_paths.tex',1,'\nvector<vector<pair<int,int>>> graph('),
 ('graphs/shortest_paths.tex',3,None),('graphs/shortest_paths.tex',5,None),
 ('math/gcd_and_modular.tex',0,None),('math/gcd_and_modular.tex',1,None),
 ('math/gcd_and_modular.tex',2,None),('math/gaussian_elimination.tex',0,None),
 ('math/matrices.tex',0,None),('math/lucas.tex',0,None),('math/ntt.tex',0,None),
 ('math/miller_rabin_pollard_rho.tex',0,None),
 ('math/miller_rabin_pollard_rho.tex',1,None),
 ('math/miller_rabin_pollard_rho.tex',2,None),
 ('geometry/geometry_2d.tex',0,None),('geometry/geometry_2d.tex',2,None),
 *[('geometry/lines_circles_polygons.tex',i,None) for i in (0,2,4,6)],
 ('geometry/geometry_3d.tex',0,None),
 ('techniques/bitmasks_and_bases.tex',0,None),
 ('techniques/bitmasks_and_bases.tex',1,None),
 ('techniques/bitmasks_and_bases.tex',2,None),
 ('techniques/bitmasks_and_bases.tex',3,None),
 ('techniques/mos_algorithm.tex',0,None),
 ('dp/divide_conquer_optimization.tex',0,None),
 ('dp/triangular_bound_dp.tex',0,None),
 ('techniques/prefix_sums.tex',0,None),
 ('techniques/coordinate_compression.tex',0,None),
 ('techniques/sqrt_heavy_light.tex',0,None),
 ('techniques/two_pointers.tex',0,None),
 ('core/parsing.tex',0,None),
 ('strings/suffix_array_lcp.tex',0,None),
 *[('core/math_cpp.tex',i,None) for i in range(12)],
 *[('core/containers.tex',i,None) for i in range(3)],
 *[('core/cpp_language_basics.tex',i,None) for i in range(2)],
 *[('core/cpp_utilities.tex',i,None) for i in range(3)],
 *[('core/stl_algorithms.tex',i,None) for i in range(3)],
 ('core/stl_cheatsheet.tex',0,None),('core/tipos_y_overflow.tex',0,None),
 ('core/input_output.tex',0,None),('core/input_output.tex',1,None),
]

REGRESSIONS = ['odt_empty','matrix_mod_one','floyd_negative_overflow',
               'binary_int_max','binary_wide_domain','divide_conquer_infinite',
               'bcc_loop','dijkstra_large','window_threshold','point_subtraction',
               'base_roundtrip','flow_reverse_index','stl_empty',
               'matching_repeat','dijkstra_limits']
CORRECTNESS = ['structures','graphs','trees','numeric','geometry','techniques','core',*REGRESSIONS]
# This counterexample violates the documented global no-negative-cycle contract.
# Keep it available via --case, outside the supported-input regression suite.
DOCUMENTED_LIMITATIONS = ['mcmf_disconnected_cycle']
FINDINGS = DOCUMENTED_LIMITATIONS
BENCHMARKS = ['bench_structures','bench_scc','bench_ntt','bench_suffix','bench_dijkstra','bench_trees',
              'stack_hld','stack_lowlink','stack_reroot','stack_euler_lca']


def library():
    code = HEADER
    hashes = {}
    for name, index, stop in SELECTION:
        text = (ROOT/name).read_text()
        hashes[name] = hashlib.sha256(text.encode()).hexdigest()
        match = list(LISTING.finditer(text))[index]
        snippet = match[2]
        if stop:
            assert stop in snippet, (name, stop)
            snippet = snippet[:snippet.index(stop)]
        line = text[:match.start(2)].count('\n') + 1
        code += f'\n#line {line} "{ROOT/name}"\n{snippet}\n'
        if name=='graphs/shortest_paths.tex' and index==0:
            recovered = re.sub(r'(?m)^(\s*)// (.*Reconstruccion.*)$',r'\1\2',snippet)
            recovered = recovered.replace('dijkstra(', 'dijkstra_recovered(')
            code += f'\n#line {line} "{ROOT/name}"\n{recovered}\n'
    # Wrap the integer example literally, to test its actual endpoint updates.
    text = (ROOT/'techniques/binary_search.tex').read_text()
    block = list(LISTING.finditer(text))[0][2]
    code += '\nll audit_integer_search(int start,int end,int threshold,bool strict=false) {\n'
    code += 'auto can=[&](int x){return strict?x>threshold:x>=threshold;};\n'
    code += f'#line 16 "{ROOT/"techniques/binary_search.tex"}"\n'+block+'\nreturn l;\n}\n'
    hashes['techniques/binary_search.tex'] = hashlib.sha256(text.encode()).hexdigest()
    return code, hashes


def measure(executable, case, timeout):
    worker = r'''
import json,resource,subprocess,sys,time
def stack_limit():
 resource.setrlimit(resource.RLIMIT_STACK,(8*1024*1024,8*1024*1024))
start=time.perf_counter()
try:
 r=subprocess.run([sys.argv[1],sys.argv[2]],capture_output=True,text=True,
                  timeout=float(sys.argv[3]),preexec_fn=stack_limit)
 rss=resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss
 print(json.dumps(dict(exit_code=r.returncode,wall_s=time.perf_counter()-start,
  peak_rss_mib=rss/(1024**2 if sys.platform=='darwin' else 1024),
  output=r.stdout.strip(),error=r.stderr[-2500:])))
except subprocess.TimeoutExpired:
 print(json.dumps(dict(timeout=True,wall_s=time.perf_counter()-start)))
'''
    r = subprocess.run([sys.executable,'-c',worker,str(executable),case,str(timeout)],
                       capture_output=True,text=True,check=True)
    return dict(case=case,**json.loads(r.stdout))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler',default='clang++')
    parser.add_argument('--cxxflag',action='append',default=[])
    parser.add_argument('--case',action='append')
    parser.add_argument('--benchmark',action='store_true')
    parser.add_argument('--repeat',type=int,default=3)
    parser.add_argument('--json-output',type=Path)
    args = parser.parse_args()
    compiler_version = subprocess.check_output([args.compiler,'--version'],text=True).splitlines()[0]
    warnings = ['-Wall','-Wextra','-Werror']
    if 'clang' in compiler_version.lower():
        # using namespace std; move(x) is valid C++17 and used by the book.
        warnings.append('-Wno-unqualified-std-cast-call')
    source, hashes = library()
    source += '\n#line 1 "audit_book_test.cpp"\n'+(ROOT/'tests/audit_book_test.cpp').read_text()
    cases = args.case or (BENCHMARKS if args.benchmark else CORRECTNESS)
    cases = [c for group in cases for c in
             (CORRECTNESS if group=='correctness' else FINDINGS if group=='findings' else [group])]
    result = []
    with tempfile.TemporaryDirectory(prefix='handbook-audit-') as tmp:
        path, exe = Path(tmp)/'audit.cpp',Path(tmp)/'audit'
        path.write_text(source)
        subprocess.run([args.compiler,'-std=c++17','-O2',*warnings,
                        *args.cxxflag,str(path),'-o',str(exe)],check=True)
        for case in cases:
            samples = [measure(exe,case,30) for _ in range(args.repeat if args.benchmark else 1)]
            row = sorted(samples,key=lambda r:r['wall_s'])[len(samples)//2].copy()
            if all('peak_rss_mib' in r for r in samples):
                row['peak_rss_mib'] = max(r['peak_rss_mib'] for r in samples)
            row['runs'] = samples
            row['all_passed'] = all(r.get('exit_code')==0 for r in samples)
            row['expected_finding'] = case in FINDINGS
            result.append(row)
            print(json.dumps({k:v for k,v in row.items() if k!='runs'}),flush=True)
        if not args.benchmark and not args.case:
            # The team's template is also tested literally with its GNU header.
            template = list(LISTING.finditer((ROOT/'core/template_cpp.tex').read_text()))[0][2]
            # Clang lacks bits/stdc++.h locally; replacing that header by standard
            # headers does not change any declarations being checked.
            path.write_text(template.replace('#include <bits/stdc++.h>',HEADER.split('using namespace std;')[0]))
            r = subprocess.run([args.compiler,'-std=c++17','-fsyntax-only',*args.cxxflag,str(path)],capture_output=True,text=True)
            result.append(dict(case='template_cpp',exit_code=r.returncode,all_passed=r.returncode==0,error=r.stderr))
            print(json.dumps(result[-1]),flush=True)
    report = dict(platform=platform.platform(),compiler=compiler_version,cxxflags=args.cxxflag,
                  stack_mib=8,seed=20261026,source_sha256=hashlib.sha256(source.encode()).hexdigest(),
                  file_sha256=hashes,results=result,
                  documented_limitations=DOCUMENTED_LIMITATIONS)
    if args.json_output:
        args.json_output.write_text(json.dumps(report,indent=2)+'\n')
    sys.exit(0 if all(r['all_passed'] for r in result) else 1)


if __name__=='__main__':
    main()
