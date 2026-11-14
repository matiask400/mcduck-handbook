#!/usr/bin/env python3
"""Check main inputs, actual Python examples, and optional GNU PBDS listing."""
import argparse
import ast
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile
from audit_book import ROOT, LISTING, HEADER


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler',default='g++')
    parser.add_argument('--cxxflag',action='append',default=[])
    parser.add_argument('--with-pbds',action='store_true')
    parser.add_argument('--json-output',type=Path)
    args=parser.parse_args()
    files=[]
    def visit(path,parents):
        assert path not in parents, f'Input cycle: {path}'
        files.append(path)
        for name in re.findall(r'(?m)^[^%\n]*?\\input\{([^}]+)\}',path.read_text()):
            child=(ROOT/name).with_suffix('.tex')
            assert child.exists(),f'Missing input: {child}'
            visit(child,parents+[path])
    visit(ROOT/'main.tex',[])
    missing=sorted(str(p.relative_to(ROOT)) for p in ROOT.glob('*/*.tex')
                   if p.parent.name!='config' and p not in files)
    assert not missing,f'Unreferenced book modules: {missing}'
    python=[]
    for path in files:
        for match in LISTING.finditer(path.read_text()):
            options=match[1] or ''
            if 'style=py' in options or 'language=Python' in options:
                ast.parse(match[2],filename=str(path))
                python.append((path,match[2]))
    template=next(code for path,code in python if path.name=='template_python.tex')
    for input_text,expected in [('0\n\n','0\n'),('4\n-5 0 2 3\n','0\n'),
                                ('2\n9223372036854775807 1\n','9223372036854775808\n')]:
        r=subprocess.run([sys.executable,'-c',template],input=input_text,
                         capture_output=True,text=True,check=True,timeout=5)
        assert r.stdout==expected,(r.stdout,expected)
    extensions='\n'.join(code for path,code in python if path.name=='extension_python.tex')
    r=subprocess.run([sys.executable,'-c',extensions],input='0\n\n',capture_output=True,
                     text=True,check=True,timeout=5)
    expected=['3','[(2, 3)]','[4, 7]','1','1','4','55','2','5','1','4','4',
              '6','24','4','True',format(0xABCDEF*0x123456,'X')]
    assert r.stdout.splitlines()==expected,r.stdout
    letters=next(code for path,code in python if path.name=='char_pos.tex')
    subprocess.run([sys.executable,'-c',letters+'\nassert (zero_indexed,one_indexed,lower_indexed)==(2,3,2)'],check=True)
    report=dict(main_inputs=len(files),python_blocks_syntax_checked=len(python),
                python_template_cases=3,python_extension_examples='passed',
                file_sha256={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in files})
    if args.with_pbds:
        code=next(LISTING.finditer((ROOT/'data_structures/pbds.tex').read_text()))[2]
        code+='\n'+next(LISTING.finditer((ROOT/'core/char_pos.tex').read_text()))[2]
        code+=r'''
int main() {
    assert(zero_indexed==2&&one_indexed==3&&lower_indexed==2);
    OrderedMultiset tree;multiset<pair<int,int>> slow;
    mt19937 rng(20261026);
    assert(tree.find_by_order(0)==tree.end());
    for(int i=0;i<10000;++i) {
        pair<int,int> key{int(rng()%31)-15,i};
        tree.insert(key);slow.insert(key);
        if(i%3==0) {tree.erase(*slow.begin());slow.erase(slow.begin());}
        if(slow.empty()) {assert(tree.empty());continue;}
        pair<int,int> query{int(rng()%31)-15,-1};
        assert(tree.order_of_key(query)==(size_t)distance(slow.begin(),slow.lower_bound(query)));
        int k=rng()%slow.size();auto it=slow.begin();advance(it,k);
        assert(*tree.find_by_order(k)==*it);
    }
}
'''
        with tempfile.TemporaryDirectory(prefix='handbook-pbds-') as tmp:
            path,exe=Path(tmp)/'pbds.cpp',Path(tmp)/'pbds'
            path.write_text(HEADER+code)
            subprocess.run([args.compiler,'-std=c++17','-O2',*args.cxxflag,str(path),'-o',str(exe)],check=True)
            subprocess.run([str(exe)],check=True,timeout=20)
        report['pbds_operations']=10000
    if args.json_output:
        args.json_output.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({k:v for k,v in report.items() if k!='file_sha256'}))


if __name__=='__main__':
    main()
