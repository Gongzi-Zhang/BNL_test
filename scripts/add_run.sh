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
    echo -e "ERROR\tAt least 1 arguments needed" >&2
    usage
    exit
fi

# info parameters
info_file=$1; shift
list_file=${info_file/Info/list}
if ! [ -f $list_file ]; then
    echo -e "FATAL\tno list file for run $info_file" >&2
    exit 4
fi
nline=$(wc -l $list_file)
if [ $nline -lt 100 ]; then
    echo -e "WARNING\tno event recorded in run $info_file, skip it" >&2
    exit 0
fi

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

StartTime='00'
StopTime=$(ls -l --time-style='+%Y%m%d %H:%M' $list_file | cut -d' ' -f6-7)
Length=-1

if [ -f $info_file ]; then
    StartTime=$(grep '\<Start Time\>' $info_file | cut -d ':' -f2-)
    StartTime=$(extract_datetime "$StartTime")
    StopTime=$(grep '\<Stop Time\>' $info_file | cut -d ':' -f2-)
    StopTime=$(extract_datetime "$StopTime")
    Length=$(grep '\<Elapsed time\>' $info_file | cut -d ' ' -f4)
    Length=$(perl -e "printf(\"%.1f\", $Length/3600)")
fi

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
Events=-1
sync_file=${info_file/Info/sync}
if [ -f $sync_file ]; then
    Events=$(tail -n1 $sync_file | awk '{print $3}')
fi
LG=$(grep '\<LG\>' ${config} | cut -d':' -f2)
HG=$(grep '\<HG\>' ${config} | cut -d':' -f2)
Ped=$(grep '\<Ped\>' ${config} | cut -d':' -f2)
Size=$(ls -lh $list_file | awk '{print $5}')
PedRun=$(grep '\<PedRun\>' ${config} | cut -d':' -f2)
if [ "$run_type" = "ptrg" ]; then
    PedRun=-1
fi
TrgRate=$(grep '\<TrgRate\>' ${config} | cut -d':' -f2)
if [ "$run_type" = "ptrg" ] || [ "$run_type" = "cosmic" ]; then
    TrgRate=0
fi
latestId=$(caliDB latest | cut -d':' -f2)

echo -e "INFO\trun ${info_file} will be insert as run $((latestId+1))"

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
   echo -e "ERROR\tFailed to insert the record" >&2
   exit 4
fi
echo -e "INFO\tsuccessfully insert run ${info_file} into database as run $((latestId+1))"

fdir=$(dirname -- $info_file)
fname=$(basename -- $info_file)
old=${fname%_Info.txt}
latestId=$(caliDB latest | cut -d':' -f2 | tr -d ' ')
new="${CALIROOT}/data/Run$latestId" 
rename "s#${fdir}/$old#$new#" ${fdir}/${old}_{Info,list,sync}.txt
exit 0
