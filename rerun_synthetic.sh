#! /bin/bash

synthetic_dir=result/synthetic2_50000_qs500
mkdir -p ${synthetic_dir}
for selectivity in '0.001' '0.005' '0.010' '0.020' '0.050' '0.100' '0.200' '0.500'; do
    ./run_expr_baseline.sh -d data/configurable_synthetic/dataset_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -r data/configurable_synthetic/queries_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -w synthetic_${datasetSize}
    ./run_expr_best.sh -d data/configurable_synthetic/dataset_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -r data/configurable_synthetic/queries_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -w synthetic_${datasetSize}
    ./run_expr_free.sh -d data/configurable_synthetic/dataset_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -r data/configurable_synthetic/queries_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -w synthetic_${datasetSize}
    ./run_expr_lpms.sh -d data/configurable_synthetic/dataset_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -r data/configurable_synthetic/queries_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -w synthetic_${datasetSize}
    ./run_expr_vggraph_greedy.sh -d data/configurable_synthetic/dataset_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -r data/configurable_synthetic/queries_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -w synthetic_${datasetSize}
    for k in 50 200 500 1000; do
        ./run_expr_vggraph_greedy.sh -d data/configurable_synthetic/dataset_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
        -r data/configurable_synthetic/queries_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
        -w synthetic_${datasetSize} -k ${k}
        ./run_expr_best.sh -d data/configurable_synthetic/dataset_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
        -r data/configurable_synthetic/queries_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
        -w synthetic_${datasetSize} -k ${k}
        ./run_expr_free.sh -d data/configurable_synthetic/dataset_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
        -r data/configurable_synthetic/queries_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
        -w synthetic_${datasetSize} -k ${k}
        ./run_expr_lpms.sh -d data/configurable_synthetic/dataset_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
        -r data/configurable_synthetic/queries_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
        -w synthetic_${datasetSize} -k ${k}
    done
done
# remove intermediate files

synthetic_dir=result/synthetic5_50000
mkdir -p ${synthetic_dir}
datasetSize=50000
for querySize in 500 1000 2000 5000; do
    ./run_expr_baseline.sh -d data/configurable_synthetic/dataset_alph16_data${datasetSize}_qs${querySize}_sel0.020.txt \
    -r data/configurable_synthetic/queries_alph16_data${datasetSize}_qs${querySize}_sel0.020.txt \
    -w synthetic_${datasetSize}
    ./run_expr_best.sh -d data/configurable_synthetic/dataset_alph16_data${datasetSize}_qs${querySize}_sel0.020.txt \
    -r data/configurable_synthetic/queries_alph16_data${datasetSize}_qs${querySize}_sel0.020.txt \
    -w synthetic_${datasetSize}
    ./run_expr_free.sh -d data/configurable_synthetic/dataset_alph16_data${datasetSize}_qs${querySize}_sel0.020.txt \
    -r data/configurable_synthetic/queries_alph16_data${datasetSize}_qs${querySize}_sel0.020.txt \
    -w synthetic_${datasetSize}
    ./run_expr_lpms.sh -d data/configurable_synthetic/dataset_alph16_data${datasetSize}_qs${querySize}_sel0.020.txt \
    -d data/configurable_synthetic/queries_alph16_data${datasetSize}_qs${querySize}_sel0.020.txt \
    -w synthetic_${datasetSize}
    ./run_expr_vggraph_greedy.sh -d data/configurable_synthetic/dataset_alph16_data${datasetSize}_qs${querySize}_sel0.020.txt \
    -r data/configurable_synthetic/queries_alph16_data${datasetSize}_qs${querySize}_sel0.020.txt \
    -w synthetic_${datasetSize}
    for k in 50 200 500 1000; do
        ./run_expr_vggraph_greedy.sh -d data/configurable_synthetic/dataset_alph16_data${datasetSize}_qs${querySize}_sel0.020.txt \
        -r data/configurable_synthetic/queries_alph16_data${datasetSize}_qs${querySize}_sel0.020.txt \
        -w synthetic_${datasetSize} -k ${k}
        ./run_expr_best.sh -d data/configurable_synthetic/dataset_alph16_data${datasetSize}_qs${querySize}_sel0.020.txt \
        -r data/configurable_synthetic/queries_alph16_data${datasetSize}_qs${querySize}_sel0.020.txt \
        -w synthetic_${datasetSize} -k ${k}
        ./run_expr_free.sh -d data/configurable_synthetic/dataset_alph16_data${datasetSize}_qs${querySize}_sel0.020.txt \
        -r data/configurable_synthetic/queries_alph16_data${datasetSize}_qs${querySize}_sel0.020.txt \
        -w synthetic_${datasetSize} -k ${k}
        ./run_expr_lpms.sh -d data/configurable_synthetic/dataset_alph16_data${datasetSize}_qs${querySize}_sel0.020.txt \
        -r data/configurable_synthetic/queries_alph16_data${datasetSize}_qs${querySize}_sel0.020.txt \
        -w synthetic_${datasetSize} -k ${k}
    done
done
