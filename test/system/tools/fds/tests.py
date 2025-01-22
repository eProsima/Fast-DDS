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

    binary_path: Fast DDS binary path

    test_name: Test to run.

    Available tests:

        test_fast_discovery_closure,
        test_fast_discovery_udpv6_address,
        test_fast_discovery_parse_XML_file_default_profile,
        test_fast_discovery_parse_XML_file_URI_profile,
        test_fast_discovery_prefix_override,
        test_fast_discovery_locator_address_override,
        test_fast_discovery_locator_override_same_address,
        test_fast_discovery_locator_port_override,
        test_fast_discovery_locator_override_same_port,
        test_fast_discovery_backup,
        test_fast_discovery_no_XML,
        test_fast_discovery_incorrect_participant,
        test_fast_discovery_no_prefix,
        test_fast_discovery_several_server_ids,
        test_fast_discovery_invalid_locator,
        test_fast_discovery_non_existent_profile,

"""

import argparse
import subprocess
import sys
import time
import signal
import os
from enum import Enum

from xml.dom import minidom

class Command_test(Enum):
    SERVER = "server" # Does not need to be specified
    AUTO = "auto"
    START = "start"
    STOP = "stop"
    ADD = "add"
    SET = "set"
    LIST = "list"
    INFO = "info"
    UNKNOWN = "unknown"

# This map is used to convert the string command to an integer used in the cpp tool
command_to_int_test = {
    Command_test.AUTO: 0,
    Command_test.START: 1,
    Command_test.STOP: 2,
    Command_test.ADD: 3,
    Command_test.SET: 4,
    Command_test.LIST: 5,
    Command_test.INFO: 6,
    Command_test.SERVER: 42
}

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
                            stderr=subprocess.PIPE,
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


def check_output(output, err, output_to_check, override, expect_error):
    """
    Check if the output is as expected.
    @output: Output of the command.
    @err: Error output of the command. Includes Logs of Fast DDS.
    @output_to_check: Expected output.
    @override: .
    @expect_error: True if the output_to_check is an error message in @err.
    """

    EXPECTED_CLOSURE = "### Server shut down ###"
    if EXPECTED_CLOSURE in output or override:
        check_output = err if expect_error else output
        if output_to_check in check_output:
            # Success
            exit_code = 0
        else:
            # Failure
            print('STDOUT:')
            print(output)
            print('STDERR:')
            print(err)
            exit_code = 3
    else:
        # Failure
        print('STDOUT:')
        print(output)
        print('STDERR:')
        print(err)
        exit_code = 3

    return exit_code


def test_fast_discovery_closure(fast_discovery_tool):
    """Test that discovery command closes correctly."""
    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-i', '0']

    output, err, exit_code = send_command(command)

    EXPECTED_CLOSURE = "### Server shut down ###"

    exit_code = check_output(output, err, EXPECTED_CLOSURE, False, False)

    sys.exit(exit_code)

def test_fast_discovery_udpv6_address(fast_discovery_tool):
    """Test that discovery command manages IPv4 and IPv6 correctly."""

    command = [
        fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-i', '1', '-l',
        '154.56.134.194', '-p', '32123', '-l', '2a02:ec80:600:ed1a::3', '-p', '14520'
    ]

    output, err, exit_code = send_command(command)

    if exit_code != 0:
        print(output)
        sys.exit(exit_code)

    EXPECTED_OUTPUTS = [
        "UDPv4:[154.56.134.194]:32123",
        "UDPv6:[2a02:ec80:600:ed1a::3]:14520",
    ]

    for pattern in EXPECTED_OUTPUTS:
        exit_code = check_output(output, err, pattern, False, False)
        if exit_code != 0:
            break

    sys.exit(exit_code)

def test_fast_discovery_parse_XML_file_default_profile(fast_discovery_tool):
    """Test that discovery command read XML default profile correctly."""

    XML_file_path = 'test_xml_discovery_server_profile.xml'
    default_profile = XML_parse_profile(XML_file_path, "")

    prefix = default_profile.getElementsByTagName('prefix')
    PREFIX = prefix[0].firstChild.data

    EXPECTED_SERVER_ADDRESS = []
    udpv4 = default_profile.getElementsByTagName('udpv4')
    for elem in udpv4:
        address2 = elem.getElementsByTagName('address')[0].firstChild.data
        port2 = elem.getElementsByTagName('port')[0].firstChild.data
        if port2[0] == '0':
            port2 = port2[1:]
        EXPECTED_SERVER_ADDRESS.append("UDPv4:[" + address2 + "]:" + port2)

    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-x', XML_file_path]

    output, err, exit_code = send_command(command)

    EXPECTED_SERVER_ID = "GUID prefix: " + PREFIX.lower()
    print(EXPECTED_SERVER_ID)

    exit_code = check_output(output, err, EXPECTED_SERVER_ID, False, False)
    if exit_code != 0:
        sys.exit(exit_code)

    for add in EXPECTED_SERVER_ADDRESS:
        exit_code = check_output(output, err, add, False, False)
        if exit_code != 0:
            sys.exit(exit_code)

    sys.exit(exit_code)


def test_fast_discovery_parse_XML_file_URI_profile(fast_discovery_tool):
    """Test that discovery command read XML profile using URI."""

    XML_file_path = 'test_xml_discovery_server_profile.xml'
    uri_profile = XML_parse_profile(XML_file_path, "UDP_server_two")

    prefix = uri_profile.getElementsByTagName('prefix')
    PREFIX = prefix[0].firstChild.data

    EXPECTED_SERVER_ADDRESS = []
    udpv4 = uri_profile.getElementsByTagName('udpv4')
    for elem in udpv4:
        address2 = elem.getElementsByTagName('address')[0].firstChild.data
        port2 = elem.getElementsByTagName('port')[0].firstChild.data
        if port2[0] == '0':
            port2 = port2[1:]
        EXPECTED_SERVER_ADDRESS.append("UDPv4:[" + address2 + "]:" + port2)

    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-x', 'UDP_server_two@' + XML_file_path]

    output, err, exit_code = send_command(command)

    EXPECTED_SERVER_ID = "GUID prefix: " + PREFIX.lower()
    print(EXPECTED_SERVER_ID)

    exit_code = check_output(output, err, EXPECTED_SERVER_ID, False, False)
    if exit_code != 0:
        sys.exit(exit_code)

    for add in EXPECTED_SERVER_ADDRESS:
        exit_code = check_output(output, err, add, False, False)
        if exit_code != 0:
            sys.exit(exit_code)
    sys.exit(exit_code)


def test_fast_discovery_prefix_override(fast_discovery_tool):
    """Test that discovery command overrides prefix given in XML file"""

    XML_file_path = 'test_xml_discovery_server_profile.xml'
    default_profile = XML_parse_profile(XML_file_path, "")

    EXPECTED_SERVER_ID = "GUID prefix: 44.53.00.5f.45.50.52.4f.53.49.4d.41"
    EXPECTED_SERVER_ADDRESS = []
    udpv4 = default_profile.getElementsByTagName('udpv4')
    for elem in udpv4:
        address2 = elem.getElementsByTagName('address')[0].firstChild.data
        port2 = elem.getElementsByTagName('port')[0].firstChild.data
        if port2[0] == '0':
            port2 = port2[1:]
        EXPECTED_SERVER_ADDRESS.append("UDPv4:[" + address2 + "]:" + port2)

    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-i', '0', '-x', XML_file_path]
    output, err, exit_code = send_command(command)

    exit_code = check_output(output, err, EXPECTED_SERVER_ID, False, False)
    if exit_code != 0:
        sys.exit(exit_code)

    for add in EXPECTED_SERVER_ADDRESS:
        exit_code = check_output(output, err, add, False, False)
        if exit_code != 0:
            sys.exit(exit_code)

    sys.exit(exit_code)


def test_fast_discovery_locator_address_override(fast_discovery_tool):
    """Test that discovery command overrides locator given in XML file when using -l option"""

    XML_file_path = 'test_xml_discovery_server_profile.xml'
    default_profile = XML_parse_profile(XML_file_path, "")

    prefix = default_profile.getElementsByTagName('prefix')
    PREFIX = prefix[0].firstChild.data
    EXPECTED_SERVER_ID = "GUID prefix: " + PREFIX.lower()
    EXPECTED_SERVER_ADDRESS = []
    EXPECTED_SERVER_ADDRESS.append("UDPv4:[172.168.43.125]:11811")
    XML_SERVER_ADDRESS = []
    udpv4 = default_profile.getElementsByTagName('udpv4')
    for elem in udpv4:
        address2 = elem.getElementsByTagName('address')[0].firstChild.data
        port2 = elem.getElementsByTagName('port')[0].firstChild.data
        if port2[0] == '0':
            port2 = port2[1:]
        XML_SERVER_ADDRESS.append("UDPv4:[" + address2 + "]:" + port2)

    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-x', XML_file_path, '-l', '172.168.43.125']
    output, err, exit_code = send_command(command)

    exit_code = check_output(output, err, EXPECTED_SERVER_ID, False, False)
    if exit_code != 0:
        sys.exit(exit_code)

    for add in EXPECTED_SERVER_ADDRESS:
        exit_code = check_output(output, err, add, False, False)
        if exit_code != 0:
            sys.exit(exit_code)
    for add in XML_SERVER_ADDRESS:
        exit_code = check_output(output, err, add, False, False)
        if exit_code != 3:
            sys.exit(3)

    sys.exit(0)


def test_fast_discovery_locator_override_same_address(fast_discovery_tool):
    """Test that discovery command overrides locator given in XML file even if the address is the same"""

    XML_file_path = 'test_xml_discovery_server_profile.xml'
    default_profile = XML_parse_profile(XML_file_path, "")

    prefix = default_profile.getElementsByTagName('prefix')
    PREFIX = prefix[0].firstChild.data
    EXPECTED_SERVER_ID = "GUID prefix: " + PREFIX.lower()
    EXPECTED_SERVER_ADDRESS = []
    EXPECTED_SERVER_ADDRESS.append("UDPv4:[127.0.0.9]:11811")
    XML_SERVER_ADDRESS = []
    udpv4 = default_profile.getElementsByTagName('udpv4')
    for elem in udpv4:
        address2 = elem.getElementsByTagName('address')[0].firstChild.data
        port2 = elem.getElementsByTagName('port')[0].firstChild.data
        if port2[0] == '0':
            port2 = port2[1:]
        XML_SERVER_ADDRESS.append("UDPv4:[" + address2 + "]:" + port2)

    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-x', XML_file_path, '-l', '127.0.0.9']
    output, err, exit_code = send_command(command)

    exit_code = check_output(output, err, EXPECTED_SERVER_ID, False, False)
    if exit_code != 0:
        sys.exit(exit_code)

    for add in EXPECTED_SERVER_ADDRESS:
        exit_code = check_output(output, err, add, False, False)
        if exit_code != 0:
            sys.exit(exit_code)
    for add in XML_SERVER_ADDRESS:
        exit_code = check_output(output, err, add, False, False)
        if exit_code != 3:
            sys.exit(3)

    sys.exit(0)


def test_fast_discovery_locator_port_override(fast_discovery_tool):
    """Test that discovery command overrides locator given in XML file when using -p option"""

    XML_file_path = 'test_xml_discovery_server_profile.xml'
    default_profile = XML_parse_profile(XML_file_path, "")

    prefix = default_profile.getElementsByTagName('prefix')
    PREFIX = prefix[0].firstChild.data
    EXPECTED_SERVER_ID = "GUID prefix: " + PREFIX.lower()
    EXPECTED_SERVER_ADDRESS = []
    EXPECTED_SERVER_ADDRESS.append("UDPv4:[0.0.0.0]:1234")
    XML_SERVER_ADDRESS = []
    udpv4 = default_profile.getElementsByTagName('udpv4')
    for elem in udpv4:
        address2 = elem.getElementsByTagName('address')[0].firstChild.data
        port2 = elem.getElementsByTagName('port')[0].firstChild.data
        if port2[0] == '0':
            port2 = port2[1:]
        XML_SERVER_ADDRESS.append("UDPv4:[" + address2 + "]:" + port2)

    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-x', XML_file_path, '-p', '1234']
    output, err, exit_code = send_command(command)

    exit_code = check_output(output, err, EXPECTED_SERVER_ID, False, False)
    if exit_code != 0:
        sys.exit(exit_code)

    for add in EXPECTED_SERVER_ADDRESS:
        exit_code = check_output(output, err, add, False, False)
        if exit_code != 0:
            sys.exit(exit_code)
    for add in XML_SERVER_ADDRESS:
        exit_code = check_output(output, err, add, False, False)
        if exit_code != 3:
            sys.exit(3)

    sys.exit(0)


def test_fast_discovery_locator_override_same_port(fast_discovery_tool):
    """Test that discovery command overrides locator given in XML file even if the port is the same"""

    XML_file_path = 'test_xml_discovery_server_profile.xml'
    default_profile = XML_parse_profile(XML_file_path, "")

    prefix = default_profile.getElementsByTagName('prefix')
    PREFIX = prefix[0].firstChild.data
    EXPECTED_SERVER_ID = "GUID prefix: " + PREFIX.lower()
    EXPECTED_SERVER_ADDRESS = []
    EXPECTED_SERVER_ADDRESS.append("UDPv4:[0.0.0.0]:2811")
    XML_SERVER_ADDRESS = []
    udpv4 = default_profile.getElementsByTagName('udpv4')
    for elem in udpv4:
        address2 = elem.getElementsByTagName('address')[0].firstChild.data
        port2 = elem.getElementsByTagName('port')[0].firstChild.data
        if port2[0] == '0':
            port2 = port2[1:]
        XML_SERVER_ADDRESS.append("UDPv4:[" + address2 + "]:" + port2)

    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-x', XML_file_path, '-p', '2811']
    output, err, exit_code = send_command(command)

    exit_code = check_output(output, err, EXPECTED_SERVER_ID, False, False)
    if exit_code != 0:
        sys.exit(exit_code)

    for add in EXPECTED_SERVER_ADDRESS:
        exit_code = check_output(output, err, add, False, False)
        if exit_code != 0:
            sys.exit(exit_code)
    for add in XML_SERVER_ADDRESS:
        exit_code = check_output(output, err, add, False, False)
        if exit_code != 3:
            sys.exit(3)

    sys.exit(0)


def test_fast_discovery_backup(fast_discovery_tool):
    """Test that launches a BACKUP using CLI and XML"""

    XML_file_path = "test_xml_discovery_server_profile.xml"
    EXPECTED_PARTICIPANT_TYPE = "Backup"
    EXPECTED_BACKUP_ID = "GUID prefix: 44.53.00.5f.45.50.52.4f.53.49.4d.41"
    EXPECTED_SERVER_ADDRESS = []
    EXPECTED_SERVER_ADDRESS.append("UDPv4:[0.0.0.0]:11811")

    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-b', '-i', '0']
    output, err, exit_code = send_command(command)

    exit_code = check_output(output, err, EXPECTED_PARTICIPANT_TYPE, False, False)
    if exit_code != 0:
        sys.exit(exit_code)
    exit_code = check_output(output, err, EXPECTED_BACKUP_ID, False, False)
    if exit_code != 0:
        sys.exit(exit_code)
    for add in EXPECTED_SERVER_ADDRESS:
        exit_code = check_output(output, err, add, False, False)
        if exit_code != 0:
            sys.exit(exit_code)

    EXPECTED_XML_BACKUP_ID = "GUID prefix: 44.53.33.5f.45.50.52.4f.53.49.4d.41"
    EXPECTED_XML_SERVER_ADDRESS = []
    EXPECTED_XML_SERVER_ADDRESS.append("UDPv4:[127.0.0.105]:11825")

    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-x', 'UDP_backup@' + XML_file_path]
    output, err, exit_code = send_command(command)

    exit_code = check_output(output, err, EXPECTED_XML_BACKUP_ID, False, False)
    if exit_code != 0:
        sys.exit(exit_code)
    for add in EXPECTED_XML_SERVER_ADDRESS:
        exit_code = check_output(output, err, add, False, False)
        if exit_code != 0:
            sys.exit(exit_code)

    sys.exit(exit_code)


def test_fast_discovery_no_prefix(fast_discovery_tool):
    """Test to set a random GUID when no server ID is provided"""

    XML_file_path = "test_xml_discovery_server_profile.xml"
    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-x', 'UDP_no_prefix@' + XML_file_path]
    output, err, exit_code = send_command(command)
    EXPECTED_OUTPUT = "UDPv4:[127.0.0.5]:11817"
    exit_code = check_output(output, err, EXPECTED_OUTPUT, False, False)
    sys.exit(exit_code)


def test_fast_discovery_no_XML(fast_discovery_tool):
    """Test that checks output when the XML file provided does not exist"""

    XML_file_path = "non_existent_xml_file.xml"
    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-x', XML_file_path]
    output, err, exit_code = send_command(command)

    print("Stats of test:")
    print("STDOUT:")
    print(output)
    print("STDERR:")
    print(err)

    exit_code = check_output(output, err, "Cannot open XML file", True, True)
    sys.exit(exit_code)


def test_fast_discovery_incorrect_participant(fast_discovery_tool):
    """Test that checks failure if the participant is not SERVER/BACKUP"""

    XML_file_path = "test_wrong_xml_discovery_server_profile.xml"
    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-x', 'UDP_simple@' + XML_file_path]
    output, err, exit_code = send_command(command)

    exit_code = check_output(output, err, "The provided configuration is not valid", True, True)
    if exit_code != 0:
        sys.exit(exit_code)

    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-x', 'UDP_client@' + XML_file_path]
    output, err, exit_code = send_command(command)

    exit_code = check_output(output, err, "The provided configuration is not valid", True, True)
    sys.exit(exit_code)


def test_fast_discovery_several_server_ids(fast_discovery_tool):
    """Test failure when several Server IDs are provided"""

    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-i', '0', '-i', '1']
    output, err, exit_code = send_command(command)
    exit_code = check_output(output, err, "only one server id can be specified", True, True)
    sys.exit(exit_code)


def test_fast_discovery_invalid_locator(fast_discovery_tool):
    """Test failure when the locator is invalid"""

    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-i', '0', '-l', '256.0.0.1']
    output, err, exit_code = send_command(command)
    exit_code = check_output(output, err, "Invalid listening locator address specified", True, True)
    sys.exit(exit_code)


def test_fast_discovery_non_existent_profile(fast_discovery_tool):
    """Test failure when the profile does not exist in the XML file"""

    XML_file_path = "test_xml_discovery_server_profile.xml"
    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-x', 'non_existent_profile@' + XML_file_path]
    output, err, exit_code = send_command(command)
    exit_code = check_output(output, err, "Error loading specified profile from XML file", True, True)
    sys.exit(exit_code)

def test_fast_discovery_security_disabled(fast_discovery_tool):
    """Test failure when Security is YES without being secure"""

    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-i', '0']
    output, err, exit_code = send_command(command)
    if exit_code != 0:
        print(output)
        sys.exit(exit_code)

    EXPECTED_OUTPUT = "UDPv4:[0.0.0.0]:11811"
    exit_code = check_output(output, err, EXPECTED_OUTPUT, True, False)
    sys.exit(exit_code)

def test_fast_discovery_security_enabled_xml_prefix(fast_discovery_tool):
    """Test failure when the printed guid is not the specified in the XML file"""

    XML_file_path = "test_xml_secure_discovery_server_profile.xml"
    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-x', XML_file_path]
    output, err, exit_code = send_command(command)
    if exit_code != 0:
        print(output)
        sys.exit(exit_code)
    EXPECTED_OUTPUT = "UDPv4:[127.0.0.1]:32823"
    exit_code = check_output(output, err, EXPECTED_OUTPUT, False, False)
    sys.exit(exit_code)

def test_fast_discovery_security_enabled_cli_prefix(fast_discovery_tool):
    """Test failure when the printed guid is not the specified in the XML file"""

    XML_file_path = "test_xml_secure_discovery_server_profile.xml"
    command = [fast_discovery_tool, str(command_to_int_test[Command_test.SERVER]), '-i', '0', '-x', 'secure_ds_no_prefix@' + XML_file_path]
    output, err, exit_code = send_command(command)
    if exit_code != 0:
        print(output)
        sys.exit(exit_code)
    EXPECTED_OUTPUT = "UDPv4:[127.0.0.1]:32823"
    exit_code = check_output(output, err, EXPECTED_OUTPUT, False, False)
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
        'test_fast_discovery_closure': lambda:
            test_fast_discovery_closure(args.binary_path),
        'test_fast_discovery_udpv6_address': lambda:
            test_fast_discovery_udpv6_address(args.binary_path),
        'test_fast_discovery_parse_XML_file_default_profile': lambda:
            test_fast_discovery_parse_XML_file_default_profile(args.binary_path),
        'test_fast_discovery_parse_XML_file_URI_profile': lambda:
            test_fast_discovery_parse_XML_file_URI_profile(args.binary_path),
        'test_fast_discovery_prefix_override': lambda:
            test_fast_discovery_prefix_override(args.binary_path),
        'test_fast_discovery_locator_address_override': lambda:
            test_fast_discovery_locator_address_override(args.binary_path),
        'test_fast_discovery_locator_override_same_address': lambda:
            test_fast_discovery_locator_override_same_address(args.binary_path),
        'test_fast_discovery_locator_port_override': lambda:
            test_fast_discovery_locator_port_override(args.binary_path),
        'test_fast_discovery_locator_override_same_port': lambda:
            test_fast_discovery_locator_override_same_port(args.binary_path),
        'test_fast_discovery_backup': lambda:
            test_fast_discovery_backup(args.binary_path),
        'test_fast_discovery_no_XML': lambda:
            test_fast_discovery_no_XML(args.binary_path),
        'test_fast_discovery_incorrect_participant': lambda:
            test_fast_discovery_incorrect_participant(args.binary_path),
        'test_fast_discovery_no_prefix': lambda:
            test_fast_discovery_no_prefix(args.binary_path),
        'test_fast_discovery_several_server_ids': lambda:
            test_fast_discovery_several_server_ids(args.binary_path),
        'test_fast_discovery_invalid_locator': lambda:
            test_fast_discovery_invalid_locator(args.binary_path),
        'test_fast_discovery_non_existent_profile': lambda:
            test_fast_discovery_non_existent_profile(args.binary_path),
        'test_fast_discovery_security_disabled': lambda:
            test_fast_discovery_security_disabled(args.binary_path),
        'test_fast_discovery_security_enabled_xml_prefix': lambda:
            test_fast_discovery_security_enabled_xml_prefix(args.binary_path),
        'test_fast_discovery_security_enabled_cli_prefix': lambda:
            test_fast_discovery_security_enabled_cli_prefix(args.binary_path)
    }

    tests[args.test_name]()
