#! /bin/bash

# dblp
kups=( 20 50 100 150 200 500 1000 2000 )
for k in ${kups[*]}; do
    for qcount in 1000 2000; do
        ./run_expr_best.sh -w dblp_small_${qcount} \
        -r data/dblp/small/query${qcount}.txt \
        -d data/dblp/small/authors.txt -k ${k}
    done
done

# # webpage
# kups=( 5 10 15 100 )
# for k in ${kups[*]}; do 
#     ./run_expr_best.sh -w webpages -k ${k}
# done

# Protein
kups=( 50 100 200 500 1000 )
for k in ${kups[*]}; do
    ./run_expr_best.sh -w protein -k ${k}
done

# DB_X
for k in ${kups[*]}; do
    ./run_expr_best.sh -w db_x  -k ${k}
done
