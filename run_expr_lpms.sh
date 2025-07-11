#! /bin/bash

thread_list=( 16 )

unset -v wl_num
unset -v dirname
extra=""
max_num_ngram=-1
while getopts ":d:r:q:t:w:k:" opt; do
    case "${opt}" in
        d) echo "Option -d is triggered  with value $OPTARG"
            data_file=$OPTARG
            ;;
        r) echo "Option -r is triggered with value $OPTARG"
            regex_file=$OPTARG
            ;;
        q) echo "Option -q is triggered  with value $OPTARG"
            extra="--test ${OPTARG} "
            ;;
        t) echo "Option -t is triggered."
            thread_list=( 16 12 10 8 6 4 2 1 )
            ;;
        k) echo "Option -k is triggered with value $OPTARG"
            extra="-k ${OPTARG} "
            max_num_ngram=${OPTARG}
            ;;
        w) echo "Option -w is triggered with value $OPTARG"
            dirname=result/${OPTARG}_lpms_result4
            echo "$OPTARG"
            if [ "$OPTARG" == "traffic" ]; then
                wl_num=1
            elif [ "$OPTARG" == "db_x" ]; then
                wl_num=2
            elif [ "$OPTARG" == "webpages" ]; then
                wl_num=3
            elif [ "$OPTARG" == "protein" ]; then
                wl_num=4
            elif [ "$OPTARG" == "sys_y" ]; then
                wl_num=5
            elif [ "$OPTARG" == "enron" ]; then
                wl_num=6
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

timeout_prefix="{ timeout 1h /usr/bin/time -v "
timeout_suffix="; } 2> ${dirname}/time_report"

echo ${dirname}
mkdir -p ${dirname} 
num_repeat=1

# Fast
for t in ${thread_list[*]}; do
    curr_suffix="${timeout_suffix}_lpms_t${t}_determ_${max_num_ngram}.txt"
    curr_cmd="${timeout_prefix} ./benchmark.out LPMS -t ${t} -w ${wl_num} -o ${dirname} --relax DETERM -e ${num_repeat} ${extra} ${curr_suffix}"
    echo ${curr_cmd}
    eval "${curr_cmd}"
    retVal=$?
    if [ $retVal -ne 0 ]; then
        echo "Timeout"
    fi
    # curr_suffix2="${timeout_suffix}_lpms_t${t}_random_${max_num_ngram}.txt"
    # curr_cmd2="${timeout_prefix} ./benchmark.out LPMS -t ${t} -w ${wl_num} -o ${dirname} --relax RANDOM -e ${num_repeat} ${extra} ${curr_suffix2}"
    # echo ${curr_cmd2}
    # eval "${curr_cmd2}"
    # retVal2=$?
    # if [ $retVal2 -ne 0 ]; then
    #     echo "Timeout"
    # fi
done
