epic=/opt/detector/epic-main/bin/thisepic.sh
if [ -f $epic ]; then
    source $epic
fi
export CALISIMROOT=$(realpath -- $(dirname -- ${BASH_SOURCE[0]}))
export EICrecon_MY=${CALISIMROOT}

function add_c_include_path {
    for path in $@; do
	if [ -z "$CPLUS_INCLUDE_PATH" ]; then
	    export CPLUS_INCLUDE_PATH=$path
	else
	    if ! [[ "$CPLUS_INCLUDE_PATH" =~ "$path" ]]; then
		export CPLUS_INCLUDE_PATH=$path:$CPLUS_INCLUDE_PATH
	    fi
	fi
    done
}

function add_c_library_path {
    for path in $@; do
	if [ -z "$LD_LIBRARY_PATH" ]; then
	    export LD_LIBRARY_PATH=$path
	else
	    if ! [[ "$LD_LIBRARY_PATH" =~ "$path" ]]; then
		export LD_LIBRARY_PATH=$path:$LD_LIBRARY_PATH
	    fi
	fi
    done
}

add_c_include_path ${CALISIMROOT}/include
add_c_library_path ${CALISIMROOT}/dd4hep/lib
