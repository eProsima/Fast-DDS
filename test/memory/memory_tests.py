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

import shlex, subprocess, time, os, socket, sys

command = os.environ.get("MEMORY_TEST_BIN")
valgrind = os.environ.get("VALGRIND_BIN")
certs_path = os.environ.get("CERTS_PATH")

if not valgrind:
    valgrind = "valgrind"

if len(sys.argv) == 3:
    command = sys.argv[2]

pubsub = sys.argv[1]

valgrind_command_rel = [valgrind, "--tool=massif", "--stacks=yes", "--detailed-freq=1", "--max-snapshots=1000", "--massif-out-file=./output/consumption_" + pubsub + "_rel.out"]
valgrind_command_be = [valgrind, "--tool=massif", "--stacks=yes", "--detailed-freq=1", "--max-snapshots=1000", "--massif-out-file=./output/consumption_" + pubsub + "_be.out"]

security_options = []

if certs_path:
    security_options = ["--security=true", "--certs=" + certs_path]

# Best effort
proc = subprocess.Popen(valgrind_command_be +
        [command, pubsub] +
        security_options)

proc.communicate()

py_command = "python3 ./memory_analysis.py ./output/consumption_" + pubsub + "_be.out ./output/MemoryTest_" + pubsub + "_be.csv"
p = subprocess.Popen(py_command, shell=True)

# Reliable
proc = subprocess.Popen(valgrind_command_rel +
        [command, pubsub, "-r", "reliable"] +
        security_options)

proc.communicate()

py_command = "python3 ./memory_analysis.py ./output/consumption_" + pubsub + "_rel.out ./output/MemoryTest_" + pubsub + "_rel.csv"
# print("Command: " + py_command)
p = subprocess.Popen(py_command, shell=True)

quit()
