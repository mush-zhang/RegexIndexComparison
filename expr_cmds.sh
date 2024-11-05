#! /bin/bash

# Expr 2
for qcount in 100 500 2000 2500 5000; do
    ./run_expr.sh -w synthetic_expr2_q${qcount}_d20000 \
    -r data/synthetic/expr2/queries/query_workload_${qcount}.txt \
    -d data/synthetic/expr2/datasets/dataset_20000.txt
done
for dcount in 20000 40000 60000 80000 100000; do
    ./run_expr.sh -w synthetic_expr2_q1000_d${dcount} \
    -r data/synthetic/expr2/queries/query_workload_1000_for_${dcount}.txt \
    -d data/synthetic/expr2/datasets/dataset_${dcount}.txt
done

# Protein

# dblp
for qcount in 1000 2000 3000 4000; do
    ./run_expr.sh -w dblp_${qcount} \
    -r data/dblp/small/query${qcount}.txt \
    -d data/dblp/small/authors.txt
done

# # Expr 4
# for rob_wl in 1 2 3 4; do
#     for perc in 10 30 50; do
#         ./run_expr.sh -w synthetic_expr4_rob0${rob_wl}_${perc} \
#         -r data/synthetic/expr4/queries/Rob0${rob_wl}_queries_${perc}pct.txt \
#         -d data/synthetic/expr4/datasets/Rob0${rob_wl}.txt
#     done
# done

# # Expr 1
# for expr in 0 1 2 3; do
#     std_val=$(( ($expr + 1) * 100 ))
#     ./run_expr.sh -w synthetic_expr1_${expr} \
#     -r data/synthetic/expr1/query_${expr}.txt \
#     -d data/synthetic/expr1/data_${expr}_std${std_val}.txt
# done
