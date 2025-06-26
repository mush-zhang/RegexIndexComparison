#! /bin/bash

# VGGraph_Greedy Experiment Script
# Run comprehensive benchmarks across different datasets

# # dblp
# ./run_expr_vggraph_greedy.sh -w dblp_small_1000 \
# -r data/dblp/small/query1000.txt \
# -d data/dblp/small/authors.txt

# # ./run_expr_vggraph_greedy.sh -w protein

# # dblp with key upper bounds
# kups=( 200 500 1000 2000 )
# for k in ${kups[*]}; do
#     ./run_expr_vggraph_greedy.sh -w dblp_small_1000 \
#     -r data/dblp/small/query1000.txt \
#     -d data/dblp/small/authors.txt -k ${k}
# done


# Protein with key upper bounds
kups=( 50 100 200 500 1000 )
for k in ${kups[*]}; do
    ./run_expr_vggraph_greedy.sh -w protein -k ${k}
done

# ./run_expr_vggraph_greedy.sh -w traffic 

# ./run_expr_vggraph_greedy.sh -w enron 

# ./run_expr_vggraph_greedy.sh -w webpages

# ./run_expr_vggraph_greedy.sh -w db_x

# ./run_expr_vggraph_greedy.sh -w sys_y

# webpage with key upper bounds
kups=( 5 10 50 100 )
for k in ${kups[*]}; do 
    ./run_expr_vggraph_greedy.sh -w webpages -k ${k}
done

# DB_X with key upper bounds
for k in ${kups[*]}; do
    ./run_expr_vggraph_greedy.sh -w db_x -k ${k}
done

# traffic with key upper bounds
for k in 5 10 15; do
    ./run_expr_vggraph_greedy.sh -w traffic -k ${k}
done

# enron with key upper bounds
kups=( 5 10 15 50 100 200)
for k in ${kups[*]}; do 
    ./run_expr_vggraph_greedy.sh -w enron -k ${k}
done

