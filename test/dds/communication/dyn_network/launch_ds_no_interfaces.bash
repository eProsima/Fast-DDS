# Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

set -e

PREFIX="@CMAKE_INSTALL_PREFIX@"
EXAMPLE_DIR="@PROJECT_BINARY_DIR@/test/dds/communication"
export EXAMPLE_DIR

echo "[INFO] Starting discovery server"
"$PREFIX/bin/fastdds" discovery -i 0 -l 0.0.0.0 &
SERVER_PID=$!


# Publisher
echo "[INFO] Starting publisher"
"$EXAMPLE_DIR/DDSCommunicationPublisher" \
  --xmlfile "$EXAMPLE_DIR/ds_client.xml" \
  --wait 1 --samples 10 --loops 1 --seed 0 --magic T &
PUB_PID=$!

# Subscriber
echo "[INFO] Starting subscriber"
"$EXAMPLE_DIR/DDSCommunicationSubscriber" \
  --xmlfile "$EXAMPLE_DIR/ds_client.xml" \
  --samples 10 --seed 0 --magic T --rescan 2 &
SUB_PID=$!

sleep 15
# Discovery server process must still be running
if ! kill "$SERVER_PID" 2>/dev/null; then
    echo "[ERROR] Discover server ended unexpectedly"
    exit 1
else
    echo "[INFO] Manually killing discovery server. Server execution successful"
fi

# PUBLISHER and SUBSCRIBER processes must end gracefully
wait $PUB_PID
wait $SUB_PID
echo "[INFO] Test completed successfully"
