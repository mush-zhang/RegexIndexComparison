#! /bin/bash
 
# dblp
for qcount in 1000 2000 3000 4000; do
    ./run_expr.sh -w dblp_small_${qcount} \
    -r data/dblp/small/query${qcount}.txt \
    -d data/dblp/small/authors.txt
done

