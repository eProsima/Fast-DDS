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

    for test in tests_definition:

        if 'kind' not in test.keys():
            print('ARGUMENT ERROR : '
                  'Test definition requites <kind> field for each participant')
            continue

        if test['kind'] == 'publisher':
            pub_args.append(['publisher'])

        elif test['kind'] == 'subscriber':
            sub_args.append(['subscriber'])

        else:
            print('ARGUMENT ERROR : '
                  '<kind> field can be publisher/subscriber')

    return pub_args, sub_args


def define_commands(pub_args, sub_args):
    """Create commands for each test addind executable to args."""
    files = glob.glob(
        os.path.join(
            script_dir,
            '**/DDSXtypesCommunication*'),
        recursive=True)
    pf = iter(files)
    command = next(pf, None)
    while command and \
        (not os.path.isfile(command)
            or not os.access(command,
            os.X_OK)):
        command = next(pf, None)

    
    # Add executable to each command
    return (
        [[command] + args for args in pub_args],
        [[command] + args for args in sub_args]
    )


def execute_command(command):
    """Execute command."""
    return subprocess.Popen(command)


def execute_commands(pub_commands, sub_commands, logger):
    """Get test definitions in command lists and execute each process."""
    pubs_proc = []
    subs_proc = []

    for subscriber_command in sub_commands:
        logger.info(f'Executing subcriber: {subscriber_command}')
        subs_proc.append(execute_command(subscriber_command))

    for publisher_command in pub_commands:
        logger.info(f'Executing publisher: {publisher_command}')
        pubs_proc.append(execute_command(publisher_command))

    ret_value = 0

    for proc in subs_proc:
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

    logger.error("test_definition")
    test_definitions = test_definition(args[0])

    logger.error(test_definitions)

    logger.error("define_args")
    pub_args, sub_args = define_args(test_definitions)

    logger.error(pub_args)
    logger.error(sub_args)

    logger.error("define_commands")
    pub_commands, sub_commands = define_commands(pub_args, sub_args)

    logger.error(pub_commands)
    logger.error(sub_commands)

    logger.error("execute_commands")
    test_value = execute_commands(
        pub_commands,
        sub_commands,
        logger)

    logger.error(test_value)

    if test_value != 0:
        sys.exit(1)
    else:
        sys.exit(0)
