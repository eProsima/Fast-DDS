# Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
