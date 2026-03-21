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

import shlex, subprocess, time, os, socket, sys, threading, glob, csv

if os.environ.get("PROFILING_BINS"):
    binaries = os.environ.get("PROFILING_BINS").split(';')

valgrind = os.environ.get("VALGRIND_BIN")
certs_path = os.environ.get("CERTS_PATH")
test_time = "10"

if not valgrind:
    valgrind = "valgrind"

def cycles_result(filename, thread_logs):
    thread_numbers = []
    thread_instruction_counts = []
    for thread_log in thread_logs:
        with open (thread_log, 'r') as f:
            lines = f.read().splitlines()
            thread_numbers.append(lines[2].split(' ')[-1])
            thread_instruction_counts.append(int(lines[4].split(' ')[-1]))
        os.remove(thread_log)

    total_instructions = sum(thread_instruction_counts)
    with open('./output/results_' + filename + '.csv','w') as results:
        writer = csv.writer(results)
        writer.writerow(['Context','Instructions'])
        writer.writerow(['Total', total_instructions])
        for number, count in zip(thread_numbers, thread_instruction_counts):
            writer.writerow(['Thread ' + number, str(count)])

def start_test(command, pubsub, time):
    os.system("mkdir -p output")

    valgrind_command_rel = [valgrind, "--tool=exp-bbv","--instr-count-only=yes", "--bb-out-file=./output/bbLog_" + pubsub + "_rel.out"]
    valgrind_command_be = [valgrind, "--tool=exp-bbv","--instr-count-only=yes", "--bb-out-file=./output/bbLog_" + pubsub + "_be.out"]

    options = ["--time=" + time]

    if certs_path:
        options.extend(["--security=true", "--certs=" + certs_path])

    # Best effort
    proc = subprocess.Popen(valgrind_command_be +
            [command, pubsub] +
            options)

    proc.communicate()

    # Reliable
    proc = subprocess.Popen(valgrind_command_rel +
            [command, pubsub, "-r", "reliable"] +
            options)

    proc.communicate()

    thread_logs_pub_rel = glob.glob('./output/bbLog_publisher_rel.out')
    thread_logs_sub_rel = glob.glob('./output/bbLog_subscriber_rel.out')
    thread_logs_pub_be = glob.glob('./output/bbLog_publisher_be.out')
    thread_logs_sub_be = glob.glob('./output/bbLog_subscriber_be.out')

    cycles_result("pub_rel", thread_logs_pub_rel)
    cycles_result("sub_rel", thread_logs_sub_rel)
    cycles_result("pub_be", thread_logs_pub_be)
    cycles_result("sub_be", thread_logs_sub_be)

if len(sys.argv) >= 4:
    test_time = sys.argv[3]

if len(sys.argv) == 3:
    binaries = [sys.argv[2]]

for command in binaries:
    if len(sys.argv) == 2:
        pubsub = sys.argv[1]
        start_test(command, pubsub, test_time)
    else:
        tpub = threading.Thread(target=start_test, args=(command, "publisher", test_time))
        tpub.start()
        tsub = threading.Thread(target=start_test, args=(command, "subscriber", test_time))
        tsub.start()

quit()
