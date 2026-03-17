# Copyright (C) 2026, Proyectos y Sistemas de Mantenimiento SL (eProsima)
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
ip route add default via 192.168.20.1;

echo "Launching publisher..."
${EXAMPLE_DIR}/configuration publisher --profile-participant initial_peer --wait 1 --samples 60 -i 500 -r &
pub_pid=$!
echo "Publisher launched."

echo "Waiting for the publisher (process id $pub_pid) to finish..."
wait $pub_pid
