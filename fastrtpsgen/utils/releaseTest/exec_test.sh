#!/bin/bash
# This scripts run the fastrtpsgen tests.

errorstatus=0
globalerrorstatus=0
configuration=x64Linux2.6gcc
failedtests=""
correcttests=""


# This function execute a test in a directory.
# @param Plaform for Visual Studio.
# @param Name of the test
function execTest
{
	#Get the IDL files:
	IDLFILES=""
	for i in $(ls);do
    	if [[ $i =~ .*\.idl$ ]];then
    	    IDLFILES="$IDLFILES $i" 
    	fi
	done
	echo "FOUND IDL FILES: $IDLFILES"
	#Generate Info
	cp ../../../lib/fastrtpsgen.jar .
	java -jar fastrtpsgen.jar -example $configuration -replace $IDLFILES
	errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
	#If the makefile is not present than can be OK.
	if [ ! -f "makefile_$configuration" ]; then return; fi
	#Compile
	make -f makefile_$configuration all
	errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
	#Remove all files:
	if [ $2 == "nodelete" ]; then return; fi
	rm -R bin/
	rm -R output/
	rm -R lib/
	rm *.cxx
	rm *.h
	rm makefile*
	rm *.jar
}

if [ $# -ge 1 ]; then
	cd $1
	execTest $1 $2
	cd ..
	if [ $errorstatus == 0 ]; then
		echo "TEST $1 SUCCESSFULL"
	else
	    echo "TEST $1 FAILED"
		globalerrorstatus=1
		failedtests="$failedtests $1"
	fi
else
	echo "EXECUTING ALL TESTS IN THIS DIRECTORY"
	for dir in $(find . -mindepth 1 -maxdepth 1 -path ./output -prune -o -path ./.svn -prune -o -type d -printf "%f\n"); do
		echo "EXECUTING IN DIRECTORY $dir"
		cd $dir
		execTest $1 $2
		cd ..
		if [ $errorstatus == 0 ]; then
		    echo "TEST $dir SUCCESSFULL"
			correcttests="$correcttests $dir"
		else
		    echo "TEST $dir FAILED"
			globalerrorstatus=1
			failedtests="$failedtests $dir"
		fi
	done	
fi

if [ $globalerrorstatus == 0 ]; then
    echo "TEST SUCCESSFULL"
else
    echo "TESTS FAILED: $failedtests"
	echo "TESTS OK: $correcttests"
fi

exit $globalerrorstatus

