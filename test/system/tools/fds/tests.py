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

    usage: test.py <install_path> <test_name>

    install_path: Fast-DDS base path installation

    test_name: Test to run.

    Available tests:

        test_fastdds_installed
        test_fastdds_discovery
        test_fastdds_shm

"""

import argparse
import subprocess
import sys
import time
import signal
import os
from pathlib import Path


def test_fast_discovery_closure(fast_discovery_tool):
    """Test that discovery command closes correctly."""
    args = [fast_discovery_tool] + '-i 0'.split(" ")

    # this subprocess cannot be executed in shell=True or using bash
    #  because a background script will not broadcast the signals
    #  it receives
    proc = subprocess.Popen(args, stdout=subprocess.PIPE)

    # sleep to let the server run
    time.sleep(1.5)

    # send SIGINT to process
    #os.kill(proc.pid, signal.SIGINT)
    proc.send_signal(signal.SIGINT)

    EXPECTED_CLOSURE = "### Server shutted down ###"

    # sleep to let the server shut down
    proc.wait()

    # joins all the output and converts to string
    output = b'\n'.join(proc.stdout.readlines()).decode()

    # check if correct shut down message has been sent
    if not EXPECTED_CLOSURE in output:
        print('test_fast_discovery_closure FAILED')
        sys.exit(1)


def get_paths(install_path):
    """
    Adjust the install path when --merge-install has been used.

    param install_path Path:
        Path to the Fast-DDS installation path

    return Path:
        Adjusted path to the installation path where fastdds tool
        is installed

    """
    tool_install_path = install_path / 'bin'

    if not os.path.exists(tool_install_path.resolve()):
        tool_install_path = tool_install_path / '..' / 'bin'
        if not os.path.exists(tool_install_path.resolve()):
            print(f'{tool_install_path} NOT FOUND')
            sys.exit(1)

    return tool_install_path


if __name__ == '__main__':

    parser = argparse.ArgumentParser(
            usage='test.py <install_path> <test_name>',
        )

    parser.add_argument('install_path',
                        help='FastDDS executables install path')

    parser.add_argument('test_name',
                        help='Test to run')

    args = parser.parse_args()
    
    tool_path = get_paths(Path(args.install_path))

    fast_discovery_tool = tool_path / 'fast-discovery-server'

    # Tests dictionary
    tests = {
        'test_fast_discovery_closure': lambda: test_fast_discovery_closure(
            fast_discovery_tool),
    }

    tests[args.test_name]()
