export CALIROOT=$(realpath -- $(dirname -- ${BASH_SOURCE[0]}))
if ! [[ "$PATH" =~ "$CALIROOT" ]]; then
    export PATH=$PATH:${CALIROOT}/bin
fi

# python path
if [ -z "${PYTHONPATH}" ]; then
    export PYTHONPATH=${CALIROOT}/lib
else
    if ! [[ "$PYTHONPATH" =~ "${CALIROOT}/lib" ]]; then
	export PYTHONPATH=${CALIROOT}/lib:$PYTHONPATH
    fi
fi

# c include path
if [ -z "${CPLUS_INCLUDE_PATH}" ]; then
    export CPLUS_INCLUDE_PATH=${CALIROOT}/include
else
    if ! [[ "$CPLUS_INCLUDE_PATH" =~ "${CALIROOT}/include" ]]; then
	export CPLUS_INCLUDE_PATH=${CALIROOT}/include:$CPLUS_INCLUDE_PATH
    fi
fi
