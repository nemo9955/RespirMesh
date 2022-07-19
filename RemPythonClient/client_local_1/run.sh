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

sleep 0.1 # just so server has time to init in case both refresh

# kill $(sudo lsof -t -i:20011); kill $(sudo lsof -t -i:20012); kill $(sudo lsof -t -i:20021); kill $(sudo lsof -t -i:20022);
client_path="${REPO_BASE}/RemPythonClient/client_local_1/main.py"
pids=()

if [[ "${@}" == *"1"* ]] ; then
    sleep 0.1
    # python3 ${client_path} localhost 20000  110  localhost 20110 &
    { python3 ${client_path} localhost 20000  110  localhost 20110 & } &> ${REPO_BASE}/../110.log
    echo "redirecting to ${REPO_BASE}/../110.log"
    pids+=($!)
fi



if [[ "${@}" == *"2"* ]] ; then
    sleep 0.1
    # python3 ${client_path} localhost 20000  120  localhost 20120 &
    { python3 ${client_path} localhost 20000  120  localhost 20120 & } &> ${REPO_BASE}/../120.log
    echo "redirecting to ${REPO_BASE}/../120.log"
    pids+=($!)
fi

if [[ "${@}" == *"3"* ]] ; then
    sleep 0.1
    # python3 ${client_path} localhost 20110  210  localhost 20210 &
    { python3 ${client_path} localhost 20110  210  localhost 20210 & } &> ${REPO_BASE}/../210.log
    echo "redirecting to ${REPO_BASE}/../210.log"
    pids+=($!)
fi


sleep 0.1
echo "pids = ${pids[*]}"
for pid_ in ${pids[*]}; do
    wait $pid_
done
