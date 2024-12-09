#!/bin/bash

# generating hepmc file for pp collisions

me=$(basename -- "$0")
CONDOR=$(realpath $(dirname -- "$0"))
ROOTDIR=$(realpath ${CONDOR}/..)
WORKDIR=`pwd`
# Executable='RHIC_pp'
Executable="${ROOTDIR}/RHIC/src/gen.sh"

usage(){
    echo "${me} 
    --help		    show this help message
    --numberOfJobs N
    --inputFile		    input cmnd file
    --outputName	    output hepmc file name prefix
    --outputDir dir 
    --countStart c
    "
}

# Input simulation parameters
OPTIONS=$(getopt --options h --longoptions  \
help,\
numberOfJobs:,\
inputFile:,\
outputName:,\
outputDir:,\
countStart: \
--name "${me}" \
-- "$@")
if [ $? != 0 ] ; then echo "Terminating..." >&2 ; exit 1 ; fi

eval set -- "$OPTIONS"

numberOfJobs=100
inputFile="${ROOTDIR}//RHIC/src/RHIC_pp_hard.cmnd"
outputName='RHIC_pp'
outputDir=`pwd`
count=1

while true; do
    case "$1" in
	-h | --help)	    usage;	exit 0 ;;
	--numberOfJobs)	    numberOfJobs=$2;		shift 2 ;;
	--inputFile)	    inputFile=$(realpath "$2");	shift 2 ;;
	--outputName)	    outputName="$2";		shift 2 ;;
	--outputDir)	    outputDir=$(realpath "$2"); shift 2 ;;
	--countStart)	    count=$2;			shift 2 ;;
	--) shift; break ;;
	*) break ;;
    esac
done

[ -d $outputDir ] || mkdir -p $outputDir
echo -e "INFO\tproducing ${numberOfJobs} hepmc files with ${inputFile}"

for ((i=1; i<=$numberOfJobs; i++))
do
    CONDOR_JOB=condor_${count}.job
    output=${outputDir}/${outputName}_${count}.hepmc
    [ -f $CONDOR_JOB ] && rm $CONDOR_JOB
    cat << END >> ${CONDOR_JOB}
Universe        = vanilla
Notification    = Never
Executable      = ${Executable}
Arguments       = $inputFile ${output}
Requirements    = (CPU_Speed >= 2)
Rank		= CPU_Speed
request_memory  = 2GB
request_cpus    = 1
Priority        = 20
GetEnv          = False
Initialdir      = ${WORKDIR}
# Input           = ${ROOTDIR}/backwards_insert.xml
# transfer_input_files = file1,file2
Output          = \$(ClusterID)_\$(ProcID).out
Error           = \$(ClusterID)_\$(ProcID).err
Log             = \$(ClusterID)_\$(ProcID).log
should_transfer_files = YES
when_to_transfer_output = ON_EXIT_OR_EVICT
PeriodicHold    = (NumJobStarts >= 1 && JobStatus == 1)
Notify_user     = weibinz@ucr.edu
Queue 1
END
    echo -e "INFO\tsubmitting job for hepmc file: ${hepmc}"
    condor_submit ${CONDOR_JOB}
    let count++
done
