#! /bin/bash

# get default scalefactor
if [ $1 == "traffic" ]; then
    wl_num=1
elif [ $1 == "db_x" ]; then
    wl_num=2
elif [ $1 == "sys_y" ]; then
    wl_num=3
else
    exit 1
fi
dirname=result/${1}_result_new
echo ${dirname}
mkdir -p ${dirname} 
thread_list=( 1 2 4 6 8 10 12 16 )
sel_list=( 0.01 0.02 0.03 0.05 0.07 0.1 0.12 0.15 0.2 0.3 0.5 0.7)
num_repeat=5

# Best
for c in ${sel_list[*]}; do
    for t in ${thread_list[*]}; do
        echo benchmark.out BEST -t ${t} -w ${wl_num} -o ${dirname} -c ${c} -e ${num_repeat}
        timeout 6h ./benchmark.out BEST -t ${t} -w ${wl_num} -o ${dirname} -c ${c} -e ${num_repeat}
    done
done
# Free
for n in 2 4 6 8 10 12 14 16; do
    for c in ${sel_list[*]}; do
        echo benchmark.out FREE -t 1 -w ${wl_num} -o ${dirname} -n ${n} --presuf -c ${c} -e ${num_repeat}
        timeout 6h ./benchmark.out FREE -t 1 -w ${wl_num} -o ${dirname} -n ${n} --presuf -c ${c} -e ${num_repeat}
        for t in ${thread_list[*]}; do
            echo benchmark.out FREE -t ${t} -w ${wl_num} -o ${dirname} -n ${n} -c ${c} -e ${num_repeat}
            timeout 6h ./benchmark.out FREE -t ${t} -w ${wl_num} -o ${dirname} -n ${n} -c ${c} -e ${num_repeat}
        done
    done
done
echo benchmark.out FAST -t 1 -w ${wl_num} -o ${dirname}  --relax DETERM -e ${num_repeat}
timeout 6h ./benchmark.out FAST -t 1 -w ${wl_num} -o ${dirname} --relax DETERM -e ${num_repeat}
echo benchmark.out FAST -t 1 -w ${wl_num} -o ${dirname}  --relax RANDOM -e ${num_repeat}
timeout 6h ./benchmark.out FAST -t 1 -w ${wl_num} -o ${dirname} --relax RANDOM -e ${num_repeat}
