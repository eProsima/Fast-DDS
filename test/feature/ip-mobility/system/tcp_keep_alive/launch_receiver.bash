# Copyright (C) 2025, Proyectos y Sistemas de Mantenimiento SL (eProsima)
#
# This program is commercial software licensed under the terms of the
# eProsima Software License Agreement Rev 03 (the "License")
#
# You may obtain a copy of the License at
# https://www.eprosima.com/licenses/LICENSE-REV03

#!/bin/bash

# Note: This script is intended to be used in a privileged container, since it requires to bring down and up the eth0 interface.

echo "Editing default route..."
ip route del default || true;
ip route add default via 192.168.10.1;

echo "Launching subscriber..."
${EXAMPLE_DIR}/configuration subscriber --profile-participant initial_peer_server --samples 20 -r &
subs_pid=$!
echo "Subscriber launched."

echo "Waiting for the subscriber (process id $subs_pid) to finish..."
wait $subs_pid
