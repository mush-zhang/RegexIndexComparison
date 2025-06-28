#! /bin/bash

# # dblp
# dblpdir=result/revision/dblp2
# mkdir -p ${dblpdir}
# { timeout 3h /usr/bin/time -v ./benchmark.out FREE -t 16 -w 0 -o ${dblpdir} -n 2 -c 0.7 -e 1 -k 150 \
#     -r data/dblp/small/query1000.txt -d data/dblp/small/authors.txt ; } 2> ${dblpdir}/time_report_free_t16_c0.7_n2_150.txt
# { timeout 3h /usr/bin/time -v ./benchmark.out FREE -t 16 -w 0 -o ${dblpdir} -n 2 -c 0.7 -e 1 -k 500 \
#     -r data/dblp/small/query1000.txt -d data/dblp/small/authors.txt ; } 2> ${dblpdir}/time_report_free_t16_c0.7_n2_500.txt


# # webpage
# webdir=result/revision/webpages2
# mkdir -p ${webdir}
# { timeout 3h /usr/bin/time -v ./benchmark.out BEST -t 16 -w 3 -o ${webdir} -c 0.1 -e 1 ; } 2> ${webdir}/time_report_best_t16_c0.1_-1.txt


# enron with key upper bounds
kups=( 5 10 15 50 100 200)
for k in ${kups[*]}; do 
    ./run_expr_baseline.sh -w enron -k ${k}
    ./run_expr_best.sh -w enron -k ${k}
    ./run_expr_free.sh -w enron -k ${k}
    ./run_expr_lpms.sh -w enron -k ${k}
    ./run_expr_trigram.sh -w enron -k ${k}
    ./run_expr_vggraph_greedy.sh -w enron -k ${k}
done

# { timeout 3h /usr/bin/time -v ./benchmark.out FREE -t 16 -w 3 -o ${webdir} -n 2 -c 0.02 -e 1 -k 5 ; } 2> ${webdir}/time_report_free_t16_c0.02_n2_5.txt
# { timeout 3h /usr/bin/time -v ./benchmark.out FREE -t 16 -w 3 -o ${webdir} -n 2 -c 0.5 -e 1 ; } 2> ${webdir}/time_report_free_t16_c0.5_n2_-1.txt
# { timeout 3h /usr/bin/time -v ./benchmark.out FREE -t 16 -w 3 -o ${webdir} -n 4 -c 0.7 -e 1 ; } 2> ${webdir}/time_report_free_t16_c0.7_n4_-1.txt
# { timeout 3h /usr/bin/time -v ./benchmark.out FREE -t 16 -w 3 -o ${webdir} -n 4 -c 0.15 -e 1 ; } 2> ${webdir}/time_report_free_t16_c0.15_n4_-1.txt

# # protein
# proteindir=result/revision/protein
# mkdir -p ${proteindir}
# { timeout 3h /usr/bin/time -v ./benchmark.out BEST -t 16 -w 4 -o ${proteindir} -c 0.7 -e 1 -k 50; } 2> ${proteindir}/time_report_best_t16_c0.7_50.txt
# { timeout 3h /usr/bin/time -v ./benchmark.out BEST -t 16 -w 4 -o ${proteindir} -c 0.7 -e 1 -k 100; } 2> ${proteindir}/time_report_best_t16_c0.7_100.txt

# { timeout 3h /usr/bin/time -v ./benchmark.out FREE -t 16 -w 4 -o ${proteindir} -n 2 -c 0.2 -e 1 ; } 2> ${proteindir}/time_report_free_t16_c0.2_n2_-1.txt
# { timeout 3h /usr/bin/time -v ./benchmark.out FREE -t 16 -w 4 -o ${proteindir} -n 2 -c 0.15 -e 1 ; } 2> ${proteindir}/time_report_free_t16_c0.15_n2_-1.txt
# { timeout 3h /usr/bin/time -v ./benchmark.out FREE -t 16 -w 4 -o ${proteindir} -n 4 -c 0.7 -e 1 ; } 2> ${proteindir}/time_report_free_t16_c0.7_n4_-1.txt


# # DB_X
# dbxdir=result/revision/dbx
# mkdir -p ${dbxdir}
# { timeout 3h /usr/bin/time -v ./benchmark.out FREE -t 16 -w 2 -o ${dbxdir} -n 2 -c 0.7 -e 1 -k 50 ; } 2> ${dbxdir}/time_report_free_t16_c0.7_n2_-1.txt
# { timeout 3h /usr/bin/time -v ./benchmark.out FREE -t 16 -w 2 -o ${dbxdir} -n 2 -c 0.03 -e 1 ; } 2> ${dbxdir}/time_report_free_t16_c0.03_n2_-1.txt

# # traffic
# trafficdir=result/revision/traffic
# mkdir -p ${trafficdir}
# { timeout 3h /usr/bin/time -v ./benchmark.out BEST -t 16 -w 4 -o ${trafficdir} -c 0.7 -e 1 ; } 2> ${trafficdir}/time_report_best_t16_c0.7_-1.txt
# { timeout 3h /usr/bin/time -v ./benchmark.out BEST -t 16 -w 4 -o ${trafficdir} -c 0.7 -e 1 -k 100; } 2> ${trafficdir}/time_report_best_t16_c0.7_100.txt

# { timeout 3h /usr/bin/time -v ./benchmark.out FREE -t 16 -w 4 -o ${trafficdir} -n 2 -c 0.02 -e 1 -k 5 ; } 2> ${trafficdir}/time_report_free_t16_c0.02_n2_5.txt
# { timeout 3h /usr/bin/time -v ./benchmark.out FREE -t 16 -w 4 -o ${trafficdir} -n 4 -c 0.3 -e 1 ; } 2> ${trafficdir}/time_report_free_t16_c0.3_n4_-1.txt
# { timeout 3h /usr/bin/time -v ./benchmark.out FREE -t 16 -w 4 -o ${trafficdir} -n 2 -c 0.7 -e 1 ; } 2> ${trafficdir}/time_report_free_t16_c0.7_n2_-1.txt

# # robust
# # for rob_wl in 1 2 3 4; do
# #     for perc in 10 30 50; do
# #         ./run_expr_best.sh -w synthetic_expr4_rob0${rob_wl}_${perc} \
# #         -r data/synthetic/expr4/queries/Rob0${rob_wl}_queries_${perc}pct.txt \
# #         -q data/synthetic/expr4/queries/Rob0${rob_wl}_test_queries_2pct.txt \
# #         -d data/synthetic/expr4/datasets/Rob0${rob_wl}.txt
# #     done
# # done

# # for rob_wl in 1 2 3 4; do
# #     for perc in 10 30 50; do
# #         ./run_expr_free.sh -w synthetic_expr4_rob0${rob_wl}_${perc} \
# #         -r data/synthetic/expr4/queries/Rob0${rob_wl}_queries_${perc}pct.txt \
# #         -q data/synthetic/expr4/queries/Rob0${rob_wl}_test_queries_2pct.txt \
# #         -d data/synthetic/expr4/datasets/Rob0${rob_wl}.txt
# #     done
# # done

# # for rob_wl in 1 2 3 4; do
# #     for perc in 10 30 50; do
# #         ./run_expr_lpms.sh -w synthetic_expr4_rob0${rob_wl}_${perc} \
# #         -r data/synthetic/expr4/queries/Rob0${rob_wl}_queries_${perc}pct.txt \
# #         -q data/synthetic/expr4/queries/Rob0${rob_wl}_test_queries_2pct.txt \
# #         -d data/synthetic/expr4/datasets/Rob0${rob_wl}.txt
# #     done
# # done

# # for k in 20 50 100 300; do
# #     rob_wl=4
# #     for perc in 10 30 50; do
# #         ./run_expr_lpms.sh -w synthetic_expr4_rob0${rob_wl}_${perc} \
# #         -r data/synthetic/expr4/queries/Rob0${rob_wl}_queries_${perc}pct.txt \
# #         -q data/synthetic/expr4/queries/Rob0${rob_wl}_test_queries_2pct.txt \
# #         -d data/synthetic/expr4/datasets/Rob0${rob_wl}.txt -k ${k}

# #         ./run_expr_free.sh -w synthetic_expr4_rob0${rob_wl}_${perc} \
# #         -r data/synthetic/expr4/queries/Rob0${rob_wl}_queries_${perc}pct.txt \
# #         -q data/synthetic/expr4/queries/Rob0${rob_wl}_test_queries_2pct.txt \
# #         -d data/synthetic/expr4/datasets/Rob0${rob_wl}.txt -k ${k}

# #         ./run_expr_best.sh -w synthetic_expr4_rob0${rob_wl}_${perc} \
# #         -r data/synthetic/expr4/queries/Rob0${rob_wl}_queries_${perc}pct.txt \
# #         -q data/synthetic/expr4/queries/Rob0${rob_wl}_test_queries_2pct.txt \
# #         -d data/synthetic/expr4/datasets/Rob0${rob_wl}.txt -k ${k}
# #     done
# # done

# # rob_wl=4
# # perc=10
# # ./run_expr_best.sh -w synthetic_expr4_rob0${rob_wl}_${perc} \
# #         -r data/synthetic/expr4/queries/Rob0${rob_wl}_queries_${perc}pct.txt \
# #         -q data/synthetic/expr4/queries/Rob0${rob_wl}_test_queries_2pct.txt \
# #         -d data/synthetic/expr4/datasets/Rob0${rob_wl}.txt

# # ./run_expr_free.sh -w synthetic_expr4_rob0${rob_wl}_${perc} \
# #         -r data/synthetic/expr4/queries/Rob0${rob_wl}_queries_${perc}pct.txt \
# #         -q data/synthetic/expr4/queries/Rob0${rob_wl}_test_queries_2pct.txt \
# #         -d data/synthetic/expr4/datasets/Rob0${rob_wl}.txt

# # ./run_expr_lpms.sh -w synthetic_expr4_rob0${rob_wl}_${perc} \
# #         -r data/synthetic/expr4/queries/Rob0${rob_wl}_queries_${perc}pct.txt \
# #         -q data/synthetic/expr4/queries/Rob0${rob_wl}_test_queries_2pct.txt \
# #         -d data/synthetic/expr4/datasets/Rob0${rob_wl}.txt