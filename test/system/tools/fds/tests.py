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

        test_fast_discovery_closure

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
    command = [fast_discovery_tool] + '-i 0'.split(" ")

    #print ("Executing command: " + str(command))

    # this subprocess cannot be executed in shell=True or using bash
    #  because a background script will not broadcast the signals
    #  it receives
    proc = subprocess.Popen(command, stdout=subprocess.PIPE)

    # sleep to let the server run
    time.sleep(1.5)

    # send SIGINT to process
    #os.kill(proc.pid, signal.SIGINT)
    if os.name == 'posix':
        proc.send_signal(signal.SIGINT)
    elif os.name == 'nt':
        proc.send_signal(signal.CTRL_C_EVENT)
    

    EXPECTED_CLOSURE = "### Server shutted down ###"

    # sleep to let the server shut down
    proc.wait()

    # joins all the output and converts to string
    output = b'\n'.join(proc.stdout.readlines()).decode()

    # check if correct shut down message has been sent
    if not EXPECTED_CLOSURE in output:
        print('test_fast_discovery_closure FAILED')
        sys.exit(1)


def get_path(install_path):
    """
    Adjust the install path when --merge-install has been used.

    param install_path Path:
        Path to the Fast-DDS installation path

    return Path:
        Adjusted path to the installation path where fastdds tool
        is installed

    """

    # TODO change to build_path
    build_tool_path_from_install = "../build/fastrtps/tools/fds"

    tool_build_path = install_path / build_tool_path_from_install

    if not os.path.exists(tool_build_path.resolve()):
        tool_build_path = install_path / '..' / build_tool_path_from_install
        if not os.path.exists(tool_build_path.resolve()):
            print(f'{tool_build_path} NOT FOUND')
            sys.exit(1)

    return tool_build_path

def get_tool(build_path, tool_name):
    
    # get all the files with the tool_name and its dates of modification
    possible_tools =  [os.path.join(build_path,f) for f in os.listdir(build_path) if f.startswith(tool_name)]
    possible_tools = [(f, os.stat(f).st_mtime) for f in possible_tools]

    if possible_tools == []:
        print(f'{tool_name} NOT FOUND in {build_path}')
        sys.exit(1)

    return sorted(possible_tools, key=lambda tup: tup[1])[0][0]


def creation_date(path_to_file):

    if os.name == 'nt':
        return os.path.getctime(path_to_file)
    elif os.name == 'posix':
        stat = os.stat(path_to_file)
        try:
            return stat.st_birthtime
        except AttributeError:
            # We're probably on Linux. No easy way to get creation dates here,
            # so we'll settle for when its content was last modified.
            return stat.st_mtime


if __name__ == '__main__':

    parser = argparse.ArgumentParser(
            usage='test.py <install_path> <test_name>',
        )

    # TODO change install path for build_path - in CMakeLists.txt change the var env
    parser.add_argument('install_path',
                        help='FastDDS executables install path')

    parser.add_argument('test_name',
                        help='Test to run')

    args = parser.parse_args()
    
    tool_path = get_path(Path(args.install_path))

    fast_discovery_tool = get_tool(tool_path, 'fast-discovery-server')

    # Tests dictionary
    tests = {
        'test_fast_discovery_closure': lambda: test_fast_discovery_closure(
            fast_discovery_tool),
    }

    tests[args.test_name]()
