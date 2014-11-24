#!/bin/sh

#: This script packs eRTPS product for any platform in Linux.
#
# This script needs the next programs to be run.
# - subversion
# - libreoffice
# - ant
# - doxygen
# Also this script needs the eProsima.documentation.changeVersion macro installed in the system.
#
# Fedora 20: This script has to be run in a Fedora 20 x64.
#            The system has to have installed: libgcc.i686 and libstdc++.i686

errorstatus=0

function package
{
    
	# Get the current version of eRTPS
	. thirdparty/dev-env/scripts/common_pack_functions.sh getVersionFromCPP ertpsversion include/fastrtps/fastrtps_version.h
	errorstatus=$?
	if [ $errorstatus != 0 ]; then return; fi

	# Compile eRTPS for i86.
	rm -rf output
	EPROSIMA_TARGET="i86Linux2.6gcc"
	COMP="g++"
	rm -rf lib/$EPROSIMA_TARGET
	EPROSIMA_TARGET=${EPROSIMA_TARGET} COMP=${COMP} make
	errorstatus=$?
	if [ $errorstatus != 0 ]; then return; fi

	# Compile eRTPS for x64.
	rm -rf output
	EPROSIMA_TARGET="x64Linux2.6gcc"
	COMP="g++"
	rm -rf lib/$EPROSIMA_TARGET
	EPROSIMA_TARGET=${EPROSIMA_TARGET} COMP=${COMP} make
	errorstatus=$?
	if [ $errorstatus != 0 ]; then return; fi

	# Compile eRTPS for ARM.
	rm -rf output
	EPROSIMA_TARGET="armelf_linux_eabi"
	COMP="arm-unknown-linux-gnueabi-g++"
	#rm -rf lib/$EPROSIMA_TARGET
	#EPROSIMA_TARGET=${EPROSIMA_TARGET} COMP=${COMP} make
	errorstatus=$?
	if [ $errorstatus != 0 ]; then return; fi

	# Create PDFS from documentation.
	cd doc	# Installation manual
	soffice --headless "macro:///eProsima.documentation.changeVersion($PWD/RTPS - Installation Manual.odt,$ertpsversion)"
	errorstatus=$?
	if [ $errorstatus != 0 ]; then return; fi
	cp "RTPS - Installation Manual.pdf" $PWD/pdf/
	errorstatus=$?
	if [ $errorstatus != 0 ]; then return; fi
	rm "RTPS - Installation Manual.pdf"
	errorstatus=$?
	if [ $errorstatus != 0 ]; then return; fi
	# User manual
	soffice --headless "macro:///eProsima.documentation.changeVersion($PWD/RTPS - User Manual.odt,$ertpsversion)"
	errorstatus=$?
	if [ $errorstatus != 0 ]; then return; fi
	cp "RTPS - User Manual.pdf" $PWD/pdf/
	errorstatus=$?
	if [ $errorstatus != 0 ]; then return; fi
	rm "RTPS - User Manual.pdf"
	errorstatus=$?
	if [ $errorstatus != 0 ]; then return; fi
	# RTPSGEN User manual
	soffice --headless "macro:///eProsima.documentation.changeVersion($PWD/RTPSGEN - User Manual.odt,$ertpsversion)"
	errorstatus=$?
	if [ $errorstatus != 0 ]; then return; fi
	cp "RTPSGEN - User Manual.pdf" $PWD/pdf/
	errorstatus=$?
	if [ $errorstatus != 0 ]; then return; fi
	rm "RTPSGEN - User Manual.pdf"
	errorstatus=$?
	if [ $errorstatus != 0 ]; then return; fi
	cd ..

	# Create README
	soffice --headless "macro:///eProsima.documentation.changeHyperlinksAndVersionToHTML($PWD/README.odt,$ertpsversion,./doc/,./)"
	errorstatus=$?
	if [ $errorstatus != 0 ]; then return; fi

	# Create doxygen information.
	# Export version
	export VERSION_DOX=$ertpsversion
	mkdir -p doc/html
	mkdir -p utils/doxygen/output
	mkdir -p utils/doxygen/output/doxygen
	cd utils/doxygen
	doxygen doxyfile_public_api
	errorstatus=$?
	if [ $errorstatus != 0 ]; then return; fi
	# Compile the latex document
	cd output/doxygen/latex
	make
	errorstatus=$?
	if [ $errorstatus != 0 ]; then return; fi
	cd ../../../../../

	# Build utilities
	#cd utils/ShapesDemo
	#qmake ShapesDemo.pro -r -spec linux-g++-64

	cd rtpsgen
	ant jars
	cd ..
	

	# Create installers
	cd utils/installers/ertps/linux
	./setup_linux.sh $ertpsversion
	errorstatus=$?
	cd ../../../../
	if [ $errorstatus != 0 ]; then return; fi

	# Remove the doxygen tmp directory
	rm -rf utils/doxygen/output


	

}

# Check that the environment.sh script was run.
#if [ "$EPROSIMADIR" == "" ]; then
#    echo "environment.sh must to be run."
#    exit -1
#fi

# Go to root
cd ../..

package

if [ $errorstatus == 0 ]; then
    echo "PACKAGING SUCCESSFULLY"
else
    echo "PACKAGING FAILED"
fi

#	exit $errorstatus


