export CALIROOT=$(realpath -- $(dirname -- ${BASH_SOURCE[0]}))
if ! [[ "$PATH" =~ "$CALIROOT" ]]; then
    export PATH=$PATH:${CALIROOT}/bin
fi
