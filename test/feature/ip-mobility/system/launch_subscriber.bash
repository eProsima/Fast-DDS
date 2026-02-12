# Copyright (C) 2025, Proyectos y Sistemas de Mantenimiento SL (eProsima)
#
# This program is commercial software licensed under the terms of the
# eProsima Software License Agreement Rev 03 (the "License")
#
# You may obtain a copy of the License at
# https://www.eprosima.com/licenses/LICENSE-REV03

#!/bin/bash

# Note: This script is intended to be used in a privileged container, since it requires to bring down and up the eth0 interface.

echo "Putting down eth0 interface..."
ifconfig eth0 down

echo "Launching subscriber..."
${EXAMPLE_DIR}/DDSCommunicationSubscriber --xmlfile ${EXAMPLE_DIR}/simple_reliable_profile.xml --samples 10 --seed 0 --magic T --rescan 2 &
subs_pid=$!
echo "Subscriber launched."

echo "Waiting 2 seconds and bring up eth0 interface..."
sleep 2s
ifconfig eth0 up
echo "eth0 interface is up."

echo "Waiting 3s for the subscriber (process id $subs_pid) to finish..."
wait $subs_pid
