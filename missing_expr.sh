#! /bin/bash

# # dblp
# dblpdir=result/missing_dblp_result
# mkdir ${dblpdir}
# { timeout 3h time -v ./benchmark.out LPMS -t 16 -w 0 -o ${dblpdir} --relax DETERM -e 1 -k 150 \
#     -r data/dblp/small/query1000.txt -d data/dblp/small/authors.txt ; } 2> ${dblpdir}/time_report_lpms_t16_determ_150.txt

# { timeout 3h time -v ./benchmark.out LPMS -t 16 -w 0 -o ${dblpdir} --relax DETERM -e 1 -k 500 \
#     -r data/dblp/small/query1000.txt -d data/dblp/small/authors.txt ; } 2> ${dblpdir}/time_report_lpms_t16_determ_500.txt

# # # webpage

# # Protein

# # DB_X
dbxdir=result/missing_dbx_result
mkdir ${dbxdir}
{ timeout 3h time -v ./benchmark.out FREE -t 16 -w 2 -o ${dbxdir} -n 4 -c 0.3 -e 1 ; } 2> ${dbxdir}/time_report_free_t16_c0.3_n4_-1.txt

# # # traffic
# trafficdir=result/missing_traffic_result
# mkdir ${trafficdir}
# { timeout 3h time -v ./benchmark.out FREE -t 16 -w 1 -o ${trafficdir} -n 2 -c 0.7 -e 1 ; } 2> ${trafficdir}/time_report_free_t16_c0.7_n2_-1.txt

# { timeout 3h time -v ./benchmark.out FREE -t 16 -w 1 -o ${trafficdir} -n 4 -c 0.3 -e 1 ; } 2> ${trafficdir}/time_report_free_t16_c0.3_n4_-1.txt

# # robust
