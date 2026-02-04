# Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

import argparse
import asyncio
import json
import logging
import os
import shutil
import subprocess
import sys
import time

from asyncio.subprocess import PIPE

# Max default time to kill a process in case it gets stucked
# This is done by ctest automatically, but this script could be
# run independently of ctest
MAX_TIME = 60*5

# Define the directory of the script
script_dir = os.path.dirname(os.path.realpath(__file__))

DESCRIPTION = """Script to run TypeLookupService tests"""
USAGE = ('python3 test_build.py [-d] <JSON file>')

def parse_options():
    """
    Parse arguments.

    :return: The arguments parsed.
    """
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        add_help=True,
        description=(DESCRIPTION),
        usage=(USAGE)
    )

    parser.add_argument(
        '-a',
        '--app',
        type=str,
        required=True,
        help='Path to the executable test.'
    )

    parser.add_argument(
        'file',
        nargs=1,
        help='JSON file.'
    )

    parser.add_argument(
        '-d',
        '--debug',
        action='store_true',
        help='Print test debugging info.'
    )

    return parser.parse_args()


def read_json(file_name):
    """Read json file with test definition."""
    structure_dic = {}
    with open(file_name) as json_file:
        structure_dic = json.load(json_file)
    return structure_dic


def define_args(participant, idx):
    """Use dictionary to get command args for each participant."""
    args = []
    args.extend([f"kind={participant.get('kind')}"])
    args.extend([f"samples={participant.get('samples', 10)}"])
    args.extend([f"timeout={participant.get('timeout', 10)}"])
    args.extend([f"expected_matches={participant.get('expected_matches', 1)}"])
    args.extend([f"seed={str(os.getpid() + idx)}"])
    args.extend([f"builtin_flow_controller_bytes={participant.get('builtin_flow_controller_bytes', 0)}"])

    # Check if 'known_types' key exists and is a list
    if 'known_types' in participant and isinstance(participant['known_types'], list):
        args.append(f'known_types={",".join(participant["known_types"])}')
    else:
        print(f'ARGUMENT ERROR: For {participant["kind"]}s, <known_types> should be a list of types')

    return args


def define_commands(executable_path, test_cases, idx):
    """Create commands for each participant adding executable to args."""
    all_commands = []
    for test_case in test_cases:
        # For each test case, create commands for all participants
        commands = [[executable_path] + define_args(participant, idx) for participant in test_case['participants']]
        all_commands.extend(commands)

    return all_commands


async def read_output(test_case, output, num_lines, index):
    """
    Read an process stream output, printing each line using the internal log.
    Also update the line counter in the num_lines list using the index argument.

    :param[in] output: Process stream output.
    :param[inout] num_lines List with line counters for each process stream output.
    :param[in] index Indicates which line counter must be updated.
    """

    while True:
        try:
            line = await asyncio.wait_for(output.readline(), timeout=None)
        except asyncio.CancelledError:
            pass
        else:
            if line:
                num_lines[index] = num_lines[index] + 1
                logger.info(test_case + ": " + line.decode('utf-8'))
                continue
        break


async def read_outputs(test_case, proc, num_lines):
    """
    Read asynchronously the stdout and stderr of the process.

    :param[in] proc Process whose stream outputs will be read.
    :param[inout] num_lines List with line counters for each process stream output.
    """
    await asyncio.gather(read_output(test_case, proc.stdout, num_lines, 0), read_output(test_case, proc.stderr, num_lines, 1))


async def run_command(test_case, process_args, timeout):
    """
    Execute a process and read its stream outputs asynchronously.

    :param[in] process_args List of process arguments.
    :param[in] environment List of environment variables to be used when executing the process.
    :param[in] timeout Expiration time of the execution.

    :return Tuple (process return code, lines printed on stderr stream output)
    """
    logger.debug("Running command: " + str(process_args))
    proc = await asyncio.create_subprocess_exec(
        *process_args,
        stdout=PIPE,
        stderr=PIPE
    )

    num_lines = [0, 0]

    try:
        await asyncio.wait_for(read_outputs(test_case, proc, num_lines), timeout)
    except (TimeoutError, asyncio.TimeoutError):
        logger.debug("     Timeout");
        pass

    return await proc.wait()


async def execute_commands(test_case, commands):
    """
    Execute a list of commands asynchronously.

    :param[in]  test_case  Name of the test case.
    :param[in]  commands   List of commands to be executed.

    :return Sum of the return codes of each command.
    """
    tasks = []
    async with asyncio.TaskGroup() as tg:
        for command in commands:
            tasks.append(tg.create_task(run_command(test_case, command, MAX_TIME)))
            await asyncio.sleep(0.3) # Avoid errors with SharedMemory starting all commands at same time

    return sum([proc.result() for proc in tasks])

async def execute_commands_with_sem(test_case, commands, sem):
    """
    Execute commands using a semaphore to limit parallel executions.

   :param[in]  test_case  Name of the test case.
   :param[in]  commands   List of commands to be executed.
   :param[in]  sem        Semaphore to limit parallel executions.
    """
    async with sem:
        return await execute_commands(test_case, commands)

async def execute_test_cases(test_cases):
    """
    Execute all test cases, retrying failed ones up to 3 times.

    :param[in]  test_cases  List of test cases to be executed.

    :return Tuple (total test value, list of successful cases, list of failling cases)
    """
    total_test_value = 0
    successful_cases = []
    failling_cases = []

    # Limit parallel executions to half of CPU cores
    sem = asyncio.Semaphore(os.cpu_count() // 2 or 6)

    pending_cases = test_cases.copy()
    max_test_runs = 3
    while pending_cases and max_test_runs > 0:
        tasks = []
        idx = 0
        async with asyncio.TaskGroup() as tg:
            for test_case in pending_cases:
                # Define commands for each test case
                commands = define_commands(args.app, [test_case], idx)
                # Execute the commands in parallel
                tasks.append(tg.create_task(execute_commands_with_sem(test_case['TestCase'], commands, sem)))
                idx += 1

        next_pending_cases = []
        max_test_runs -= 1

        # Collect results
        for test_case, task in zip(pending_cases, tasks):
            test_value = task.result()
            total_test_value += test_value
            if test_value == 0:
                successful_cases.append(f"Test {test_case.get('TestCase')}")
            else:
                next_pending_cases.append(test_case)

        # If there are pending cases, they will be retried
        pending_cases = next_pending_cases
        if pending_cases.__len__() > 0 and max_test_runs > 0:
            # Inform about cases to be retried
            logger.info("----------- RETRYING CASES -----------")
            for failed_test in pending_cases:
                logger.info(f"Test {failed_test.get('TestCase')}")

            # Wait a bit before retrying
            await asyncio.sleep(2)
            total_test_value = 0

    # If there are still pending cases, they are considered failling
    for test_case in pending_cases:
        failling_cases.append(f"Test {test_case.get('TestCase')}")

    return total_test_value, successful_cases, failling_cases


if __name__ == '__main__':

    # Parse arguments
    args = parse_options()

    # Create a custom logger
    logger = logging.getLogger('TypeLookupServiceTester')
    # Create handlers
    l_handler = logging.StreamHandler()
    # Create formatters and add it to handlers
    l_format = '[%(asctime)s][%(name)s][%(levelname)s] %(message)s'
    l_format = logging.Formatter(l_format)
    l_handler.setFormatter(l_format)
    # Add handlers to the logger
    logger.addHandler(l_handler)
    # Set log level
    if args.debug:
        logger.setLevel(logging.DEBUG)
    else:
        logger.setLevel(logging.INFO)

    # Read test cases from the provided JSON file
    test_cases = read_json(args.file[0])['test_cases']

    total_test_value, successful_cases, failling_cases = asyncio.run(execute_test_cases(test_cases))

    # Print the results
    if successful_cases.__len__() > 0:
        logger.info("---------- SUCCESSFUL CASES ----------")
        for successful_test in successful_cases:
            logger.info(successful_test)

    if failling_cases.__len__() > 0:
        logger.info("----------- FAILLING CASES -----------")
        for failed_test in failling_cases:
            logger.info(failed_test)

    # Exit with appropriate exit code based on total test value
    if total_test_value != 0:
        sys.exit(1)
    else:
        sys.exit(0)
