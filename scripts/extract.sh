#!/bin/bash

script=$(realpath -- ${BASH_SOURCE[0]})
ROOT=$(dirname $script)/..
source ${ROOT}/setup.sh

if [ $# -lt 2 ]; then
    echo "Usage\n${script} run_list.txt output_file"
    exit 4
fi

output=$2
tmp=${2}.tmp
[ -f $tmp ] && rm $tmp
${ROOT}/analysis/extract.py $1 > $tmp
awk '$3 == 0 {print $1}' $tmp >> ${output}_0
awk '$3 == 1 {print $1}' $tmp >> ${output}_1
awk '$3 == 2 {print $1}' $tmp >> ${output}_2

paste ${output}_{0..2} >> ${output}
rm ${tmp} ${output}_{0..2}
