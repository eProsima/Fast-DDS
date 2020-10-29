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

    binary: Fast-DDS binary path

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

def test_fast_discovery_closure(fast_discovery_tool):
    """Test that discovery command closes correctly."""
    command = [fast_discovery_tool, '-i', '0']

#   print("Executing command: " + str(command))

    # this subprocess cannot be executed in shell=True or using bash
    #  because a background script will not broadcast the signals
    #  it receives
    proc = subprocess.Popen(command,
                            stdout=subprocess.PIPE
                           )

    # sleep to let the server run
    time.sleep(1)

    # send SIGINT to process
    try:

        while proc.poll() is None:
#           print('iterating...')
            if os.name == 'posix':
                proc.send_signal(signal.SIGINT)
            elif os.name == 'nt':
                proc.send_signal(signal.CTRL_C_EVENT)
            # sleep to let the server shut down
            time.sleep(1)

    except KeyboardInterrupt:
       pass
#      print('Catched a KeyboardInterrupt')

    # CTest behaves differently than calling python directly

    EXPECTED_CLOSURE = "### Server shutted down ###"

    # joins all the output and converts to string
    output = proc.stdout.readlines()
    output = b'\n'.join(output).decode()

    # check if correct shut down message has been sent
    if EXPECTED_CLOSURE in output:
        # success
        print('test_fast_discovery_closure SUCCEED')
        proc = None
        return

    print('test_fast_discovery_closure FAILED')
    sys.exit(1)

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
