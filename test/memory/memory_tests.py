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

valgrind_command_pub_rel = [valgrind, "--tool=massif", "--stacks=yes", "--detailed-freq=1", "--max-snapshots=1000", "--massif-out-file=./consumption_pub_rel.out"]
valgrind_command_sub_rel = [valgrind, "--tool=massif", "--stacks=yes", "--detailed-freq=1", "--max-snapshots=1000", "--massif-out-file=./consumption_sub_rel.out"]
valgrind_command_pub_be = [valgrind, "--tool=massif", "--stacks=yes", "--detailed-freq=1", "--max-snapshots=1000", "--massif-out-file=./consumption_pub_be.out"]
valgrind_command_sub_be = [valgrind, "--tool=massif", "--stacks=yes", "--detailed-freq=1", "--max-snapshots=1000", "--massif-out-file=./consumption_sub_be.out"]

security_options = []

if certs_path:
    security_options = ["--security=true", "--certs=" + certs_path]

# Best effort
subscriber_proc = subprocess.Popen(valgrind_command_sub_be +
        [command, "subscriber", "--seed", str(os.getpid()), "--hostname"] +
        security_options)
publisher_proc = subprocess.Popen(valgrind_command_pub_be +
        [command, "publisher", "--seed", str(os.getpid()), "--hostname"] +
        security_options)

subscriber_proc.communicate()
publisher_proc.communicate()

py_command = "python3 ./memory_analysis.py ./consumption_pub_be.out ./stack_pub_be.csv"
p = subprocess.Popen(py_command, shell=True)
py_command = "python3 ./memory_analysis.py ./consumption_sub_be.out ./stack_sub_be.csv"
p = subprocess.Popen(py_command, shell=True)

# Reliable
subscriber_proc = subprocess.Popen(valgrind_command_sub_rel +
        [command, "subscriber", "-r", "reliable", "--seed", str(os.getpid()), "--hostname"] +
        security_options)
publisher_proc = subprocess.Popen(valgrind_command_pub_rel +
        [command, "publisher", "-r", "reliable", "--seed", str(os.getpid()), "--hostname"] +
        security_options)

subscriber_proc.communicate()
publisher_proc.communicate()

py_command = "python3 ./memory_analysis.py ./consumption_pub_rel.out ./stack_pub_rel.csv"
# print("Command: " + py_command)
p = subprocess.Popen(py_command, shell=True)
py_command = "python3 ./memory_analysis.py ./consumption_sub_rel.out ./stack_sub_rel.csv"
p = subprocess.Popen(py_command, shell=True)

quit()
