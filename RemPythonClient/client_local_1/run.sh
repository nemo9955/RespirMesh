#!/usr/bin/env bash

REPO_BASE="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )/../.." &> /dev/null && pwd )"
echo "REPO_BASE ${REPO_BASE}"

trap ctrl_c INT SIGHUP SIGINT SIGTERM
function ctrl_c() {
    # pkill -P $$

    for pid_ in ${pids[*]}; do
        echo "Killing/terminating ${pid_}"
        kill $pid_
        # kill -TERM $pid_
        # kill -INT $pid_
    done
}


fuser -k -n tcp 20121
fuser -k -n udp 20122
fuser -k -n tcp 20111
fuser -k -n udp 20112
fuser -k -n tcp 20211
fuser -k -n udp 20212

# kill $(sudo lsof -t -i:20121)
# kill $(sudo lsof -t -i:20122)
# kill $(sudo lsof -t -i:20111)
# kill $(sudo lsof -t -i:20112)
# kill $(sudo lsof -t -i:20211)
# kill $(sudo lsof -t -i:20212)




# ufw deny 20121
# ufw deny 20122
# ufw deny 20111
# ufw deny 20112
# ufw deny 20211
# ufw deny 20212

sleep 0.3 # just so server has time to init in case both refresh

client_path="${REPO_BASE}/RemPythonClient/client_local_1/main.py"
pids=()


if [[ "${@}" == *"1"* ]] ; then
    sleep 0.2
    python3 ${client_path} localhost 20000  110  localhost 20110 &
    # { python3 -u ${client_path} localhost 20000  110  localhost 20110 & } &> ${REPO_BASE}/../110.log
    echo "redirecting to ${REPO_BASE}/../110.log"
    pids+=($!)
fi



if [[ "${@}" == *"2"* ]] ; then
    sleep 0.5
    python3 ${client_path} localhost 20000  120  localhost 20120 &
    # { python3 -u ${client_path} localhost 20000  120  localhost 20120 & } &> ${REPO_BASE}/../120.log
    echo "redirecting to ${REPO_BASE}/../120.log"
    pids+=($!)
fi

if [[ "${@}" == *"3"* ]] ; then
    sleep 1
    python3 ${client_path} localhost 20110  210  localhost 20210 &
    # { python3 -u ${client_path} localhost 20110  210  localhost 20210 & } &> ${REPO_BASE}/../210.log
    echo "redirecting to ${REPO_BASE}/../210.log"
    pids+=($!)
fi


sleep 0.5
echo "pids = ${pids[*]}"
for pid_ in ${pids[*]}; do
    wait $pid_
done
