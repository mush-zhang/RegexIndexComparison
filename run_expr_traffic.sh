#! /bin/bash

dirname=result/traffic_result_new
mkdir -p ${dirname} 
thread_list=( 1 2 4 6 8 10 )
sel_list=( 0.01 0.02 0.03 0.05 0.07 0.1 0.12 0.15 0.2 0.3 0.5 )

# Best
for c in ${sel_list[*]}; do
    for t in ${thread_list[*]}; do
        echo benchmark.out BEST -t ${t} -w 1 -o ${dirname} -c ${c}
        ./benchmark.out BEST -t ${t} -w 1 -o ${dirname} -c ${c}
    done
done
# Free
for n in 2 4 6 8 10; do
    for c in ${sel_list[*]} 0.7; do
        echo benchmark.out FREE -t 1 -w 1 -o ${dirname} -n ${n} --presuf -c ${c}
        ./benchmark.out FREE -t 1 -w 1 -o ${dirname} -n ${n} --presuf -c ${c}
        for t in ${thread_list[*]}; do
            echo benchmark.out FREE -t ${t} -w 1 -o ${dirname} -n ${n} -c ${c} 
            ./benchmark.out FREE -t ${t} -w 1 -o ${dirname} -n ${n} -c ${c}
        done
    done
done
