# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

import shlex, subprocess, time, os, socket, sys, threading

if os.environ.get("PROFILING_BINS"):
    binaries = os.environ.get("PROFILING_BINS").split(';')

valgrind = os.environ.get("VALGRIND_BIN")
certs_path = os.environ.get("CERTS_PATH")
test_time = "10"

if not valgrind:
    valgrind = "valgrind"

def start_test(command, pubsub, time, transport):
    os.system("mkdir -p output")

    valgrind_command_rel = [valgrind, "--tool=massif", "--stacks=yes", "--detailed-freq=1", "--max-snapshots=1000", "--massif-out-file=./output/consumption_" + pubsub + "_" + transport + "_rel.out"]
    valgrind_command_be = [valgrind, "--tool=massif", "--stacks=yes", "--detailed-freq=1", "--max-snapshots=1000", "--massif-out-file=./output/consumption_" + pubsub + "_" + transport + "_be.out"]

    options = ["--time=" + time]

    if certs_path:
        options.extend(["--security=true", "--certs=" + certs_path])

    # Best effort
    print(valgrind_command_be +
            [command, pubsub] +
            options)
    proc = subprocess.Popen(valgrind_command_be +
            [command, pubsub] +
            options)

    proc.communicate()

    py_command = "python3 ./memory_analysis.py ./output/consumption_" + pubsub + "_" + transport + "_be.out ./output/MemoryTest_" + pubsub + "_" + transport + "_be.csv"
    p = subprocess.Popen(py_command, shell=True)

    # Reliable
    proc = subprocess.Popen(valgrind_command_rel +
            [command, pubsub, "-r", "reliable"] +
            options)

    proc.communicate()

    py_command = "python3 ./memory_analysis.py ./output/consumption_" + pubsub + "_" + transport + "_rel.out ./output/MemoryTest_" + pubsub + "_" + transport + "_rel.csv"
    # print("Command: " + py_command)
    p = subprocess.Popen(py_command, shell=True)

transport = ""

if len(sys.argv) >= 5:
    transport = sys.argv[4]

if len(sys.argv) >= 4:
    test_time = sys.argv[3]

if len(sys.argv) >= 3:
    binaries = [sys.argv[2]]

for command in binaries:
    if len(sys.argv) >= 2:
        pubsub = sys.argv[1]
        start_test(command, pubsub, test_time, transport)
    else:
        tpub = threading.Thread(target=start_test, args=(command, "publisher", test_time, transport))
        tpub.start()
        tsub = threading.Thread(target=start_test, args=(command, "subscriber", test_time, transport))
        tsub.start()

quit()
