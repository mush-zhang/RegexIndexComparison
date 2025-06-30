#! /bin/bash

synthetic_dir=result/synthetic2_50000_qs500
mkdir -p ${synthetic_dir}
for selectivity in 0.001 0.005 0.010 0.02 0.05 0.1 0.2 0.5; do
    ./run_expr_baseline.sh -r data/configurable_synthetics/dataset_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -d data/configurable_synthetics/queries_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -w synthetic_${datasetSize}
    ./run_expr_best.sh -r data/configurable_synthetics/dataset_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -d data/configurable_synthetics/queries_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -w synthetic_${datasetSize}
    ./run_expr_free.sh -r data/configurable_synthetics/dataset_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -d data/configurable_synthetics/queries_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -w synthetic_${datasetSize}
    ./run_expr_lpms.sh -r data/configurable_synthetics/dataset_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -d data/configurable_synthetics/queries_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -w synthetic_${datasetSize}
    ./run_expr_vggraph_greedy.sh -r data/configurable_synthetics/dataset_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -d data/configurable_synthetics/queries_alph16_data${datasetSize}_qs500_sel${selectivity}.txt \
    -w synthetic_${datasetSize}
done
# remove intermediate files

synthetic_dir=result/synthetic5_50000
mkdir -p ${synthetic_dir}
datasetSize=50000
for querySize in 500 1000 2000 5000; do
    ./run_expr_baseline.sh -r data/configurable_synthetics/dataset_alph16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -d data/configurable_synthetics/queries_alph16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -w synthetic_${datasetSize}
    ./run_expr_best.sh -r data/configurable_synthetics/dataset_alph16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -d data/configurable_synthetics/queries_alph16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -w synthetic_${datasetSize}
    ./run_expr_free.sh -r data/configurable_synthetics/dataset_alph16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -d data/configurable_synthetics/queries_alph16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -w synthetic_${datasetSize}
    ./run_expr_lpms.sh -r data/configurable_synthetics/dataset_alph16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -d data/configurable_synthetics/queries_alph16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -w synthetic_${datasetSize}
    ./run_expr_vggraph_greedy.sh -r data/configurable_synthetics/dataset_alph16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -d data/configurable_synthetics/queries_alph16_data${datasetSize}_qs${querySize}_sel0.005.txt \
    -w synthetic_${datasetSize}
done
