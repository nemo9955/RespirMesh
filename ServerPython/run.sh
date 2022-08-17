#!/usr/bin/env bash


REPO_BASE="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )/.." &> /dev/null && pwd )"
# echo "REPO_BASE ${REPO_BASE}"

# echo -e " \n\n\n===============================\n"

fuser -k -n tcp 20001
fuser -k -n udp 20002

python3 -u ${REPO_BASE}/ServerPython/server.py \
    localhost 20000 \
    root




