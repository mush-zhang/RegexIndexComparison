#! /bin/bash

# sysy
sysydir=result/sysy_results
mkdir ${sysydir}

kups=( 50 100 500 1000 2000 5000 10000 )
for k in ${kups[*]}; do
        time -v ./benchmark.out FREE -t 16 -w 5 -o ${sysydir} -n 2 -c 0.7 -e 10 -k ${k}  2> ${sysydir}/time_report_free_t16_c0.7_n2_${k}.txt
        echo "end executing FREE max_key=${k}"
done

for k in 5 10 20; do
        time -v ./benchmark.out LPMS -t 16 -w 5 -o ${sysydir} --relax DETERM -e 10 -k ${k}  2> ${sysydir}/time_report_lpms_t16_determ_${k}.txt
        echo "end executing LPMS max_key=${k}"
done

for k in 5 10 20; do
        time -v ./benchmark.out BEST -t 16 -w 5 -o ${sysydir} -c 0.1 -e 10 -k ${k}  2> ${sysydir}/time_report_best_t16_c0.1_${k}.txt
        echo "end executing BEST c=0.1 max_key=${k}"
        time -v ./benchmark.out BEST -t 16 -w 5 -o ${sysydir} -c 0.5 -e 10 -k ${k}  2> ${sysydir}/time_report_best_t16_c0.5_${k}.txt
        echo "end executing BEST c=0.5 max_key=${k}"
done
