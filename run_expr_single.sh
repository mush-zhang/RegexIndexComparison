#! /bin/bash

dirname=traffic_result
mkdir ${dirname} 

# Free
for n in 2 3 4 6 10; do
    for c in 0.1 0.2 0.3 0.5 0.7 1; do
        ./benchmark.out FREE -t 1 -w 1 -o ${dirname} -n ${n} -c ${c}
        ./benchmark.out FREE -t 1 -w 1 -o ${dirname} -n ${n} --presuf -c ${c}
    done
done
# Rei
num_ngrams_list=( 2 4 8 16 24 32 48 64 80 96 128 160 192 256 384 512 )
grams_list=( 2 3 4 )
for k in ${num_ngrams_list[*]}; do
    for n in ${grams_list[*]}; do
        ./benchmark.out REI -t 1 -w 1 -o ${dirname} -n ${n} -c ${c} -k ${k}
        ./benchmark.out REI -t 1 -w 1 -o ${dirname} -n ${n} -c ${c} -k ${k}
    done
done 
# Best
for c in 0.1 0.2 0.3 0.5 0.7 1; do
    ./benchmark.out BEST -t 1 -w 1 -o ${dirname} -c ${c}
done
