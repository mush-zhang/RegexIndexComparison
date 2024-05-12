#! /bin/bash

dirname=result/traffic_result_new
mkdir -p ${dirname} 

# Free
for n in 2 4 6 8 10; do
    for c in 0.01 0.02 0.03 0.05 0.07 0.1 0.2 0.3 0.5 0.7; do
        echo benchmark.out FREE -t 1 -w 1 -o ${dirname} -n ${n} --presuf -c ${c}
        ./benchmark.out FREE -t 1 -w 1 -o ${dirname} -n ${n} --presuf -c ${c}
        for t in 1 2 4 6 8 10; do
            echo benchmark.out FREE -t ${t} -w 1 -o ${dirname} -n ${n} -c ${c} 
            ./benchmark.out FREE -t ${t} -w 1 -o ${dirname} -n ${n} -c ${c}
        done
    done
done
# Best
for c in 0.01 0.02 0.03 0.05 0.07 0.1 0.12 0.15 0.2 0.3 0.5; do
    for t in 1 2 4; do
        echo benchmark.out BEST -t ${t} -w 1 -o ${dirname} -c ${c}
        ./benchmark.out BEST -t ${t} -w 1 -o ${dirname} -c ${c}
    done
done
