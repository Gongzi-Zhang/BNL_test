script=$(basename -- ${BASH_SOURCE[0]})

export SSH_AUTH_SOCK=/run/user/1000/keyring/ssh

black="\033[30;20m"
red="\033[31;20m"
green="\033[32;20m"
yellow="\033[33;20m"
blue="\033[34;20m"
bold_red="\033[31;1m"
reset="\033[0m"

declare -A color=( \
    [debug]=$blue \
    [info]=$green \
    [warning]=$yellow \
    [error]=$red \
    [fatal]=$bold_red \
)

logger()
{
    level=$1; shift
    echo -e "($script:$LINENO) ${color[$level]} ${level^^} $reset" $@
}

test()
{
    logger debug "debug"
    logger info "info"
    logger warning "warning"
    logger error "error"
    logger fatal "fatal"
}
