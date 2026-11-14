#!/usr/bin/env python3
"""Compare actual book NTT, cached roots and adapted pinned KACTL; no book edits."""
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

HERE = ROOT / 'tests'
VARIANTS = HERE / 'ntt_variants'
PIN = 'b3548e973f8772010c1ef84e6d427c749733fd2c'


def active_lines(code):
    code = re.sub(r'/\*.*?\*/|//[^\n]*', '', code, flags=re.S)
    return sum(bool(line.strip()) for line in code.splitlines())


def sources(baseline_ref='cd5adda'):
    raw = (ROOT/'math/ntt.tex').read_text()
    installed = next(LISTING.finditer(raw))[2]
    baseline = subprocess.check_output(['git','show',baseline_ref+':math/ntt.tex'],cwd=ROOT,text=True)
    original = next(LISTING.finditer(baseline))[2]
    modular = list(LISTING.finditer((ROOT/'math/gcd_and_modular.tex').read_text()))[1][2]
    # Keep the book's actual dependency, including its checked modular arithmetic.
    dependency = modular[:modular.index('\noptional<ll> mod_inverse')]
    # Only these three helpers are required when copying the NTT into a solution.
    required_dependency = modular[:modular.index('\nll mod_add')] + modular[modular.index('ll mod_multiply'):modular.index('\noptional<ll> mod_inverse')]
    cached_transform = (VARIANTS/'cached_transform.hpp').read_text()
    cached = original[:original.index('void number_theoretic_transform')]+cached_transform+'\n'+original[original.index('vector<int> convolution'):]
    def tokens(text):
        return re.sub(r'\s+', '', re.sub(r'/\*.*?\*/|//[^\n]*', '', text, flags=re.S))
    if tokens(installed) == tokens(cached):
        cached = installed  # Test the installed listing itself, not a separate copy.
        installed_variant = 'cached'
    elif tokens(installed) == tokens(original):
        installed_variant = 'current'
    else:
        raise AssertionError('Installed NTT differs from both measured alternatives; review it before comparing.')
    kraw = (VARIANTS/'vendor/NumberTheoreticTransform.h').read_text()
    praw = (VARIANTS/'vendor/ModPow.h').read_text()
    def clean(code):
        code = re.sub(r'/\*.*?\*/', '', code, flags=re.S)
        return re.sub(r'^#.*$', '', code, flags=re.M).strip()
    power = clean(praw).replace('1000000007','998244353')
    core = clean(kraw).replace('const ll mod = (119 << 23) + 1, root = 62;', 'const ll root = 3;')
    # Original floor(log2(s))+1 wastes a level when s is a power of two.
    core = core.replace('B = 32 - __builtin_clz(s),', 'B = s == 1 ? 0 : 32 - __builtin_clz(s - 1),')
    assert 'root = 3;' in core and '__builtin_clz(s - 1)' in core
    adapter = (VARIANTS/'kactl_adapter.hpp').read_text()
    kactl = power+'\n'+core+'\n'+adapter
    code = HEADER+'\nusing vi=vector<int>;\n#define sz(x) (int)(x).size()\n#define rep(i,a,b) for(int i=(a);i<(b);++i)\n'
    for name, impl in [('current',original),('cached',cached)]:
        code += '\nnamespace '+name+' {\n'+dependency+impl+'\n}\n'
    code += '\nnamespace kactl {\n'+kactl+'\n}\n'
    code += (HERE/'ntt_variants_test.cpp').read_text()
    files = [ROOT/'math/ntt.tex',ROOT/'math/gcd_and_modular.tex',HERE/'ntt_variants_test.cpp',
             VARIANTS/'cached_transform.hpp',VARIANTS/'kactl_adapter.hpp',
             VARIANTS/'vendor/NumberTheoreticTransform.h',VARIANTS/'vendor/ModPow.h',Path(__file__)]
    metadata = dict(kactl_revision=PIN,baseline_ref=baseline_ref,
        baseline_ntt_sha256=hashlib.sha256(baseline.encode()).hexdigest(),installed_variant=installed_variant,
        generated_cpp_sha256=hashlib.sha256(code.encode()).hexdigest(),
        sha256={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in files},
        active_lines=dict(current=dict(algorithm=active_lines(original),dependency=active_lines(required_dependency)),
                          cached=dict(algorithm=active_lines(cached),dependency=active_lines(required_dependency)),
                          kactl=dict(core_and_power=active_lines(power+'\n'+core),adapter=active_lines(adapter))),
        adaptations=['KACTL modulus configured to 998244353; duplicate declaration removed.',
                     'Root 62 -> 3 to preserve exact forward-transform values.',
                     'Minimum power-of-two padding, including s=1, preserves full 2^23 result limit.',
                     'Public vector<int> transform/inverse/convolution wrappers included in all measurements.'])
    return code, metadata


WORKER = r'''
import json,resource,subprocess,sys
r=subprocess.run(sys.argv[1:],capture_output=True,text=True,check=True,timeout=240)
d=json.loads(r.stdout)
d['peak_rss_mib']=resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss/(1024**2 if sys.platform=='darwin' else 1024)
print(json.dumps(d))
'''


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--compiler',default='clang++')
    p.add_argument('--baseline-ref',default='cd5adda')
    p.add_argument('--cxxflag',action='append',default=[])
    p.add_argument('--benchmark',action='store_true')
    p.add_argument('--max-size',action='store_true')
    p.add_argument('--repeat',type=int,default=5)
    p.add_argument('--json-output',type=Path)
    p.add_argument('--emit-source',type=Path)
    args=p.parse_args()
    code,report=sources(args.baseline_ref)
    report.update(compiler=subprocess.check_output([args.compiler,'--version'],text=True).splitlines()[0],
                  platform=platform.platform(),seed=20261028,flags=['-std=c++17','-O2',*args.cxxflag],benchmarks=[])
    if args.emit_source: args.emit_source.write_text(code)
    with tempfile.TemporaryDirectory(prefix='handbook-ntt-') as tmp:
        cpp,exe=Path(tmp)/'test.cpp',Path(tmp)/'test';cpp.write_text(code)
        subprocess.run([args.compiler,*report['flags'],str(cpp),'-o',str(exe)],check=True)
        r=subprocess.run([str(exe)],capture_output=True,text=True,check=True,timeout=180)
        report['correctness']=r.stdout.strip();print(r.stdout.strip(),flush=True)
        if args.max_size:
            report['maximum_transform']=[]
            for i in range(3):
                r=subprocess.run([str(exe),'max',str(i)],capture_output=True,text=True,check=True,timeout=240)
                report['maximum_transform'].append(r.stdout.strip());print(r.stdout.strip(),flush=True)
        if args.benchmark:
            names=['current','cached','kactl']
            for case in ['large','boundary','above_boundary','tiny_batch','medium_batch','large_batch','unbalanced']:
                for state in ['cold','warm']:
                    samples={name:[] for name in names}
                    for repeat in range(args.repeat):
                        # Rotate process order to reduce systematic scheduling bias.
                        for j in range(3):
                            i=(j+repeat)%3
                            r=subprocess.run([sys.executable,'-c',WORKER,str(exe),case,str(i),state],capture_output=True,text=True,check=True)
                            samples[names[i]].append(json.loads(r.stdout))
                    assert len({s['checksum'] for runs in samples.values() for s in runs})==1
                    row=dict(case=case,state=state,samples=samples)
                    for name,runs in samples.items():
                        row[name]=dict(ms=statistics.median(s['ms'] for s in runs),
                                       peak_rss_mib=max(s['peak_rss_mib'] for s in runs),
                                       convolutions=runs[0]['convolutions'],a_size=runs[0]['a_size'],b_size=runs[0]['b_size'])
                    report['benchmarks'].append(row)
                    print(json.dumps({k:v for k,v in row.items() if k!='samples'}),flush=True)
        print('Active lines:',json.dumps(report['active_lines']),flush=True)
    if args.json_output:args.json_output.write_text(json.dumps(report,indent=2)+'\n')


if __name__=='__main__':main()
