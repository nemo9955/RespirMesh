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




client_path="${REPO_BASE}/RemPythonClient/client_local_1/main.py"
pids=()


function start_client(){
    local sleep_=$1 ; shift
    local device_id=$3
    local client_port=$5

    fuser -k -n tcp $((client_port+1))
    fuser -k -n tcp $((client_port+2))

    sleep ${sleep_}
    python3 ${client_path} "${@}" &
    # echo "redirecting to ${REPO_BASE}/../${device_id}.log"
    # { python3 -u ${client_path} "${@}" & } &> ${REPO_BASE}/../${device_id}.log
    pids+=($!)
}



sleep 0.5
start_client 0.6 localhost 20000  110  localhost 20110
start_client 0.6 localhost 20000  120  localhost 20120
sleep 0.5
start_client 0.6 localhost 20110  210  localhost 20210
sleep 0.5
start_client 0.6 localhost 20210  310  localhost 20310
sleep 0.5
start_client 0.6 localhost 20310  310  localhost 20410

# sleep 0.5
# start_client 0.6 localhost 20000  110  localhost 20110
# start_client 0.6 localhost 20000  120  localhost 20120
# start_client 0.6 localhost 20000  130  localhost 20130
# start_client 0.6 localhost 20000  140  localhost 20140
# sleep 1
# start_client 0.6 localhost 20110  210  localhost 20210
# start_client 0.6 localhost 20120  220  localhost 20220
# start_client 0.6 localhost 20120  230  localhost 20230
# start_client 0.6 localhost 20130  240  localhost 20240
# sleep 1
# start_client 0.6 localhost 20240  310  localhost 20310
# start_client 0.6 localhost 20240  320  localhost 20320

sleep 1
echo "pids = ${pids[*]}"
for pid_ in ${pids[*]}; do
    wait $pid_
done
