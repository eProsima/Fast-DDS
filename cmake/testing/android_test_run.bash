#!/bin/bash

test_executable=$1
dest_dir=/data/local/tmp

echo $1
shift

adb shell test -e $dest_dir/$test_executable
if [[ $? != 0 ]]; then
    adb push $test_executable $dest_dir/$test_executable &> /dev/null
    adb shell chmod 755 $dest_dir/$test_executable &> /dev/null
fi
adb shell $dest_dir/$test_executable $@
