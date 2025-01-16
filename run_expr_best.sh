#! /bin/bash

thread_list=( 16 )
extra=""

unset -v wl_num
unset -v dirname
extra=""

while getopts ":d:r:t:w:" opt; do
    case "${opt}" in
        d) echo "Option -d is triggered  with value $OPTARG"
            data_file=$OPTARG
            ;;
        r) echo "Option -r is triggered with value $OPTARG"
            regex_file=$OPTARG
            ;;
        t) echo "Option -t is triggered."
            thread_list=( 16 12 10 8 6 4 2 1 )
            ;;
        k) echo "Option -k is triggered with value $OPTARG"
            extra="-k ${OPTARG} "
            ;;
        w) echo "Option -w is triggered with value $OPTARG"
            dirname=result/${OPTARG}_best_result
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
    extra="${extra}-r ${regex_file} -d ${data_file}"
fi

timeout_prefix="{ timeout 3h time -v "
timeout_suffix="; } 2> ${dirname}/time_report"

echo ${dirname}
mkdir -p ${dirname} 
sel_list=( 0.7 0.5 0.2 0.15 0.12 0.1 0.05 0.02 )
num_repeat=1
red_list=( 0.05 0.1 0.2 0.3 0.5 0.7 0.85 )
# Best
for t in ${thread_list[*]}; do
    for c in ${sel_list[*]}; do
        curr_suffix="${timeout_suffix}_best_t${t}_red${red}_c${c}.txt"
        for red in ${red_list[*]}; do
            curr_cmd="${timeout_prefix} ./benchmark.out BEST -t ${t} -w ${wl_num} -o ${dirname} -c ${c} -e ${num_repeat}  --wl_reduce ${red} ${extra} ${curr_suffix}"
            echo ${curr_cmd}
            eval "${curr_cmd}"
            retVal=$?
            if [ $retVal -ne 0 ]; then
                echo "Timeout"
                break
            fi
        done
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
done