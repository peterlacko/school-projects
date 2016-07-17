#!/bin/bash

#pocet cisel bud zadam nebo 8 :)
if [ $# -lt 1 ];then 
    numbers=8;
else
    numbers=$1;
fi;

function log2(){
n=$1
m=1
while [[ $n -gt 2 ]]; do
let n=n/2
let m=m+1
done

return $m
}

#preklad cpp zdrojaku
mpic++ --prefix /usr/local/share/OpenMPI -o pms pms.cpp &>/dev/null

#vyrobeni souboru s random cisly
dd if=/dev/urandom bs=1 count=$numbers of=numbers &>/dev/null

log2 $numbers
numbers=`expr $? + 1`

#spusteni
mpirun --prefix /usr/local/share/OpenMPI -np $numbers pms

#uklid
rm pms
rm numbers
