#!/bin/sh

# This scripts creates a tar.gz file with all the linux installation.
# Also it creates a RPM package.
# @param The version of the project

# To create the source tar file you have to install next packages:
# autoconf, automake, libtool

# To create RPM in Fedora you have to follow this section on the link:
#   https://fedoraproject.org/wiki/How_to_create_an_RPM_package#Preparing_your_system

# To create RPM in CentOs you have to follow this link:
#   http://wiki.centos.org/HowTos/SetupRpmBuildEnvironment

project="eprosimartps"


installer()
{
    # Copy licenses.
    cp ../../../../doc/licenses/EPROSIMARTPS_LIBRARY_LICENSE.txt tmp/$project
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cp ../../../../doc/licenses/LGPLv3_LICENSE.txt tmp/$project
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # Copy documentation.
    mkdir -p tmp/$project/doc
    mkdir -p tmp/$project/doc/pdf
    mkdir -p tmp/$project/doc/doxygen/public_api
    cp "../../../../doc/pdf/RTPS - Installation Manual.pdf" tmp/$project/doc/pdf/
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cp "../../../../doc/pdf/RTPS - User Manual.pdf" tmp/$project/doc/pdf/
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cp "../../../../doc/index.html" tmp/$project/doc/
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cp -r "../../../../output/doxygen/public_api/html" tmp/$project/doc/doxygen/public_api
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    #cp "../../../../output/doxygen/latex/refman.pdf" "tmp/$project/doc/pdf/eProsimaRTPS - API C++ Manual.pdf"
    #errorstatus=$?
    #if [ $errorstatus != 0 ]; then return; fi

    # Copy README
    cp ../../../../doc/README.html tmp/$project/
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # Copy example.
    mkdir -p tmp/$project/examples/C++
    cp -r ../../../../utils/useTests/* tmp/$project/examples/C++
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # Copy ShapesDemo
    mkdir -p tmp/$project/examples/ShapesDemo
    cp -r ../../../../utils/ShapesDemo/release/linux/* tmp/$project/examples/ShapesDemo
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # Copy RTPSGEN scripts and jar
    mkdir -p tmp/$project/rtpsgen
    cp -r ../../../../rtpsgen/scripts/* tmp/$project/rtpsgen
    cp    ../../../../rtpsgen/lib/* tmp/$project/rtpsgen    
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # M4 files
    mkdir -p tmp/$project/m4
    cp -r m4/* tmp/$project/m4
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # eProsimaRTPS headers
    mkdir -p tmp/$project/include
    cp -r ../../../../include/eprosimartps tmp/$project/include
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # Copy eProsima header files
    mkdir -p tmp/$project/include/eprosimartps/eProsima_cpp
    cp $EPROSIMADIR/code/eProsima_cpp/eProsima_auto_link.h tmp/$project/include/eprosimartps/eProsima_cpp
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cp $EPROSIMADIR/code/eProsima_cpp/eProsimaMacros.h tmp/$project/include/eprosimartps/eProsima_cpp
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # Copy eProsimaRTPS sources
    cd ../../../..
    cp --parents `cat building/makefiles/eprosimartps_sources` utils/installers/autoconf/linux/tmp/$project
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    mkdir -p utils/installers/autoconf/linux/tmp/$project/src/cpp/submessages/
    cp --parents src/cpp/submessages/* utils/installers/autoconf/linux/tmp/$project/
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cd utils/installers/autoconf/linux

    # Copy autoconf configuration files.
    cp configure.ac tmp/$project/configure.ac
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cp Makefile.am tmp/$project/Makefile.am
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cp include_Makefile.am tmp/$project/include/Makefile.am
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # Generate autoconf files
    cd tmp/$project
    sed -i "s/VERSION/${version}/g" configure.ac
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    sed -i "s/VERSION_MAJOR/`echo ${version} | cut -d. -f1`/g" Makefile.am
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    sed -i "s/VERSION_MINOR/`echo ${version} | cut -d. -f2`/g" Makefile.am
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    sed -i "s/VERSION_RELEASE/`echo ${version} | cut -d. -f3`/g" Makefile.am
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    sourcefiles=$(cat ../../../../../../building/makefiles/eprosimartps_sources | sed -e ':a;N;$!ba;s/\n/ /g')
    sed -i -e "s#EPROSIMARTPS_SOURCES#$sourcefiles#" Makefile.am
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    includefiles=$(cat ../../../../../../building/includes/eprosimartps_includes | sed -e 's#^#eprosimartps/#')
    includefiles=$(echo $includefiles | sed -e ':a;N;$!ba;s/\n/ /g')
    #includefiles+=" eprosimartps/eProsima_auto_link.h eprosimartps/eprosimartps_dll.h"
    sed -i -e "s#INCLUDE_FILES#$includefiles#" include/Makefile.am
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    autoreconf --force --install
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cd ../..

    # Erase backup files from vim
    find tmp/ -iname "*~" -exec rm -f {} \;

    cd tmp
    tar cvzf "../eProsima_RTPS_${version}-Linux.tar.gz" $project
    errorstatus=$?
    cd ..
    if [ $errorstatus != 0 ]; then return; fi
}


if [ $# -lt 1 ]; then
    echo "Needs as parameter the version of the product $project"
    exit -1
fi

version=$1

# Create the temporaly directory.
mkdir tmp
mkdir tmp/$project

installer

# Remove temporaly directory
rm -rf tmp

if [ $errorstatus != 0 ]; then
    echo "INSTALLER FAILED"
else
    echo "INSTALLER SUCCESSFULLY" 
fi

exit $errorstatus

