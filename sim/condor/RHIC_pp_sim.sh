#!/bin/bash
me=$(basename -- "$0")
CONDOR=$(realpath $(dirname -- "$0"))
ROOTDIR=$(realpath ${CONDOR}/..)
WORKDIR=`pwd`

usage(){
    echo "${me} 
    --help		    show this help message
    --numberOfEvents	    number of events in one hepmc file
    --compactFile F
    --hepmcDir		    dir. contains hepmc files
    --outputDir dir 
    "
}

# Input simulation parameters
OPTIONS=$(getopt --options h --longoptions  \
help,\
numberOfEvents:,\
compactFile:,\
hepmcDir:,\
outputDir: \
--name "${me}" \
-- "$@")
if [ $? != 0 ] ; then echo "Terminating..." >&2 ; exit 1 ; fi

eval set -- "$OPTIONS"

numberOfEvents=1000
compactFile="prototype.xml"
hepmcDir=`pwd`
outputDir=`pwd`

while true; do
    case "$1" in
	-h | --help)	    usage;	exit 0 ;;
	--numberOfEvents)   numberOfEvents="$2";    shift 2 ;;
	--compactFile)	    compactFile=$(realpath "$2");   shift 2 ;;
	--hepmcDir)	    hepmcDir=$(realpath "$2");	shift 2 ;;
	--outputDir)	    outputDir=$(realpath "$2");   shift 2 ;;
	--) shift; break ;;
	*) break ;;
    esac
done

[ -d $outputDir ] || mkdir -p $outputDir
echo -e "INFO\trunning simulations over hepmc files in ${hepmcDir}"

i=1
ls $hepmcDir/*.hepmc | while read file; do
    hepmc=$(basename $file)
    CONDOR_JOB=condor_${i}.job
    [ -f $CONDOR_JOB ] && rm $CONDOR_JOB
    cat << END >> ${CONDOR_JOB}
Universe        = vanilla
Notification    = Never
Executable      = ${CONDOR}/run_eic.csh
Arguments       = ${ROOTDIR}/run.sh \
    --numberOfEvents ${numberOfEvents}\
    --compactFile ${compactFile}\
    --inputFile ${file}\
    --outputDir ${outputDir}\
    --outputFileName ${hepmc%.hepmc}
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
    let i++
done
