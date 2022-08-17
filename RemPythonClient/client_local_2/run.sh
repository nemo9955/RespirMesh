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




client_path="${REPO_BASE}/RemPythonClient/client_local_2/main.py"
pids=()


function start_client(){
    local sleep_=$1 ; shift
    local device_id=$3
    # local client_port=$5

    # fuser -k -n tcp $((client_port+1))
    # fuser -k -n tcp $((client_port+2))

    sleep ${sleep_}
    python3 ${client_path} "${@}" &
    # echo "redirecting to ${REPO_BASE}/../${device_id}.log"
    # { python3 -u ${client_path} "${@}" & } &> ${REPO_BASE}/../${device_id}.log
    pids+=($!)
}

sleep 1

echo -e "\n"
python3 ${client_path} generate_mesh $1 localhost 30000 localhost 20000
echo -e "\n"

start=$(cat ${1} | jq -r ".start")
stop=$(cat ${1} | jq -r ".stop")

echo "start:${start} stop:${stop}"

for i in $(seq ${start} ${stop});do
    start_client 0.1 run_node $1 $i
done

sleep 1
echo "pids = ${pids[*]}"
for pid_ in ${pids[*]}; do
    wait $pid_
done
