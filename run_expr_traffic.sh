#! /bin/bash

dirname=result/traffic_result_new
mkdir -p ${dirname} 
thread_list=( 1 2 4 6 8 10 )
sel_list=( 0.01 0.02 0.03 0.05 0.07 0.1 0.12 0.15 0.2 0.3 0.5 )
num_repeat=5

# Best
for c in ${sel_list[*]}; do
    for t in ${thread_list[*]}; do
        echo benchmark.out BEST -t ${t} -w 1 -o ${dirname} -c ${c} -e ${num_repeat}
        ./benchmark.out BEST -t ${t} -w 1 -o ${dirname} -c ${c} -e ${num_repeat}
    done
done
# Free
for n in 2 4 6 8 10; do
    for c in ${sel_list[*]}; do
        echo benchmark.out FREE -t 1 -w 1 -o ${dirname} -n ${n} --presuf -c ${c} -e ${num_repeat}
        ./benchmark.out FREE -t 1 -w 1 -o ${dirname} -n ${n} --presuf -c ${c} -e ${num_repeat}
        for t in ${thread_list[*]}; do
            echo benchmark.out FREE -t ${t} -w 1 -o ${dirname} -n ${n} -c ${c} -e ${num_repeat}
            ./benchmark.out FREE -t ${t} -w 1 -o ${dirname} -n ${n} -c ${c} -e ${num_repeat}
        done
    done
done
echo benchmark.out FAST -t 1 -w 1 -o ${dirname}  --relax DETERM -e ${num_repeat}
./benchmark.out FAST -t 1 -w 1 -o ${dirname} --relax DETERM -e ${num_repeat}
echo benchmark.out FAST -t 1 -w 1 -o ${dirname}  --relax RANDOM -e ${num_repeat}
./benchmark.out FAST -t 1 -w 1 -o ${dirname} --relax RANDOM -e ${num_repeat}
