#!/usr/bin/env bash


REPO_BASE="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )/.." &> /dev/null && pwd )"
# echo "REPO_BASE ${REPO_BASE}"



shopt -s globstar


ls \
    ${REPO_BASE}/RemPythonClient/**/*.py \
    ${REPO_BASE}/RemPythonClient/**/*.sh \
    ${REPO_BASE}/ServerPython/**/*.py \
    ${REPO_BASE}/ServerPython/**/*.sh \
    ${REPO_BASE}/RemPythonCommon/**/*.py \
    \
    | entr -cr \
    \
    bash ${REPO_BASE}/ServerPython/run.sh




