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
            elif [ "$OPTARG" == "protein" ]; then
                wl_num=4
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

timeout_prefix="{ timeout 3h time -v "
timeout_suffix="; } 2> ${dirname}/time_report"

echo ${dirname}
mkdir -p ${dirname} 
# sel_list=( 0.01 0.02 0.03 0.05 0.07 0.1 0.12 0.15 0.2 0.3 0.5 0.7)
sel_list=( 0.05 0.1 0.12 0.15 0.2 0.5 0.7)
num_repeat=1

# Best
for c in ${sel_list[*]}; do
    for t in ${thread_list[*]}; do
        curr_suffix="${timeout_suffix}_best_t${t}_c${c}.txt"
        curr_cmd="${timeout_prefix} ./benchmark.out BEST -t ${t} -w ${wl_num} -o ${dirname} -c ${c} -e ${num_repeat} ${extra} ${curr_suffix}"
        echo ${curr_cmd}
        eval "${curr_cmd}"
        retVal=$?
        if [ $retVal -ne 0 ]; then
            echo "Timeout"
            break
        fi
    done
fi
# Free
# for n in 2 4 6 8 10 12 14 16; do
for n in 2 4 6; do
    for c in ${sel_list[*]}; do
        # curr_cmd="${timeout_prefix} ./benchmark.out FREE -t 1 -w ${wl_num} -o ${dirname} -n ${n} --presuf -c ${c} -e ${num_repeat} ${extra} echo ${curr_cmd}
        #     eval "${curr_cmd}""
        # echo ${curr_cmd}
        # eval "${curr_cmd}"
        for t in ${thread_list[*]}; do
            curr_suffix="${timeout_suffix}_free_t${t}_c${c}_n${n}.txt"
            curr_cmd="${timeout_prefix} ./benchmark.out FREE -t ${t} -w ${wl_num} -o ${dirname} -n ${n} -c ${c} -e ${num_repeat} ${extra} ${curr_suffix}"
            echo ${curr_cmd}
            eval "${curr_cmd}"
        done
    done
done

# Fast
for t in ${thread_list[*]}; do
    curr_suffix="${timeout_suffix}_lpms_t${t}_determ.txt"
    curr_cmd="${timeout_prefix} ./benchmark.out LPMS -t ${t} -w ${wl_num} -o ${dirname} --relax DETERM -e ${num_repeat} ${extra} ${curr_suffix}"
    echo ${curr_cmd}
    eval "${curr_cmd}"

    curr_suffix2="${timeout_suffix}_lpms_t${t}_random.txt"
    curr_cmd2="${timeout_prefix} ./benchmark.out LPMS -t ${t} -w ${wl_num} -o ${dirname} --relax RANDOM -e ${num_repeat} ${extra} ${curr_suffix2}"
    echo ${curr_cmd2}
    eval "${curr_cmd2}"
done
