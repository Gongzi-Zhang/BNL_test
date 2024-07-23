#!/bin/bash

script=$(realpath -- ${BASH_SOURCE[0]})
ROOT=$(dirname $script)/..
source ${ROOT}/setup.sh

${ROOT}/scripts/github.sh 
${ROOT}/scripts/sync.sh 
python3 ${ROOT}/scripts/IV.py
scp figures/misc/IV.png weibin@cali:public/cali/BNL_test/figures/misc/
