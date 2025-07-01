#! /bin/bash

# # dblp
# ./run_expr_trigram.sh -w dblp_small_1000 \
# -r data/dblp/small/query1000.txt \
# -d data/dblp/small/authors.txt

# ./run_expr_trigram.sh -w traffic 

# ./run_expr_trigram.sh -w protein

# ./run_expr_trigram.sh -w enron 

# ./run_expr_trigram.sh -w webpages

# ./run_expr_trigram.sh -w db_x

# ./run_expr_trigram.sh -w sys_y


# # webpage
kups=( 170000 360000 1500000 )
for k in ${kups[*]}; do 
    ./run_expr_trigram.sh -w webpages -k ${k}
done

# # Protein
# kups=( 50 100 200 500 1000 )
# for k in ${kups[*]}; do
#     ./run_expr_trigram.sh -w protein -k ${k}
# done

# # DB_X
# for k in ${kups[*]}; do
#     ./run_expr_trigram.sh -w db_x -k ${k}
# done

# # traffic
# for k in 5 10 15; do
#     ./run_expr_trigram.sh -w traffic  -k ${k}
# done

# kups=( 5 10 15 50 100 200)
# for k in ${kups[*]}; do 
#     ./run_expr_trigram.sh -w enron -k ${k}
# done

kups=( 200 500 1000 2000 )
for k in ${kups[*]}; do
    ./run_expr_trigram.sh -w dblp_small_1000 \
    -r data/dblp/small/query1000.txt \
    -d data/dblp/small/authors.txt -k ${k}
done