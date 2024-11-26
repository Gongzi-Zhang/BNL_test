#!/bin/bash
me=$(basename -- "$0")
CONDOR=$(realpath $(dirname -- "$0"))
ROOTDIR=$(realpath ${CONDOR}/..)
WORKDIR=`pwd`

usage(){
    echo "${me} 
    --help		    show this help message
    --simDir		    dir. contains hepmc files
    --outputDir dir 
    --countStart c
    "
}

# Input simulation parameters
OPTIONS=$(getopt --options h --longoptions  \
help,\
simDir:,\
outputDir:,\
countStart: \
--name "${me}" \
-- "$@")
if [ $? != 0 ] ; then echo "Terminating..." >&2 ; exit 1 ; fi

eval set -- "$OPTIONS"

simDir=`pwd`
outputDir=`pwd`
count=1

while true; do
    case "$1" in
	-h | --help)	    usage;	exit 0 ;;
	--simDir)	    simDir=$(realpath "$2");	shift 2 ;;
	--outputDir)	    outputDir=$(realpath "$2"); shift 2 ;;
	--countStart)	    count=$2;			shift 2 ;;
	--) shift; break ;;
	*) break ;;
    esac
done

echo -e "INFO\trunning analysis over sim files in ${simDir}"

ls $simDir/sim_*.edm4hep.root | while read f; do
    CONDOR_JOB=condor_${count}.job
    output=${f/.edm4hep/.hit}
    if [ -f "$output" ]; then
	continue
    fi
    # output=${f/.edm4hep/_hist}
    [ -f $CONDOR_JOB ] && rm $CONDOR_JOB
    cat << END >> ${CONDOR_JOB}
Universe        = vanilla
Notification    = Never
Executable      = ${CONDOR}/run_eic.csh
Arguments       = root -l -q '${ROOTDIR}/macros/make_tree.C(\"${f}\", \"${output}\")'
# Arguments       = root -l -q '${ROOTDIR}/macros/calibrate.C(\"${f}\", \"${output}\")'
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
