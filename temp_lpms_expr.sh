#! /bin/bash

# dblp
# kups=( 20 50 100 150 200 500 1000 2000 )
# for k in ${kups[*]}; do
#     for qcount in 1000 2000; do
#         ./run_expr_lpms.sh -w dblp_small_${qcount} \
#         -r data/dblp/small/query${qcount}.txt \
#         -d data/dblp/small/authors.txt -k ${k}
#     done
# done

# # webpage
kups=( 5 10 50 100 )
for k in ${kups[*]}; do 
    ./run_expr_lpms.sh -w webpages -k ${k}
done

# # Protein
# kups=( 50 100 200 500 1000 )
# for k in ${kups[*]}; do
#     ./run_expr_lpms.sh -w protein -k ${k}
# done

# # DB_X
# for k in ${kups[*]}; do
#     ./run_expr_lpms.sh -w db_x -k ${k}
# done

# # traffic
# for k in 5 10 15; do
#     ./run_expr_lpms.sh -w traffic  -k ${k}
# done

# robust
# for rob_wl in 1 2 3 4; do
#     for perc in 10 30 50; do
#         ./run_expr_lpms.sh -w synthetic_expr4_rob0${rob_wl}_${perc} \
#         -r data/synthetic/expr4/queries/Rob0${rob_wl}_queries_${perc}pct.txt \
#         -q data/synthetic/expr4/queries/Rob0${rob_wl}_test_queries_2pct.txt \
#         -d data/synthetic/expr4/datasets/Rob0${rob_wl}.txt
#     done
# done

# Enron
kups=( 5 10 15 50 100 200)
for k in ${kups[*]}; do 
    ./run_expr_lpms.sh -w enron -k ${k}
done