#!/bin/bash
me=$(basename -- "$0")
CONDOR=$(realpath $(dirname -- "$0"))
ROOTDIR=$(realpath ${CONDOR}/..)
WORKDIR=`pwd`

usage(){
    echo "${me} 
    --help		    show this help message
    --hepmcDir		    dir. contains hepmc files
    --countStart c
    "
}

# Input simulation parameters
OPTIONS=$(getopt --options h --longoptions  \
help,\
hepmcDir:,\
countStart: \
--name "${me}" \
-- "$@")
if [ $? != 0 ] ; then echo "Terminating..." >&2 ; exit 1 ; fi

eval set -- "$OPTIONS"

hepmcDir=`pwd`
count=1

while true; do
    case "$1" in
	-h | --help)	    usage;	exit 0 ;;
	--hepmcDir)	    hepmcDir=$(realpath "$2");	shift 2 ;;
	--countStart)	    count=$2;			shift 2 ;;
	--) shift; break ;;
	*) break ;;
    esac
done

echo -e "INFO\trunning analysis over sim files in ${hepmcDir}"

ls $hepmcDir/*.hepmc | while read f; do
    CONDOR_JOB=condor_${count}.job
    output=${f/.hepmc/_hist.root}
    if [ -f "$output" ]; then
	continue
    fi
    [ -f $CONDOR_JOB ] && rm $CONDOR_JOB
    cat << END >> ${CONDOR_JOB}
Universe        = vanilla
Notification    = Never
Executable      = ${CONDOR}/run_eic.csh
Arguments       = source ${ROOTDIR}/setup.sh; root -l -q '${ROOTDIR}/macros/make_hepmc_hist.C(\"${f}\", \"${output}\")'
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
