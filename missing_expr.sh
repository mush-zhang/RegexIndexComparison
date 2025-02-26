#! /bin/bash

# # dblp
dblpdir=result/missing_dblp_result2
mkdir ${dblpdir}
{ timeout 3h time -v ./benchmark.out BEST -t 16 -w 0 -o ${dblpdir} -c 0.5 -e 10 -k 150 \
    -r data/dblp/small/query1000.txt -d data/dblp/small/authors.txt ; } 2> ${dblpdir}/time_report__best_t16_c0.5_150.txt

# { timeout 3h time -v ./benchmark.out LPMS -t 16 -w 0 -o ${dblpdir} --relax DETERM -e 1 -k 150 \
#     -r data/dblp/small/query1000.txt -d data/dblp/small/authors.txt ; } 2> ${dblpdir}/time_report_lpms_t16_determ_150.txt

# { timeout 3h time -v ./benchmark.out LPMS -t 16 -w 0 -o ${dblpdir} --relax DETERM -e 1 -k 500 \
#     -r data/dblp/small/query1000.txt -d data/dblp/small/authors.txt ; } 2> ${dblpdir}/time_report_lpms_t16_determ_500.txt
#kups=( 20 50 100 150 200 500 1000 2000 )

kups = ( 150 500 )
for k in ${kups[*]}; do
    for qcount in 1000; do
        ./run_expr_free.sh -w dblp_small_${qcount} \
        -r data/dblp/small/query${qcount}.txt \
        -d data/dblp/small/authors.txt -k ${k}
    done
done

# # # webpage
for kval in 30 20 10; do
    ./run_expr_best.sh -w webpages -k ${kval}
    ./run_expr_lpms.sh -w webpages -k ${kval}
done

# # Protein

# # DB_X
# dbxdir=result/missing_dbx_result
# mkdir ${dbxdir}
# { timeout 3h time -v ./benchmark.out FREE -t 16 -w 2 -o ${dbxdir} -n 4 -c 0.03 -e 1 ; } 2> ${dbxdir}/time_report_free_t16_c0.3_n4_-1.txt

# # # traffic

# traffic
# ./run_expr_free.sh -w traffic

# trafficdir=result/missing_traffic_result
# # mkdir ${trafficdir}

# { timeout 3h time -v ./benchmark.out FREE -t 16 -w 1 -o ${trafficdir} -n 2 -c 0.7 -e 10 ; } 2> ${trafficdir}/time_report_free_t16_c0.7_n2_-1.txt

# # { timeout 3h time -v ./benchmark.out FREE -t 16 -w 1 -o ${trafficdir} -n 2 -c 0.7 -e 1 ; } 2> ${trafficdir}/time_report_free_t16_c0.7_n2_-1.txt

# { timeout 3h time -v ./benchmark.out FREE -t 16 -w 1 -o ${trafficdir} -n 4 -c 0.3 -e 10 ; } 2> ${trafficdir}/time_report_free_t16_c0.3_n4_-1.txt

# { timeout 3h time -v ./benchmark.out FREE -t 10 -w 1 -o ${trafficdir} -n 4 -c 0.3 -e 10 ; } 2> ${trafficdir}/time_report_free_t10_c0.3_n4_-1.txt
# # # robust
