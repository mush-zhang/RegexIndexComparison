#! /bin/bash

thread_list=( 16 )
extra=""

unset -v wl_num

while getopts "d:r:t:w" opt; do
    case ${opt} in
        d) echo "Option -d is triggered."
            data_file=$OPTARG
            ;;
        r) echo "Option -r is triggered."
            regex_file=$OPTARG
            ;;
        t) echo "Option -t is triggered."
            thread_list=( 1 2 4 6 8 10 12 16 )
            ;;
        w) echo "Option -w is triggered."
            dirname=result/${OPTARG}_result
            if [ "$OPTARG" == "traffic" ]; then
                wl_num=1
            elif [ "$OPTARG" == "db_x" ]; then
                wl_num=2
            elif [ "$OPTARG" == "webpages" ]; then
                wl_num=3
            else
                extra="-r ${regex_file} -d ${data_file}"
                wl_num = 0
            fi
            ;;
        \?)echo "Invalid argument $OPTARG" >&2
            ;;
    esac
done
: ${wl_num:?Missing -h}

# if [ "$2" == "comp_thread" ]; then
#     thread_list=( 1 2 4 6 8 10 12 16 )
# else 
#     thread_list=( 16 )

# extra=""
# if [ "$1" == "traffic" ]; then
#     wl_num=1
# elif [ "$1" == "db_x" ]; then
#     thread_list=( 16 )
#     wl_num=2
# elif [ "$1" == "webpages" ]; then
#     thread_list=( 16 )
#     wl_num=3
# else
#     wl_num=0
#     extra=$3 # argument expected in form "-r [regex_file] -d [data_file]" with quotations
# fi

timeout_prefix="timeout --foreground 1h"

# dirname=result/${1}_result_new
echo ${dirname}
mkdir -p ${dirname} 
sel_list=( 0.01 0.02 0.03 0.05 0.07 0.1 0.12 0.15 0.2 0.3 0.5 0.7)
num_repeat=5

# Best
if [ ${wl_num} != 1 ]; then
    for c in ${sel_list[*]}; do
        for t in ${thread_list[*]}; do
            echo benchmark.out BEST -t ${t} -w ${wl_num} -o ${dirname} -c ${c} -e ${num_repeat}
            ${timeout_prefix} ./benchmark.out BEST -t ${t} -w ${wl_num} -o ${dirname} -c ${c} -e ${num_repeat}
        done
    done
fi
# Free
for n in 2 4 6 8 10 12 14 16; do
    for c in ${sel_list[*]}; do
        echo benchmark.out FREE -t 1 -w ${wl_num} -o ${dirname} -n ${n} --presuf -c ${c} -e ${num_repeat} ${extra}
        ${timeout_prefix} ./benchmark.out FREE -t 1 -w ${wl_num} -o ${dirname} -n ${n} --presuf -c ${c} -e ${num_repeat} ${extra}
        for t in ${thread_list[*]}; do
            echo benchmark.out FREE -t ${t} -w ${wl_num} -o ${dirname} -n ${n} -c ${c} -e ${num_repeat} ${extra}
            ${timeout_prefix} ./benchmark.out FREE -t ${t} -w ${wl_num} -o ${dirname} -n ${n} -c ${c} -e ${num_repeat} ${extra}
        done
    done
done
# Fast
for t in ${thread_list[*]}; do
    echo benchmark.out FAST -t ${t} -w ${wl_num} -o ${dirname}  --relax DETERM -e ${num_repeat} ${extra}
    ${timeout_prefix} ./benchmark.out FAST -t ${t} -w ${wl_num} -o ${dirname} --relax DETERM -e ${num_repeat} ${extra}
    echo benchmark.out FAST -t ${t} -w ${wl_num} -o ${dirname}  --relax RANDOM -e ${num_repeat} ${extra}
    ${timeout_prefix} ./benchmark.out FAST -t ${t} -w ${wl_num} -o ${dirname} --relax RANDOM -e ${num_repeat} ${extra}
done
