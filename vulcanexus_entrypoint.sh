#!/bin/bash

# Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

print_usage()
{
    echo "-------------------------------------------------------------------------------------"
    echo "Script for the automatic configuration of a Vulcanexus container and direct usage of"
    echo "the tools included in it."
    echo "-------------------------------------------------------------------------------------"
    echo "OPTIONAL ARGUMENTS:"
    echo "   -s --shm [on|off|off_localhost_only]     Enable/disable Shared Memory Transport"
    echo "                                        on    -Enable SHM (Default)"
    echo "                                       off    -Disable SHM; use UDP Transport instead"
    echo "                        off_localhost_only    -Disable SHM; use UDP Transport instead"
    echo "                                               ; only localhost communication"
    echo ""
    echo "   -r --run [\"command\"]                     The tool/command to be run. If the command"
    echo "                                            is composed of several terms, it must be"
    echo "                                            enclosed by quotation marks"
    echo ""
    echo "   -h --help                                Print help"
    echo ""
    echo "EXAMPLE: bash vulcanexus_entrypoint.sh --shm off --run monitor"
    echo ""
    exit ${1}
}

parse_options()
{
    TEMP=`getopt \
            -o s:r:h \
            --long shm:,run:,help \
            -n 'Error' \
            -- "$@"`

    eval set -- "${TEMP}"

    SHM="on"
    RUN_CMD=
    while true; do
    case "$1" in
        # Optional args
        -s | --shm ) SHM="$2"; shift 2;;
        -r | --run ) RUN_CMD="$2"; shift 2;;
        -h | --help ) print_usage 0; shift ;;
        # Wrong args
        -- ) shift; break ;;
        * ) break ;;
    esac
    done

    if ! [[ "${SHM}" =~ ^("on"|"off"|"off_localhost_only")$ ]]
    then
        echo "-------------------------------------------------------------------------------------"
        echo "-s --shm only accepts on|off|off_localhost_only values"
        print_usage 1
    fi
}

main ()
{
    set -e
    parse_options "$@"
    EXIT_CODE=0

    # Shared Memory
    if [[ ${SHM} == "off" ]] ; then
        export FASTRTPS_DEFAULT_PROFILES_FILE=/tmp/disable_fastdds_shm.xml
    elif [[ ${SHM} == "off_localhost_only" ]] ; then
        export FASTRTPS_DEFAULT_PROFILES_FILE=/tmp/disable_fastdds_shm_localhost_only.xml
    fi

    # Setup environment
    source "/opt/ros/$ROS_DISTRO/setup.bash"
    source "/opt/vulcanexus/$VULCANEXUS_DISTRO/setup.bash"
    source "/vulcanexus_ws/install/setup.bash"

    # Launch tool
    if ! [[ "${RUN_CMD}" =~ ^()$ ]]
    then
        case ${RUN_CMD} in
            monitor ) fastdds_monitor;;
            shapesdemo ) ShapesDemo;;
            * ) ${RUN_CMD};;
        esac
    fi

    exec bash
}

main "$@"
