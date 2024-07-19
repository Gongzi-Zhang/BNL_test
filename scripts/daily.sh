#!/bin/bash

script=$(realpath -- ${BASH_SOURCE[0]})
ROOT=$(dirname $script)/..
source ${ROOT}/setup.sh

${ROOT}/scripts/github.sh 2>>${ROOT}/work/daily.log
${ROOT}/scripts/sync.sh 2>>${ROOT}/work/daily.log
python3 ${ROOT}/scripts/IV.py 2>>${ROOT}/work/daily.log
scp figures/misc/IV.png weibin@cali:public/cali/BNL_test/figures/misc/
