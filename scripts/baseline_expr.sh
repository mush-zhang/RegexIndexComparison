#! /bin/bash

./run_expr_baseline.sh -w webpages

# # Protein
./run_expr_baseline.sh -w protein

# # DB_X
./run_expr_baseline.sh -w db_x

# # traffic
./run_expr_baseline.sh -w traffic

# dblp
for qcount in 1000; do
    ./run_expr_baseline.sh -w dblp_small_${qcount} \
    -r data/dblp/small/query${qcount}.txt \
    -d data/dblp/small/authors.txt -k ${k}
done

# enron
./run_expr_baseline.sh -w enron