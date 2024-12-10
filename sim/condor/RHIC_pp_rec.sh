#!/bin/bash
me=$(basename -- "$0")
CONDOR=$(realpath $(dirname -- "$0"))
ROOTDIR=$(realpath ${CONDOR}/..)
WORKDIR=`pwd`

usage(){
    echo "${me} 
    --help		    show this help message
    --numberOfEvents N
    --compactFile f
    --rootDir		    dir. contains root files
    --outputDir dir 
    "
}

# Input simulation parameters
OPTIONS=$(getopt --options h --longoptions  \
help,\
numberOfEvents:,\
compactFile:,\
rootDir:,\
outputDir: \
--name "${me}" \
-- "$@")
if [ $? != 0 ] ; then echo "Terminating..." >&2 ; exit 1 ; fi

eval set -- "$OPTIONS"

numberOfEvents=1000
compactFile=${ROOTDIR}/prototype.xml
rootDir=`pwd`
outputDir=`pwd`

while true; do
    case "$1" in
	-h | --help)	    usage;	exit 0 ;;
	--numberOfEvents)   numberOfEvents=$2; shift 2;;
	--compactFile)	    compactFile=$(realpath "$2"); shift 2;;
	--rootDir)	    rootDir=$(realpath "$2");	shift 2 ;;
	--outputDir)	    outputDir=$(realpath "$2");   shift 2 ;;
	--) shift; break ;;
	*) break ;;
    esac
done

i=1
ls $rootDir/sim_*.edm4hep.root | while read simFile; do
    CONDOR_JOB=condor_${i}.job
    recFile=${simFile/sim_/rec_}
    [ -f $CONDOR_JOB ] && rm $CONDOR_JOB
    cat << END >> ${CONDOR_JOB}
Universe        = vanilla
Notification    = Never
Executable      = ${CONDOR}/run_eic.csh
Arguments       = source ${ROOTDIR}/setup.sh && eicrecon -Pplugins=CALI \
	-Pjana:nevents=${numberOfEvents} \
	-Pdd4hep:xml_files=${compactFile} \
	-Ppodio:output_file=${recFile} \
	-Ppodio:output_include_collections=CALIHits,CALIRawHits,CALIRecHits,CALIImagingTopoClusters,CALIImagingClusters \
	${simFile}
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
    echo -e "INFO\tsubmitting job for root file: ${file}"
    condor_submit ${CONDOR_JOB}
    let i++
done
