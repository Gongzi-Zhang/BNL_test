#!/bin/bash

script=$(realpath -- ${BASH_SOURCE[0]})
ROOT=$(dirname $script)/..
DISK="/media/arratialab/CALI/BNL_test/data"

cd "$ROOT/data"
ls Run* | while read f; do
    if ! [ -f "$DISK/$f" ] || [ "$f" -nt "$DISK/$f" ]; then
	cp "$f" "$DISK/"
    fi
done

find . -size +999M -exec rm {} \;
