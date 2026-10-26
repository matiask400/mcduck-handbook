#!/usr/bin/env python3
"""Compile actual DP/string listings and check reconstruction against independent oracles."""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile
ROOT = Path(__file__).resolve().parents[1]
LISTING = re.compile(r'\\begin\{lstlisting\}\s*(?:\[[^]]*\])?\s*\n(.*?)\\end\{lstlisting\}', re.S)
HEADER = '\n#include <algorithm>\n#include <array>\n#include <bitset>\n#include <cassert>\n#include <chrono>\n#include <cstdint>\n#include <functional>\n#include <iomanip>\n#include <iostream>\n#include <limits>\n#include <numeric>\n#include <optional>\n#include <queue>\n#include <random>\n#include <set>\n#include <sstream>\n#include <string>\n#include <tuple>\n#include <vector>\nusing namespace std;\nusing ll = long long;\nusing i128 = __int128_t;\n\nconst ll LINF = numeric_limits<ll>::max()/4;\n'
def blocks(name):
    return LISTING.findall((ROOT/name).read_text())
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler',default='clang++')
    parser.add_argument('--cxxflag',action='append',default=[])
    args=parser.parse_args()
    code=HEADER
    for name in ('dp/string_dp.tex','dp/knapsack_variants.tex','dp/digit_dp.tex','dp/lis.tex'):
        code+='\n'.join(blocks(name))+'\n'
    for name,recovered in (('knapsack_01','knapsack_with_items'),('hamiltonian_path','hamiltonian_with_vertices')):
        snippet=next(b for b in blocks('dp/knapsack_and_bitmask.tex') if 'll '+name+'(' in b)
        code+=snippet+'\n'+re.sub(r'(?m)^(\s*)// (.*)$',r'\1\2',snippet).replace(name+'(',recovered+'(')+'\n'
    for name in ('strings/hashing.tex','strings/trie.tex','strings/manacher.tex','strings/suffix_array_lcp.tex','strings/aho_corasick.tex'):
        code+=blocks(name)[0]+'\n'
    code+=next(b for b in blocks('strings/prefix_and_z.tex') if 'vector<int> prefix_function(' in b)+'\n'
    code+=next(b for b in blocks('strings/prefix_and_z.tex') if 'vector<int> z_function(' in b)+'\n'
    code+=(ROOT/'tests/dp_strings_test.cpp').read_text()
    with tempfile.TemporaryDirectory(prefix='handbook-dp-strings-') as tmp:
        source,exe=Path(tmp)/'test.cpp',Path(tmp)/'test'
        source.write_text(code)
        subprocess.run([args.compiler,'-std=c++17','-O2','-Wall','-Wextra','-Werror',*args.cxxflag,str(source),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
if __name__=='__main__':
    main()
