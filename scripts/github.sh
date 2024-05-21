#!/bin/bash

script=$(realpath -- ${BASH_SOURCE[0]})
ROOT=$(dirname $script)/..
source ${ROOT}/setup.sh

cd "$ROOT"
bin/caliDB export -f db.csv
mv db.csv database/
git add database/db.csv
git add database/BNL_test.db
if ! git diff --staged --quiet; then
    git commit -m "daily automatic update"
    export SSH_AUTH_SOCK=/run/user/1000/keyring/ssh
    git push
fi
