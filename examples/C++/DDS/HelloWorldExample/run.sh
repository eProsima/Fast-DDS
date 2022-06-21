#!/bin/bash
set -x

run_index=1

while true
do

echo "Run ${run_index}"

./DDSHelloWorldExample publisher 100 > pub.log &

pub_pid=$(ps -ef | grep "DDSHelloWorldExample publisher" | grep -v grep | awk '{print $2}')

sleep 1

./DDSHelloWorldExample subscriber | tee sub.log &

sub_pid=$(ps -ef | grep "DDSHelloWorldExample subscriber" | grep -v grep | awk '{print $2}')

wait ${pub_pid}

sync

kill ${sub_pid}

grep 'Fail:' sub.log >/dev/null 2>&1
if [ $? -eq 0 ];then
	break
fi

let "run_index+=1"

done
