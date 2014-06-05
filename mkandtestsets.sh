#!/bin/bash

if [ -z "$1" ]; then
  echo "You need at least 1 argument"
  exit 1
fi

make clean && make -j4 || exit 1;

mv pheet_test.out pheet_old_test.out

for i in $(seq 1 1 $1)
do
    echo
    echo
    echo
    echo
    echo "--------------------------------------------------------------------------------"
    echo
    echo " TEST NUMBER $i "
    echo 
    echo "--------------------------------------------------------------------------------"
    echo
    echo
    echo
    echo
    bin/pheet_test | tee -a pheet_test.out
done

python2.7 csvheet < pheet_test.out > pheet_test.csv
