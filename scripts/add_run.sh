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
input_file=$1; shift
run_type=data
if [ $# -ge 1 ]; then
    run_type=$1
    shift
fi
Flag='good'
if [ $# -ge 1 ]; then
    Flag=$1
    shift
fi
Note=''
if [ $# -ge 1 ]; then
    Note=$1
    shift
fi
config=${CALIROOT}/data/config.cfg

StartTime=$(grep '\<Start Time\>' $input_file | cut -d ':' -f2-)
StartTime=$(extract_datetime "$StartTime")
StopTime=$(grep '\<Stop Time\>' $input_file | cut -d ':' -f2-)
StopTime=$(extract_datetime "$StopTime")
Length=$(grep '\<Elapsed time\>' $input_file | cut -d ' ' -f4)
Length=$(perl -e "printf(\"%.1f\", $Length/3600)")
Geometry=$(grep '\<Geometry\>' ${config} | cut -d':' -f2)
Channels=$(grep '\<Channels\>' ${config} | cut -d':' -f2)
Trigger=$(grep '\<Trigger\>' ${config} | cut -d':' -f2)
if [ "$run_type" = "ptrg" ]; then
    Trigger=0
fi
T1=$(grep '\<T1\>' ${config} | cut -d':' -f2)
T2=$(grep '\<T2\>' ${config} | cut -d':' -f2)
T3=$(grep '\<T3\>' ${config} | cut -d':' -f2)
T4=$(grep '\<T4\>' ${config} | cut -d':' -f2)
sync_file=${input_file/Info/sync}
Events=$(tail -n1 $sync_file | awk '{print $3}')
LG=$(grep '\<LG\>' ${config} | cut -d':' -f2)
HG=$(grep '\<HG\>' ${config} | cut -d':' -f2)
Ped=$(grep '\<Ped\>' ${config} | cut -d':' -f2)
list_file=${input_file/Info/list}
Size=$(ls -lh $list_file | awk '{print $5}')
PedRun=$(grep '\<PedRun\>' ${config} | cut -d':' -f2)
TrgRate=$(grep '\<TrgRate\>' ${config} | cut -d':' -f2)
latestId=$(caliDB latest | cut -d':' -f2)

echo -e "INFO\trun ${input_file} will be insert as run $((latestId+1))"

caliDB insert \
    --Type $run_type	\
    --Flag $Flag	\
    --StartTime "$StartTime" \
    --StopTime "$StopTime"   \
    --Length $Length	    \
    --Geometry $Geometry    \
    --Channels $Channels    \
    --Trigger $Trigger	    \
    --T1 ${T1}		    \
    --T2 ${T2}		    \
    --T3 ${T3}		    \
    --T4 ${T4}		    \
    --Events $Events	    \
    --LG $LG	    \
    --HG $HG	    \
    --Ped $Ped	    \
    --Size $Size    \
    --PedRun $PedRun	    \
    --TrgRate $TrgRate	    \
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
