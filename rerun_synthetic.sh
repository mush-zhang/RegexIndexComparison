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


# # enron with key upper bounds
# kups=( 15 50 100 200 500)
# for k in ${kups[*]}; do 
#     ./run_expr_baseline.sh -w enron -k ${k}
#     ./run_expr_best.sh -w enron -k ${k}
#     ./run_expr_free.sh -w enron -k ${k}
#     ./run_expr_lpms.sh -w enron -k ${k}
#     ./run_expr_trigram.sh -w enron -k ${k}
#     # ./run_expr_vggraph_greedy.sh -w enron -k ${k}
# done
# ./run_expr_vggraph_greedy.sh -w enron -k 500



synthetic_dir=result/synthetic2_50000_qs500
mkdir -p ${synthetic_dir}
for selectivity in 0.001 0.005 0.010 0.02 0.05 0.1 0.2 0.5; do
    ./run_expr_baseline.sh -r data/configurable_synthetics/dataset_alpha16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -d data/configurable_synthetics/queries_alpha16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -w synthetic_${datasetSize}
    ./run_expr_best.sh -r data/configurable_synthetics/dataset_alpha16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -d data/configurable_synthetics/queries_alpha16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -w synthetic_${datasetSize}
    ./run_expr_free.sh -r data/configurable_synthetics/dataset_alpha16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -d data/configurable_synthetics/queries_alpha16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -w synthetic_${datasetSize}
    ./run_expr_lpms.sh -r data/configurable_synthetics/dataset_alpha16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -d data/configurable_synthetics/queries_alpha16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -w synthetic_${datasetSize}
    ./run_expr_vggraph_greedy.sh -r data/configurable_synthetics/dataset_alpha16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -d data/configurable_synthetics/queries_alpha16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -w synthetic_${datasetSize}
done
# remove intermediate files

synthetic_dir=result/synthetic5_50000
mkdir -p ${synthetic_dir}
datasetSize=50000
for querySize in 500 1000 2000 5000; do
    ./run_expr_baseline.sh -r data/configurable_synthetics/dataset_alpha16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -d data/configurable_synthetics/queries_alpha16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -w synthetic_${datasetSize}
    ./run_expr_best.sh -r data/configurable_synthetics/dataset_alpha16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -d data/configurable_synthetics/queries_alpha16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -w synthetic_${datasetSize}
    ./run_expr_free.sh -r data/configurable_synthetics/dataset_alpha16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -d data/configurable_synthetics/queries_alpha16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -w synthetic_${datasetSize}
    ./run_expr_lpms.sh -r data/configurable_synthetics/dataset_alpha16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -d data/configurable_synthetics/queries_alpha16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -w synthetic_${datasetSize}
    ./run_expr_vggraph_greedy.sh -r data/configurable_synthetics/dataset_alpha16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -d data/configurable_synthetics/queries_alpha16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -w synthetic_${datasetSize}
done

datasetSize=50000


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