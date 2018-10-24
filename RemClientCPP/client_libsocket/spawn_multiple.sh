#!/bin/bash

CDIR="$( cd "$(dirname "$0")" ; pwd -P )"

echo $CDIR

start_server_port=10000

if [[ ! -e $CDIR/ports.txt ]] ; then
    echo 9995 > $CDIR/ports.txt
    echo 9995 > $CDIR/ports.txt
    echo 9995 > $CDIR/ports.txt
    echo $start_server_port >> $CDIR/ports.txt
fi



function lst_port(){
    tail -1 $CDIR/ports.txt
}

function nxt_port(){
    start_server_port=$(expr $(tail -1 $CDIR/ports.txt  ) + 1 )
    echo $start_server_port >> $CDIR/ports.txt
    tail -1 $CDIR/ports.txt
}

function rnd_ports(){
    the_clients=" "
    the_clients+=$(echo " \t localhost \t " $(tail -10 $CDIR/ports.txt| head -9  | shuf | tail -1))
    the_clients+=$(echo " \t localhost \t " $(tail -10 $CDIR/ports.txt| head -9  | shuf | tail -1))
    the_clients+=$(echo " \t localhost \t " $(tail -10 $CDIR/ports.txt| head -9  | shuf | tail -1))
    echo -e "$the_clients"
}

# echo "0.0.0.0" $(nxt_port) $(lst_port)     $(rnd_ports)
# echo "0.0.0.0" $(nxt_port) $(lst_port)     $(rnd_ports)
# echo "0.0.0.0" $(nxt_port) $(lst_port)     $(rnd_ports)
# echo "0.0.0.0" $(nxt_port) $(lst_port)     $(rnd_ports)
# echo "0.0.0.0" $(nxt_port) $(lst_port)     $(rnd_ports)
# echo "0.0.0.0" $(nxt_port) $(lst_port)     $(rnd_ports)
# echo "0.0.0.0" $(nxt_port) $(lst_port)     $(rnd_ports)


cd $CDIR && make

echo " ******************************************* "
echo " ******************************************* "
echo " ******************************************* "
echo " ******************************************* "
echo ""
echo ""
echo ""
echo ""
echo ""

# set -o xtrace

for i in {1..10} ; do
    sleep 0.1 ; ($CDIR/client_libsocket.o "0.0.0.0" $(nxt_port) $(lst_port)     $(rnd_ports)         )&
done

[[ $1 ]] || read asdf

rm $CDIR/ports.txt
