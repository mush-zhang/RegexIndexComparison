#! /bin/bash

for qcount in 100 500 2000 2500 5000; do
    ./run_expr.sh -w synthetic_q${qcount}_d20000 \
    -r data/synthetic/expr2/queries/query_workload_${qcount}.txt \
    -d data/synthetic/expr2/datasets/dataset_20000.txt
for dcount in 20000 40000 60000 80000 100000; do
    ./run_expr.sh -w synthetic_q1000_d${dcount} \
    -r data/synthetic/expr2/queries/query_workload_1000_for_${dcount}.txt \
    -d data/synthetic/expr2/datasets/dataset_${dcount}.txt