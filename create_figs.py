import sys
import csv

from collections import defaultdict

import numpy as np
import matplotlib.pyplot as plt

import math

import re

def classname(string):
    pattern = re.compile(r'(\w+)(<\w+>)?')

    match = pattern.match(string)

    return match.group(1) if match else 'unknown'

tests = []

with open('pheet_test.csv', 'r') as file:
    csv_file = csv.DictReader(file, delimiter=',', quotechar='"')
    
    for line in csv_file:
        tests.append(line)    

index = defaultdict(lambda : defaultdict(lambda : defaultdict(list)))

for test in tests:
    test['ds'] = classname(test['ds'])
    test.pop('prefill')
    test.pop('scheduler')
    test.pop('hostname')
    test.pop('date')
    test.pop('test')
    test.pop('seed')

    index[test.pop('type')][test.pop('ds')][int(test.pop('cpus'))].append(test)

#print (index)
#print (list(index.keys()))

for testedfunction,testtype in [('contains','1:1:2d'), ('remove','1:1:2'), ('add','1:1:2')]:
    fig = plt.figure()
    #plt.title('Throughput for %s() (%s)' % (testedfunction,testtype))
    L = []
    C = []
    cpulist = None
    for setclass,tests in index[testtype].items():

        Y = []
        X = []
        for cpus in sorted(tests.keys()):
            cputests = tests[cpus]
            X.append(math.log(cpus))
            Y.append(np.average([float(t['%s throughput' % testedfunction]) for t in cputests]))

        l, = plt.plot(X,Y,'.-')

        cpulist = list(sorted(tests.keys()))
        
        L.append(l)
        C.append(setclass)
    
    ax = plt.gca()
    ax.set_xlabel('CPUs')
        
    cputicks = [math.log(p) for p in cpulist]
        
    plt.xticks(cputicks, cpulist)
    plt.xlim(xmin = cputicks[0], xmax = cputicks[-1])
    
        
    ax.set_yscale('log')
    ax.set_ylabel('throughput [calls/s]')

    fig.legend(L, C)
    
    plt.tight_layout()
    #plt.show()
    plt.savefig('presentation/%s.pdf' % testedfunction)
