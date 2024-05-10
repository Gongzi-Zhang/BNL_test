#!/bin/bash

script=$(realpath -- ${BASH_SOURCE[0]})
ROOT=$(dirname $script)/..
source ${ROOT}/setup.sh

RAW_DATA_DIR=$(realpath ~/Documents/STAR_Prototype_Data/STAR_Cosmic_Runs)

# find out all runs
runs=$(ls -tr $RAW_DATA_DIR/Run*_{Info,list,sync}.txt 2>/dev/null | sed 's/_\(Info\|list\|sync\).txt//' | uniq)
for run in $runs; do
    info_file=${run}_Info.txt
    list_file=${run}_list.txt
    time=$(ls -l --time-style='+%s' $list_file | cut -d' ' -f6)
    now=$(date +%s)
    diff=$((now - time))
    nfile=$(ls ${run}_{Info,list,sync}.txt 2>/dev/null | wc -l)

    if [ $nfile -lt 3 ] && [ $diff -lt 7200 ]; then
	# current run, skip it
	continue
    fi

    ${ROOT}/scripts/add_run.sh ${info_file}
    # if [ $? -ne 0 ]; then
done
