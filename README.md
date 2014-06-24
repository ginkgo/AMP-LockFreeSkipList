AMP-LockFreeSkipList
====================

This repository contains the implementation of a lock-free skip list in C++ for the Advanced Multiprocessor Programming course.
It also contains other list-based set classes.

There's a correctness test program `settest.cpp` that can be built with

```shell
$ ./makesettest.sh
```

and run with

```shell
$ ./settest
```

The stress test is built with 

```shell
$ make
```

or with

```shell
$ .\mkandtestsets.sh <N>
```

which also executes the stress tests and collects data. ```<N>``` is the number of time each test should be performed.
Modify ```test/ds_stress/DsStressTests.cpp``` to add your own set classes.
Modify ```test/test_variants/ds.h``` to chane test parameters.

The Python script ```create_figs.py``` parses the output CSV file and creates figures.

The folder ```presentation``` contains the LaTeX source for the first presentation.
