#!/bin/bash

CDIR="$( cd "$(dirname "$0")" ; pwd -P )"

echo $CDIR

cd $CDIR && make

($CDIR/client_t.o 9995 9001 1564)&
($CDIR/client_t.o 9995 9002 254)&
($CDIR/client_t.o 9995 9003 3521)&
($CDIR/client_t.o 9995 9004 4521)&
($CDIR/client_t.o 9995 9005 5521)&
sleep 1
($CDIR/client_t.o 9001 9011 111)&
($CDIR/client_t.o 9002 9012 111)&


