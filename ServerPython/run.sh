#!/usr/bin/env bash


REPO_BASE="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )/.." &> /dev/null && pwd )"
# echo "REPO_BASE ${REPO_BASE}"

python3 ${REPO_BASE}/ServerPython/server.py \
    localhost 20000 \
    10



