#!/bin/bash

CDIR="$( cd "$(dirname "$0")" ; pwd -P )"

echo $CDIR

cd $CDIR && make

($CDIR/client_t.o 9995 9001 0)&
($CDIR/client_t.o 9995 9002 1)&
($CDIR/client_t.o 9995 9003 2)&
# ($CDIR/client_t.o 9995 9004 4521)&
# ($CDIR/client_t.o 9995 9005 5521)&
sleep 1
($CDIR/client_t.o 9001 9011 3)&
($CDIR/client_t.o 9001 9012 4)&
# ($CDIR/client_t.o 9001 9013 916711)&
# ($CDIR/client_t.o 9001 9014 176811)&
# sleep 1
# ($CDIR/client_t.o 9011 9021 5)&
# ($CDIR/client_t.o 9011 9022 6)&
# ($CDIR/client_t.o 9011 9023 7)&
# ($CDIR/client_t.o 9011 9024 179811)&
# ($CDIR/client_t.o 9011 9025 456811)&
# sleep 1
# ($CDIR/client_t.o 9995 9023 9)&
# ($CDIR/client_t.o 9001 9023 9)&
# ($CDIR/client_t.o 9011 9023 9)&
# ($CDIR/client_t.o 9003 9023 9)&
# ($CDIR/client_t.o 9022 9023 9)&
# ($CDIR/client_t.o 9024 9023 9)&
# ($CDIR/client_t.o 9004 9023 9)&
# sleep 1
# ($CDIR/client_t.o 9023 9033 8)&

