#!/bin/bash

script=$(realpath -- ${BASH_SOURCE[0]})
ROOT=$(dirname $script)/..
source ${ROOT}/setup.sh

function usage
{
    echo -e "$script run_Info.txt [run_type] [note]"
}

function extract_datetime
{
    dt=$1; shift
    day=$(echo $dt | cut -d'/' -f1)
    month=$(echo $dt | cut -d'/' -f2)
    year=$(echo $dt | cut -d' ' -f1 | cut -d'/' -f3)
    time=$(echo $dt | cut -d' ' -f2)
    echo "$year$month$day $time"
}

if [ $# -lt 1 ]; then
    echo -e "ERROR\tAt least 1 arguments needed"
    usage
    exit
fi
input_file=$1
run_type=data
if [ $# -ge 2 ]; then
    run_type=$2
fi
Note=''
if [ $# -ge 3 ]; then
    Note=$3
fi
config=${CALIROOT}/data/config.cfg

StartTime=$(grep 'Start Time' $input_file | cut -d ':' -f2-)
StartTime=$(extract_datetime "$StartTime")
StopTime=$(grep 'Stop Time' $input_file | cut -d ':' -f2-)
StopTime=$(extract_datetime "$StopTime")
Length=$(grep 'Elapsed time' $input_file | cut -d ' ' -f4)
Length=$(perl -e "printf(\"%.1f\", $Length/3600)")
Geometry=$(grep 'Geometry' ${config} | cut -d':' -f2)
Channels=$(grep 'Channels' ${config} | cut -d':' -f2)
Trigger=$(grep 'Trigger' ${config} | cut -d':' -f2)
if [ "$run_type" = "ptrg" ]; then
    Trigger=0
fi
sync_file=${input_file/Info/sync}
Events=$(tail -n1 $sync_file | awk '{print $3}')
list_file=${input_file/Info/list}
Size=$(ls -lh $list_file | awk '{print $5}')
latestId=$(caliDB latest | cut -d':' -f2)

echo -e "INFO\trun ${input_file} will be insert as run $((latestId+1))"

caliDB insert \
    --Type $run_type \
    --StartTime "$StartTime" \
    --StopTime "$StopTime"   \
    --Length $Length	    \
    --Geometry $Geometry    \
    --Channels $Channels    \
    --Trigger $Trigger	    \
    --Events $Events	    \
    --Size $Size    \
    --Note "$Note"
if [ $? -ne 0 ]; then
   echo -e "Failed to insert the record"
   exit 4
fi
echo -e "INFO\tsuccessfully insert run ${input_file} into database as run $((latestId+1))"

fdir=$(dirname -- $input_file)
fname=$(basename -- $input_file)
old=${fname%_Info.txt}
latestId=$(caliDB latest | cut -d':' -f2 | tr -d ' ')
new="${CALIROOT}/data/Run$latestId" 
rename "s#${fdir}/$old#$new#" ${fdir}/${old}_{Info,list,sync}.txt
