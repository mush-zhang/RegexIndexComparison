#! /bin/bash

./run_expr_baseline.sh -w webpages

# # Protein
./run_expr_baseline.sh -w protein

# # DB_X
./run_expr_baseline.sh -w db_x

# # traffic
./run_expr_baseline.sh -w traffic

# robust
for rob_wl in 1 2 3 4; do
    for perc in 10 30 50; do
        ./run_expr_baseline.sh -w synthetic_expr4_rob0${rob_wl}_${perc} \
        -r data/synthetic/expr4/queries/Rob0${rob_wl}_queries_${perc}pct.txt \
        -q data/synthetic/expr4/queries/Rob0${rob_wl}_test_queries_2pct.txt \
        -d data/synthetic/expr4/datasets/Rob0${rob_wl}.txt
    done
done

# dblp
# for qcount in 1000 2000; do
for qcount in 1000; do
    ./run_expr_baseline.sh -w dblp_small_${qcount} \
    -r data/dblp/small/query${qcount}.txt \
    -d data/dblp/small/authors.txt -k ${k}
done