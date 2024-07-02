# Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima)
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

import glob
import os
import queue
import subprocess
import sys
import threading
import time

script_dir = os.path.dirname(os.path.realpath(__file__))

# Test executable
process_command = os.environ.get(
    'CLIENT_SERVER_DYNAMIC_DISCOVERY_BIN')
if not process_command:
    process_files = glob.glob(
        os.path.join(
           script_dir,
           '**/DDSParticipantDiscovery*'),
        recursive=True)
    pf = iter(process_files)
    process_command = next(pf, None)
    while process_command and \
        (not os.path.isfile(process_command)
         or not os.access(process_command,
         os.X_OK)):
        process_command = next(pf, None)
assert(process_command)

# Thread that read process output and push it into a queue
def output_reader(proc, outq):
    for line in iter(proc.stdout.readline, b''):
        outq.put(line.decode('utf-8'))

def first_step(outq):
    first_step_fulfilled = False
    server_1_discover_client = False
    count = 0
    initial_time = time.time()
    while not first_step_fulfilled:
        global stop_threads
        if stop_threads:
            break
        try:
            line = outq.get(block=False).rstrip()
            print(line)
            sys.stdout.flush()

            assert '44.53.00.5f.45.50.52.4f.53.49.4d.41' in line
            assert 'discovered participant' in line
            count = count + 1
            if 'discovered participant 44.53.00.5f.45.50.52.4f.53.49.4d.41|0.0.1.c1: 1' in line:
                print('CLIENT OVERRIDE discovered SERVER 1')
                server_1_discover_client = True
        except queue.Empty:
            # Ensure that 2 s has passed so the file watch can detect that the file has changed
            if server_1_discover_client and count >= 2 and (time.time() - initial_time) > 2:
                if count == 2:
                    first_step_fulfilled = True
                else:
                    print('ERROR: More discoveries than expected')
                    stop_threads = True
                    sys.exit(1)
            sys.stdout.flush()
        except AssertionError:
            print('ASSERTION ERROR: ' + line)
            stop_threads = True
            sys.exit(1)
        time.sleep(0.1)

def second_step(outq):
    second_step_fulfilled = False
    server_2_discover_client = False
    client_2_warning = False
    count = 0
    initial_time = time.time()
    while not second_step_fulfilled:
        global stop_threads
        if stop_threads:
            break
        try:
            line = outq.get(block=False).rstrip()
            print(line)
            sys.stdout.flush()

            if 'discovered participant' in line:
                assert '44.53.01.5f.45.50.52.4f.53.49.4d.41' in line
                count = count + 1
                if 'discovered participant 44.53.01.5f.45.50.52.4f.53.49.4d.41|0.0.1.c1: 2' in line:
                    print('CLIENT OVERRIDE discovered SERVER 2')
                    server_2_discover_client = True
            elif 'Warning' in line:
                # Client 2 does not discover anyone
                assert 'Trying to add Discovery Servers to a participant which is not a SERVER, BACKUP or an' in line
                assert 'overriden CLIENT (SIMPLE participant transformed into CLIENT with the environment variable)' in line
                client_2_warning = True
            else:
                assert 'detected changes on participant' in line
        except queue.Empty:
            # Ensure that 2 s has passed so the file watch can detect that the file has changed
            if server_2_discover_client and client_2_warning and count >= 2 and (time.time() - initial_time) > 2:
                if count == 2:
                    second_step_fulfilled = True
                else:
                    print('ERROR: More discoveries than expected')
                    stop_threads = True
                    sys.exit(1)
            sys.stdout.flush()
        except AssertionError:
            print('ASSERTION ERROR: ' + line)
            stop_threads = True
            sys.exit(1)
        time.sleep(0.1)

def third_step(outq):
    third_step_fulfilled = False
    server_1_discover_server_2 = False
    server_2_discover_server_1 = False
    initial_time = time.time()
    while not third_step_fulfilled:
        global stop_threads
        if stop_threads:
            break
        try:
            line = outq.get(block=False).rstrip()
            print(line)
            sys.stdout.flush()

            if 'Participant 44.53.01.5f.45.50.52.4f.53.49.4d.41|0.0.1.c1 discovered participant' in line and \
                '44.53.00.5f.45.50.52.4f.53.49.4d.41|0.0.1.c1: 2' in line:
                print ('SERVER 2 discovers SERVER 1')
                server_2_discover_server_1 = True
            elif 'Participant 44.53.00.5f.45.50.52.4f.53.49.4d.41|0.0.1.c1 discovered participant' in line and \
                '44.53.01.5f.45.50.52.4f.53.49.4d.41|0.0.1.c1: 2' in line:
                print ('SERVER 1 discovers SERVER 2')
                server_1_discover_server_2 = True
            else:
                assert 'detected changes on participant' in line
        except queue.Empty:
            # Ensure that 2 s has passed so the file watch can detect that the file has changed
            if server_1_discover_server_2 and server_2_discover_server_1 and (time.time() - initial_time) > 2:
                third_step_fulfilled = True
            sys.stdout.flush()
        except AssertionError:
            print('ASSERTION ERROR: ' + line)
            stop_threads = True
            sys.exit(1)
        time.sleep(0.1)

def fourth_step(outq):
    fourth_step_fulfilled = False
    warning_client_2 = False
    count = 0
    initial_time = time.time()
    while not fourth_step_fulfilled:
        global stop_threads
        if stop_threads:
            break
        try:
            line = outq.get(block=False).rstrip()
            print(line)
            sys.stdout.flush()

            if 'Trying to add Discovery Servers to a participant which is not a SERVER, BACKUP or an' in line \
                and 'overriden CLIENT (SIMPLE participant transformed into CLIENT with the environment variable)' in line:
                warning_client_2 = True
            elif 'discovered participant' in line:
                count = count + 1
        except queue.Empty:
            # Ensure that 2 s has passed so the file watch can detect that the file has changed
            if warning_client_2 and count == 0 and (time.time() - initial_time) > 2:
                fourth_step_fulfilled = True
            elif count > 0:
                print('ERROR: More discoveries than expected')
                stop_threads = True
                sys.exit(1)
            sys.stdout.flush()
        except AssertionError:
            print('ASSERTION ERROR: ' + line)
            stop_threads = True
            sys.exit(1)
        time.sleep(0.1)

def fifth_step(outq):
    fifth_step_fulfilled = False
    count = 0
    initial_time = time.time()
    while not fifth_step_fulfilled:
        global stop_threads
        if stop_threads:
            break
        try:
            line = outq.get(block=False).rstrip()
            print(line)
            sys.stdout.flush()

            if 'discovered participant' in line:
                count = count + 1
        except queue.Empty:
            # Ensure that 2 s has passed so the file watch can detect that the file has changed
            if count == 0 and (time.time() - initial_time) > 2:
                fifth_step_fulfilled = True
            elif count > 0:
                print('ERROR: More discoveries than expected')
                stop_threads = True
                sys.exit(1)
            sys.stdout.flush()
        except AssertionError:
            print('ASSERTION ERROR: ' + line)
            stop_threads = True
            sys.exit(1)
        time.sleep(0.1)

def exit(cv):
    cv.release()
    print('ERROR: timeout without expected discovery happening')
    global stop_threads
    stop_threads = True
    os.remove(server_1_env_file)
    os.remove(server_2_env_file)
    os.remove(client_env_file)
    sys.exit(1)

def communication(proc, outq, outt, cv):
    """A"""
    t = threading.Thread(target=output_reader, args=(proc,outq))
    t.start()

    try:
        time.sleep(0.2)

        while True:
            global stop_threads
            if stop_threads:
                break
            try:
                line = outt.get(block=False).rstrip()
                print(line)
                sys.stdout.flush()

                if "FIRST STEP" in line:
                    first_step(outq)
                    cv.acquire()
                    cv.notify()
                    cv.release()
                elif "SECOND STEP" in line:
                    second_step(outq)
                    cv.acquire()
                    cv.notify()
                    cv.release()
                elif "THIRD STEP" in line:
                    third_step(outq)
                    cv.acquire()
                    cv.notify()
                    cv.release()
                elif "FOURTH STEP" in line:
                    fourth_step(outq)
                    cv.acquire()
                    cv.notify()
                    cv.release()
                elif "FIFTH STEP" in line:
                    fifth_step(outq)
                    cv.acquire()
                    cv.notify()
                    cv.release()


            except queue.Empty:
                sys.stdout.flush()

            time.sleep(0.1)
    finally:
        proc.terminate()

    while outq.empty() != True:
        line = outq.get(block=False).rstrip()
        print(line)

    t.join()

# Random unicast port
random_port_server_1 = os.environ.get(
    'W_UNICAST_PORT_RANDOM_NUMBER')
random_port_server_2 = str(int(random_port_server_1) + 1)

# Condition variable
cv = threading.Condition()

# Environment files
server_1_env_file = "server_1_env_file.json"
server_2_env_file = "server_2_env_file.json"
client_env_file = "client_env_file.json"
# Both server environment files are created empty
open(server_1_env_file, 'w+').close()
open(server_2_env_file, 'w+').close()
# Client environment file should include the locator for the first server
f = open(client_env_file, 'w+')
f.write('{"ROS_DISCOVERY_SERVER": "localhost:')
f.write(random_port_server_1)
f.write('"}')
f.close()

outq = queue.Queue()
outt = queue.Queue()
outt.put("TEST RUNNING\n")
outt.put("FIRST STEP: Override Client discovers Server 1\n")

server_1_process = subprocess.Popen([process_command,
    "--discovery_protocol", "SERVER",
    "--guid_prefix", "44.53.00.5F.45.50.52.4F.53.49.4D.41",
    "--unicast_metatraffic_locator", random_port_server_1],
    env={"FASTDDS_ENVIRONMENT_FILE": server_1_env_file},
    stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
server_2_process = subprocess.Popen([process_command,
    "--discovery_protocol", "SERVER",
    "--guid_prefix", "44.53.01.5F.45.50.52.4F.53.49.4D.41",
    "--unicast_metatraffic_locator", random_port_server_2],
    env={"FASTDDS_ENVIRONMENT_FILE": server_2_env_file},
    stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
# The client must be DiscoveryProtocol::SIMPLE to use the environment variable
client_override_process = subprocess.Popen(process_command,
    env={"FASTDDS_ENVIRONMENT_FILE": client_env_file},
    stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
# DiscoveryProtocol::CLIENT, environment variable does not apply either initializing as updating
client_process = subprocess.Popen([process_command,
    "--discovery_protocol", "CLIENT"],
    env={"FASTDDS_ENVIRONMENT_FILE": client_env_file},
    stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

stop_threads = False
t_0 = threading.Thread(target=communication, args=(server_1_process,outq,outt,cv))
t_1 = threading.Thread(target=communication, args=(server_2_process,outq,outt,cv))
t_2 = threading.Thread(target=communication, args=(client_override_process,outq,outt,cv))
t_3 = threading.Thread(target=communication, args=(client_process,outq,outt,cv))

t_0.start()
t_1.start()
t_2.start()
t_3.start()

# Wait 10 seconds for the condition variable to be notified
cv.acquire()
result = cv.wait(10)
if result == False:
    exit(cv)

outt.put("SECOND STEP: Override Client discovers both Servers. Normal Client does not discover anyone\n")

# Add second server to client
f = open(client_env_file, 'w+')
f.write('{"ROS_DISCOVERY_SERVER": "localhost:')
f.write(random_port_server_1)
f.write(';localhost:')
f.write(random_port_server_2)
f.write('"}')
f.close()

result = cv.wait(10)
if result == False:
    exit(cv)

outt.put("THIRD STEP: Both Servers discover each other\n")

# Add second server to first server
f = open(server_1_env_file, 'w+')
f.write('{"ROS_DISCOVERY_SERVER": ";localhost:')
f.write(random_port_server_2)
f.write('"}')
f.close()

result = cv.wait(10)
if result == False:
    exit(cv)

outt.put("FOURTH STEP: Removing a Server from the Client list outputs a Log Warning\n")

# Remove first server from client list
f = open(client_env_file, 'w+')
f.write('{"ROS_DISCOVERY_SERVER": ";localhost:')
f.write(random_port_server_2)
f.write('"}')
f.close()

result = cv.wait(10)
if result == False:
    exit(cv)

outt.put("FIFTH STEP: Removing a Server from the  Server list outputs a Log Warning\n")

# Remove server from the server 1 list
f = open(server_1_env_file, 'w+')
f.write('{"ROS_DISCOVERY_SERVER": ""}')
f.close()

result = cv.wait(10)
if result == False:
    exit(cv)

outt.put("Killing processes\n")

cv.release()
# Kill processes
stop_threads = True

t_0.join()
t_1.join()
t_2.join()
t_3.join()

# Delete files
os.remove(server_1_env_file)
os.remove(server_2_env_file)
os.remove(client_env_file)
