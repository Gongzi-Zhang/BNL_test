#!/bin/bash

script=$(realpath -- ${BASH_SOURCE[0]})
ROOT=$(dirname $script)/..
source ${ROOT}/setup.sh

RAW_DATA_DIR="~/Documents/STAR_Prototype_Data/STAR_Cosmic_Runs"

# find out all runs
runs=$(ls -tr $RAW_DATA_DIR/Run*_{Info,list,sync}.txt | sed 's/_\(Info\|list\|sync\).txt//' | uniq)
for run in $runs; do
    ${ROOT}/add_run.sh ${run}_Info.txt
done
