export CALIROOT=$(realpath -- $(dirname -- ${BASH_SOURCE[0]}))
if ! [[ "$PATH" =~ "$CALIROOT" ]]; then
    export PATH=$PATH:${CALIROOT}/bin
fi

# python path
if [ -z "${PYTHONPATH}" ]; then
    PYTHONPATH=${CALIROOT}/lib
else
    PYTHONPATH=${CALIROOT}/lib:$PYTHONPATH
fi

# c include path
if [ -z "${CPLUS_INCLUDE_PATH}" ]; then
    CPLUS_INCLUDE_PATH=${CALIROOT}/include
else
    CPLUS_INCLUDE_PATH=${CALIROOT}/include:$CPLUS_INCLUDE_PATH
fi
