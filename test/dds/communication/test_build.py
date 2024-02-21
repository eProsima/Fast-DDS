"""Execute a DDS communictaion test following a json definition file."""
# Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
import json
import logging
import os
import subprocess
import sys
import time

script_dir = os.path.dirname(os.path.realpath(__file__))
seed = str(os.getpid())


def read_json(file_name):
    """Read json file with test definition."""
    structure_dic = {}
    with open(file_name) as json_file:
        structure_dic = json.load(json_file)
    return structure_dic


def test_definition(file_name):
    """Return a list with each test process defined in dictionary."""
    return read_json(file_name)['participants']


def define_args(tests_definition):
    """Use list dictionary to get commands args for each test process."""
    sub_args = []
    pub_args = []
    pubsub_args = []

    for test in tests_definition:

        if 'kind' not in test.keys():
            print('ARGUMENT ERROR : '
                  'Test definition requires <kind> field for each participant')
            continue

        # All processes has seed argument
        test_arguments = ['--seed', seed]

        possible_arguments = ['samples',
                              'wait',
                              'magic',
                              'publishers',
                              'sleep_before_exec',
                              'interval',
                              'timeout']

        for argument in possible_arguments:
            if argument in test.keys():
                test_arguments.append('--' + argument)
                test_arguments.append(test[argument])

        # Add arguments without value
        possible_flags = ['exit_on_lost_liveliness',
                          'zero_copy',
                          'fixed_type',
                          'notexit',
                          'succeed_on_timeout']

        for flag in possible_flags:
            if flag in test.keys():
                test_arguments.append('--' + flag)

        xmlfile_arg = 'xmlfile'
        if xmlfile_arg in test.keys():
            xml_path = os.path.join(script_dir, test[xmlfile_arg])
            test_arguments.append('--' + xmlfile_arg)
            test_arguments.append(xml_path)

        if test['kind'] == 'publisher':
            pub_args.append(test_arguments)

        elif test['kind'] == 'subscriber':
            sub_args.append(test_arguments)

        elif test['kind'] == 'pubsub':
            pubsub_args.append(test_arguments)

        else:
            print('ARGUMENT ERROR : '
                  '<kind> field can be publisher/subscriber/pubsub')

    return pub_args, sub_args, pubsub_args


def define_commands(pub_args, sub_args, pubsub_args):
    """Create commands for each test addind executable to args."""
    # Publisher executable
    publisher_command = os.environ.get(
        'DDS_SIMPLE_COMMUNICATION_PUBLISHER_BIN')
    if not publisher_command:
        publisher_files = glob.glob(
            os.path.join(
                script_dir,
                '**/DDSCommunicationPublisher*'),
            recursive=True)
        pf = iter(publisher_files)
        publisher_command = next(pf, None)
        while publisher_command and \
                (not os.path.isfile(publisher_command)
                 or not os.access(publisher_command,
                 os.X_OK)):
            publisher_command = next(pf, None)
    assert publisher_command

    # Subscriber executable
    subscriber_command = os.environ.get(
        'DDS_SIMPLE_COMMUNICATION_SUBSCRIBER_BIN')
    if not subscriber_command:
        subscriber_files = glob.glob(
            os.path.join(
                script_dir,
                '**/DDSCommunicationSubscriber*'),
            recursive=True)
        pf = iter(subscriber_files)
        subscriber_command = next(pf, None)
        while subscriber_command and \
            (not os.path.isfile(subscriber_command)
             or not os.access(subscriber_command,
             os.X_OK)):
            subscriber_command = next(pf, None)
    assert subscriber_command

    # Pub Sub executable
    pubsub_command = os.environ.get('DDS_SIMPLE_COMMUNICATION_PUBSUB_BIN')
    if not pubsub_command:
        pubsub_files = glob.glob(
            os.path.join(
                script_dir,
                '**/DDSCommunicationPubSub*'),
            recursive=True)
        pf = iter(pubsub_files)
        pubsub_command = next(pf, None)
        while pubsub_command and \
            (not os.path.isfile(pubsub_command)
             or not os.access(pubsub_command,
             os.X_OK)):
            pubsub_command = next(pf, None)
    assert pubsub_command

    # Add executable to each command
    return (
        [[publisher_command] + args for args in pub_args],
        [[subscriber_command] + args for args in sub_args],
        [[pubsub_command] + args for args in pubsub_args]
    )


def execute_command(command):
    """Execute command after possibly waiting some time."""
    sleep_tag = '--sleep_before_exec'
    if sleep_tag in command:
        time.sleep(int(command.pop(command.index(sleep_tag) + 1)))
        command.remove(sleep_tag)
    return subprocess.Popen(command)


def execute_commands(pub_commands, sub_commands, pubsub_commands, logger):
    """Get test definitions in command lists and execute each process."""
    pubs_proc = []
    subs_proc = []
    pubsubs_proc = []

    for subscriber_command in sub_commands:
        logger.info(f'Executing subcriber: {subscriber_command}')
        subs_proc.append(execute_command(subscriber_command))

    for pubsub_command in pubsub_commands:
        logger.info(f'Executing pubsub: {pubsub_command}')
        pubsubs_proc.append(execute_command(pubsub_command))

    for publisher_command in pub_commands:
        logger.info(f'Executing publisher: {publisher_command}')
        pubs_proc.append(execute_command(publisher_command))

    ret_value = 0

    for proc in subs_proc:
        proc.communicate()
        ret_value = ret_value + proc.returncode

    for proc in pubsubs_proc:
        proc.communicate()
        ret_value = ret_value + proc.returncode

    for proc in pubs_proc:
        proc.kill()

    return ret_value


if __name__ == '__main__':

    logger = logging.getLogger('DDS COMMUNICATION TEST')
    logger.setLevel(logging.INFO)

    logger.error("TEST RUNNING")

    args = sys.argv[1:]

    if len(args) != 1:
        logger.error('ARGUMENTS ERROR : 1 argument required: '
                     'path to .json file with test definition')
        sys.exit(1)

    test_definitions = test_definition(args[0])

    logger.error(test_definitions)

    pub_args, sub_args, pubsub_args = define_args(test_definitions)

    logger.error(pub_args)
    logger.error(sub_args)
    logger.error(pubsub_args)

    pub_commands, sub_commands, pubsub_commands = \
        define_commands(pub_args, sub_args, pubsub_args)

    logger.error(pub_commands)
    logger.error(sub_commands)
    logger.error(pubsub_commands)

    test_value = execute_commands(
        pub_commands,
        sub_commands,
        pubsub_commands,
        logger)

    logger.error(test_value)

    if test_value != 0:
        sys.exit(1)
    else:
        sys.exit(0)
