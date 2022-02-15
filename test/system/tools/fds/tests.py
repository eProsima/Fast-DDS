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
        test_fast_discovery_parse_XML_file_prefix_OK
        test_fast_discovery_parse_XML_file_prefix_OK_URI
        test_fast_discovery_parse_XML_file_server_address
        test_fast_discovery_parse_XML_file_server_address_URI
"""

import argparse
import subprocess
import sys
import time
import signal
import os

from xml.dom import minidom


def signal_handler(signum, frame):
    # ignore signals if the test generates them
    pass


def send_command(command):
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
    return output, err, exit_code


def XML_parse_profile(XMLfile, profile_name):

    XML_file = minidom.parse(XMLfile)

    participants = XML_file.getElementsByTagName("participant")
    for participant in participants:
        if not profile_name:
            if participant.getAttribute("is_default_profile") == "true":
                participant_profile = participant
        else:
            if participant.getAttribute("profile_name") == profile_name:
                participant_profile = participant

    return participant_profile


def check_output(output, err, output_to_check):

    EXPECTED_CLOSURE = "### Server shut down ###"
    if EXPECTED_CLOSURE in output:
        if output_to_check in output:
            # Success
            exit_code = 0
            print('test_fast_discovery_closure SUCCEED')
        else:
            # Failure
            print('test_fast_discovery_closure FAILED')
            print('STDOUT:')
            print(output)
            print('STDERR:')
            print(err)
            exit_code = 3
    else:
        # Failure
        print('test_fast_discovery_closure FAILED')
        print('STDOUT:')
        print(output)
        print('STDERR:')
        print(err)
        exit_code = 3

    return exit_code


def test_fast_discovery_closure(fast_discovery_tool):
    """Test that discovery command closes correctly."""
    command = [fast_discovery_tool, '-i', '0']

    output, err, exit_code = send_command(command)

    EXPECTED_CLOSURE = "### Server shut down ###"

    check_output(output, err, EXPECTED_CLOSURE)

    sys.exit(exit_code)


def test_fast_discovery_parse_XML_file_prefix_OK(fast_discovery_tool):
    """Test that discovery command read XML prefix."""

    XML_file_path = 'test_xml_discovery_server.xml'

    profile = XML_parse_profile(XML_file_path, "")

    prefix = profile.getElementsByTagName('prefix')

    PREFIX = prefix[0].firstChild.data

    command = [fast_discovery_tool, '-x', XML_file_path]

    output, err, exit_code = send_command(command)

    EXPECTED_SERVER_ID = "Server GUID prefix: " + PREFIX.lower()
    print(EXPECTED_SERVER_ID)

    exit_code = check_output(output, err, EXPECTED_SERVER_ID)
    sys.exit(exit_code)


def test_fast_discovery_parse_XML_file_prefix_OK_URI(fast_discovery_tool):
    """Test that discovery command read XML prefix usuing URI."""

    XML_file_path = 'test_xml_discovery_server.xml'

    profile = XML_parse_profile(XML_file_path, "UDP_server_two")

    prefix = profile.getElementsByTagName('prefix')

    PREFIX = prefix[0].firstChild.data

    command = [fast_discovery_tool, '-x', XML_file_path + '@UDP_server_two']

    output, err, exit_code = send_command(command)

    EXPECTED_SERVER_ID = "Server GUID prefix: " + PREFIX.lower()
    print(EXPECTED_SERVER_ID)

    exit_code = check_output(output, err, EXPECTED_SERVER_ID)
    sys.exit(exit_code)


def test_fast_discovery_parse_XML_file_server_address(fast_discovery_tool):
    """Test that discovery command read XML server_address."""

    EXPECTED_SERVER_ADDRESS = []

    XML_file_path = 'test_xml_discovery_server.xml'

    default_profile = XML_parse_profile(XML_file_path, "")

    udpv4 = default_profile.getElementsByTagName('udpv4')
    for elem in udpv4:
        address2 = elem.getElementsByTagName('address')[0].firstChild.data
        port2 = elem.getElementsByTagName('port')[0].firstChild.data
        if port2[0] == '0':
            port2 = port2[1:]
        EXPECTED_SERVER_ADDRESS.append("UDPv4:[" + address2 + "]:" + port2)

    command = [fast_discovery_tool, '-x', XML_file_path]

    output, err, exit_code = send_command(command)

    for add in EXPECTED_SERVER_ADDRESS:
        exit_code = check_output(output, err, add)
        if exit_code != 0:
            sys.exit(exit_code)

    sys.exit(exit_code)


def test_fast_discovery_parse_XML_file_server_address_URI(fast_discovery_tool):
    """Test that discovery command read XML server_address usuing URI."""

    EXPECTED_SERVER_ADDRESS = []

    XML_file_path = 'test_xml_discovery_server.xml'

    default_profile = XML_parse_profile(XML_file_path, "UDP_server_two")

    udpv4 = default_profile.getElementsByTagName('udpv4')
    for elem in udpv4:
        address2 = elem.getElementsByTagName('address')[0].firstChild.data
        port2 = elem.getElementsByTagName('port')[0].firstChild.data
        if port2[0] == '0':
            port2 = port2[1:]
        EXPECTED_SERVER_ADDRESS.append("UDPv4:[" + address2 + "]:" + port2)

    command = [fast_discovery_tool, '-x', XML_file_path + '@UDP_server_two']

    output, err, exit_code = send_command(command)

    for add in EXPECTED_SERVER_ADDRESS:
        exit_code = check_output(output, err, add)
        if exit_code != 0:
            sys.exit(exit_code)

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
        'test_fast_discovery_parse_XML_file_prefix_OK': lambda:
            test_fast_discovery_parse_XML_file_prefix_OK(args.binary_path),
        'test_fast_discovery_parse_XML_file_server_address': lambda:
            test_fast_discovery_parse_XML_file_server_address(
                args.binary_path),
        'test_fast_discovery_parse_XML_file_prefix_OK_URI': lambda:
            test_fast_discovery_parse_XML_file_prefix_OK_URI(
                args.binary_path),
        'test_fast_discovery_parse_XML_file_server_address_URI': lambda:
            test_fast_discovery_parse_XML_file_server_address_URI(
                args.binary_path),

    }

    tests[args.test_name]()
