#!/bin/bash
me=$(basename -- "$0")
ROOTDIR=$(realpath $(dirname -- "$0"))
WORKDIR=`pwd`
if [ -z "$DETERCTOR_PATH" ]; then source ${ROOTDIR}/setup.sh; fi

usage(){
    echo "${me} 
    --help		show this help message
    --particle P
    --particleEnergy E	in GeV
    --numberOfEvents N  number of events in each job process 
    --compactFile F
    --steeringFile F
    --inputFile	F	input hepmc file, conflicts with --steeringFile, --particle and --particleEnergy
    --outputDir	dir	output directory
    --outputFileName o	No file extension
    "
}

# Input simulation parameters
OPTIONS=$(getopt --options h --longoptions  \
help,\
particle:,\
particleEnergy:,\
numberOfEvents:,\
compactFile:,\
steeringFile:,\
inputFile:,\
outputDir:,\
outputFileName:, \
--name "${me}" -- "$@")
if [ $? != 0 ] ; then echo "Terminating..." >&2 ; exit 1 ; fi
eval set -- "$OPTIONS"

# default configurations
particle="e-"
particleEnergy=10   # GeV
numberOfEvents=1000
compactFile="${ROOTDIR}/prototype.xml"
steeringFile="${ROOTDIR}/compact/steering.py"
inputFile=''
outputDir=`pwd`
outputFileName="output"

useParticle=false
useParticleEnergy=false
useSteeringFile=false
useInputFile=false

while true; do
    case "$1" in
	-h | --help)	usage;	exit 0 ;;
	--numberOfEvents)   numberOfEvents="$2";    shift 2 ;;
	--compactFile)	    compactFile="$2";	    shift 2 ;;
	--particle)	    particle="$2";	useParticle=true;	shift 2 ;;
	--particleEnergy)   particleEnergy="$2";useParticleEnergy=true;	shift 2 ;;
	--steeringFile)	    steeringFile="$2";	useSteeringFile=true;	shift 2 ;;
	--inputFile)	    inputFile="$2";	useInputFile=true;	shift 2 ;;
	--outputDir)	    outputDir="$2";	    shift 2 ;;
	--outputFileName)   outputFileName="$2";	    shift 2 ;;
	--) shift; break ;;
	*) break ;;
    esac
done

parameter="--compactFile ${compactFile}"
parameter+=" --numberOfEvents ${numberOfEvents}"
if $useInputFile; then
    echo -e "INFO:\tuse inputFiles, ignore configurations about particle, particleEnergy and steeringFile"
    if ! [ -f $inputFile ]; then
	echo -e "ERROR:\tinputFile doesn't exsit: ${inputFile}"
	exit 4
    fi
    parameter+=" --inputFile ${inputFile}"
else
    if ! $useSteeringFile; then
	echo -e "INFO:\tuse default steeringFile: ${steeringFile}"
    fi
    if ! [ -f ${steeringFile} ]; then
	echo -e "ERROR:\tsteeringFile doesn't exist: ${steeringFile}"
	exit 4
    fi
    parameter+=" --steeringFile ${steeringFile}"

    if $useParticle; then
	echo -e "INFO:\tsimulate particle: ${particle}"
	parameter+=" --enableGun --gun.particle \"${particle}\""

	if $useParticleEnergy; then
	    echo -e "INFO:\tbeam energy: ${particleEnergy} GeV"
	    parameter+=" --gun.energy ${particleEnergy}*GeV"
	fi
    fi
fi
simFile="${outputDir}/sim_${outputFileName}.edm4hep.root"
recFile="${outputDir}/rec_${outputFileName}.edm4hep.root"
parameter+=" --outputFile ${simFile}"

echo $LD_LIBRARY_PATH

echo -e "INFO:\trunning simulation with
    \t${parameter}"

# Running simulation
if [ -f $simFile ]; then
    echo -e "INFO:\tsim file already exist, skip simulation: ${simFile}"
else
    ddsim ${parameter} || exit
fi

# Running reconstruction
if [ -f $recFile ]; then
    echo -e "INFO:\treco file already exist, skip reconstruction: ${recFile}"
else
    # export EICrecon_MY=${ROOTDIR}/
    eicrecon -Pplugins=CALI \
	-Pjana:nevents=$numberOfEvents \
	-Pdd4hep:xml_files=${compactFile} \
	-Ppodio:output_file=${recFile} \
	-Ppodio:output_collections=CALIHits,CALIRecHits,CALIImagingTopoClusters,CALIImagingClusters \
	${simFile}
fi

# analysis
myrecFile="${outputDir}/${outputFileName}.myrec.root"
if [ -f "$myrecFile" ]; then
    echo -e "INFO:\toutput file already exist, skip it: $myrecFile"
else
    root -l -q "${ROOTDIR}/macros/make_myrec_tree.C(\"$recFile\", \"$myrecFile\")"
fi
