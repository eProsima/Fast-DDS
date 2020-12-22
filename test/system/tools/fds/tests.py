# Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

"""
    Tests for the fastdds tool.

    Contains a package of system test for fastdds tool

    usage: test.py <binary_path> <test_name>

    binary_path: Fast-DDS binary path

    test_name: Test to run.

    Available tests:

        test_fast_discovery_closure

"""

import argparse
import subprocess
import sys
import time
import signal
import os

def signal_handler(signum, frame):
    # ignore signals if the test generates them
    pass

def test_fast_discovery_closure(fast_discovery_tool):
    """Test that discovery command closes correctly."""
    command = [fast_discovery_tool, '-i', '0']

    print("Executing command: " + str(command))

    # this subprocess cannot be executed in shell=True or using bash
    #  because a background script will not broadcast the signals
    #  it receives
    proc = subprocess.Popen(command,
                            stdout=subprocess.PIPE,
                            universal_newlines=True
                           )

    # sleep to let the server run
    time.sleep(1)

    # 1. An exit code of 0 means everything was alright
    # 2. An exit code of 1 means the tool's process terminated before even
    #    sending the kill signal.
    # 3. An exit code of 2 means the signal could not terminate the process
    # 4. An exit code of 3 means the signal terminated the process, but the
    #    output was different than expected
    exit_code = 0

    # Check whether the process has terminated already
    if not proc.poll() is None:
        # If the process has already exit means something has gone wrong.
        # Capture and print output for traceability and exit with code s1.
        output, err = proc.communicate()
        print('test_fast_discovery_closure FAILED on launching tool')
        print('STDOUT:')
        print(output)
        print('STDERR:')
        print(err)
        sys.exit(1)

    # direct this script to ignore SIGINT
    signal.signal(signal.SIGINT, signal_handler)

    # send SIGINT to process and wait for processing
    lease = 0
    while True:

        if os.name == 'posix':
            proc.send_signal(signal.SIGINT)
        elif os.name == 'nt':
            proc.send_signal(signal.CTRL_C_EVENT)

        time.sleep(1)
        lease += 1

        # Break when signal kills the process or it hangs
        if proc.poll() is None and lease < 10:
            print('iterating...')
        else:
            break

    # Check whether SIGINT was able to terminate the process
    if proc.poll() is None:
        # SIGINT couldn't terminate the process. Kill it and exit with code 2
        proc.kill()
        print('Signal could not kill process')
        sys.exit(2)

    # Get process output
    output, err = proc.communicate()

    EXPECTED_CLOSURE = "### Server shut down ###"

    # Check whether everything closed down nicely
    if EXPECTED_CLOSURE in output:
        # Success
        print('test_fast_discovery_closure SUCCEED')
    else:
        # Failure
        print('test_fast_discovery_closure FAILED')
        print('STDOUT:')
        print(output)
        print('STDERR:')
        print(err)
        exit_code = 3

    sys.exit(exit_code)

if __name__ == '__main__':

    parser = argparse.ArgumentParser(
            usage='test.py <binary_path> <test_name>',
        )

    parser.add_argument('binary_path',
                        help='''fast-discovery-server binary fully
                        qualified path''')

    parser.add_argument('test_name',
                        help='Test to run')

    args = parser.parse_args()

    # Tests dictionary
    tests = {
        'test_fast_discovery_closure': lambda: test_fast_discovery_closure(
            args.binary_path),
    }

    tests[args.test_name]()
