#!/bin/bash

if [ $# -lt 1 ]; then
    echo "ERROR\tAt least one parameter needed"
    exit 1
fi

me=$(basename -- "$0")
ROOTDIR=$(realpath $(dirname -- "$0"))
WORKDIR=`pwd`
if [ -z "$DETERCTOR_PATH" ]; then source ${ROOTDIR}/setup.sh; fi

for recFile in $*; do
    myrecFile=${recFile/.edm4hep/.myrec}                                                 
    myrecFile=${myrecFile/rec_/}    
    # if ! [ -f "$myrecFile" ]; then
	# root -l -q "${ROOTDIR}/macros/make_myrec_tree.C(\"$recFile\", \"$myrecFile\" )"
	${ROOTDIR}/../bin/clustering $myrecFile
    # fi
done
