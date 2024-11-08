#! /bin/bash

thread_list=( 16 )
extra=""

unset -v wl_num
unset -v dirname

while getopts ":d:r:t:w:" opt; do
    case "${opt}" in
        d) echo "Option -d is triggered  with value $OPTARG"
            data_file=$OPTARG
            ;;
        r) echo "Option -r is triggered with value $OPTARG"
            regex_file=$OPTARG
            ;;
        t) echo "Option -t is triggered."
            thread_list=( 1 2 4 6 8 10 12 16 )
            ;;
        w) echo "Option -w is triggered with value $OPTARG"
            dirname=result/${OPTARG}_result
            echo "$OPTARG"
            if [ "$OPTARG" == "traffic" ]; then
                wl_num=1
            elif [ "$OPTARG" == "db_x" ]; then
                wl_num=2
            elif [ "$OPTARG" == "webpages" ]; then
                wl_num=3
            else
                wl_num=0
            fi
            ;;
        \?) echo "Invalid argument $OPTARG" >&2
            ;;
    esac
done
: ${wl_num:?Missing -h}

if [ "$wl_num" == "0" ]; then
    extra="-r ${regex_file} -d ${data_file}"
fi

timeout_prefix="{timeout 1h time -v "
timeout_suffix=";} 2> ${dirname}/time_report"

echo ${dirname}
mkdir -p ${dirname} 
sel_list=( 0.01 0.02 0.03 0.05 0.07 0.1 0.12 0.15 0.2 0.3 0.5 0.7)
num_repeat=1

# Best
if [ ${wl_num} != 1 ]; then
    for c in ${sel_list[*]}; do
        for t in ${thread_list[*]}; do
            curr_suffix="${timeout_suffix}_best_t${t}_c${c}.txt"
            echo benchmark.out BEST -t ${t} -w ${wl_num} -o ${dirname} -c ${c} -e ${num_repeat} ${extra} ${curr_suffix}
            ${timeout_prefix} ./benchmark.out BEST -t ${t} -w ${wl_num} -o ${dirname} -c ${c} -e ${num_repeat} ${extra} ${curr_suffix}
        done
    done
fi
# Free
for n in 2 4 6 8 10 12 14 16; do
    for c in ${sel_list[*]}; do
        # echo benchmark.out FREE -t 1 -w ${wl_num} -o ${dirname} -n ${n} --presuf -c ${c} -e ${num_repeat} ${extra}
        # ${timeout_prefix} ./benchmark.out FREE -t 1 -w ${wl_num} -o ${dirname} -n ${n} --presuf -c ${c} -e ${num_repeat} ${extra}
        for t in ${thread_list[*]}; do
            curr_suffix="${timeout_suffix}_free_t${t}_c${c}_n${n}.txt"
            echo benchmark.out FREE -t ${t} -w ${wl_num} -o ${dirname} -n ${n} -c ${c} -e ${num_repeat} ${extra} ${curr_suffix}
            ${timeout_prefix} ./benchmark.out FREE -t ${t} -w ${wl_num} -o ${dirname} -n ${n} -c ${c} -e ${num_repeat} ${extra} ${curr_suffix}
        done
    done
done
# Fast
for t in ${thread_list[*]}; do
    curr_suffix="${timeout_suffix}_lpsm_t${t}_determ.txt"
    echo benchmark.out LPSM -t ${t} -w ${wl_num} -o ${dirname}  --relax DETERM -e ${num_repeat} ${extra} ${curr_suffix}
    ${timeout_prefix} ./benchmark.out LPSM -t ${t} -w ${wl_num} -o ${dirname} --relax DETERM -e ${num_repeat} ${extra} ${curr_suffix}

    curr_suffix2="${timeout_suffix}_lpsm_t${t}_random.txt"
    echo benchmark.out LPSM -t ${t} -w ${wl_num} -o ${dirname}  --relax RANDOM -e ${num_repeat} ${extra} ${curr_suffix2}
    ${timeout_prefix} ./benchmark.out LPSM -t ${t} -w ${wl_num} -o ${dirname} --relax RANDOM -e ${num_repeat} ${extra} ${curr_suffix2}
done
