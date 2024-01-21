import json
import logging
import os
import shutil
import subprocess
import sys
import time

script_dir = os.path.dirname(os.path.realpath(__file__))


def read_json(file_name):
    """Read json file with test definition."""
    structure_dic = {}
    with open(file_name) as json_file:
        structure_dic = json.load(json_file)
    return structure_dic


def participants_definition(file_name):
    """Return a list with each participant defined in dictionary."""
    return read_json(file_name)['participants']


def define_args(participant):
    """Use dictionary to get command args for each participant."""
    args = []
    args.extend([f"kind={participant.get('kind')}"])
    args.extend([f"samples={participant.get('samples', 10)}"])
    args.extend([f"timeout={participant.get('timeout', 10)}"])
    args.extend([f"expected_types={participant.get('expected_types', 1)}"])

    if 'known_types' in participant and isinstance(participant['known_types'], list):
        args.append(f'known_types={",".join(participant["known_types"])}')
    else:
        print(f'ARGUMENT ERROR: For {participant["kind"]}s, <known_types> should be a list of types')

    return args

def find_executable(executable_name):
    """Find the full path of an executable file by name."""
    executable_path = shutil.which(executable_name)
    if not executable_path or not os.path.isfile(executable_path) or not os.access(executable_path, os.X_OK):
        # Try looking in the current working directory
        executable_path = os.path.join(os.getcwd(), executable_name)
        if not os.path.isfile(executable_path) or not os.access(executable_path, os.X_OK):
            return None
    return executable_path


def define_commands(participants):
    """Create commands for each participant adding executable to args."""
    executable_path = find_executable('DDSXtypesCommunication')
    if not executable_path:
        print("ERROR: Executable 'DDSXtypesCommunication' not found in the system PATH or is not executable.")
        sys.exit(1)

    commands = [[executable_path] + define_args(participant) for participant in participants]

    return commands


def execute_commands(commands, logger):
    """Get test definitions in command lists and execute each process."""
    processes = []

    for command in commands:
        logger.info(f'Executing: {command}')
        processes.append(subprocess.Popen(command))
        time.sleep(1)  # Delay for consistency

    ret_value = 0

    for proc in processes:
        proc.communicate()
        ret_value += proc.returncode

    return ret_value


if __name__ == '__main__':

    logger = logging.getLogger('XTYPES COMMUNICATION TEST')
    logger.setLevel(logging.INFO)

    args = sys.argv[1:]

    if len(args) != 1:
        logger.error('ARGUMENTS ERROR : 1 argument required: path to .json file with test definition')
        sys.exit(1)

    participants = participants_definition(args[0])

    commands = define_commands(participants)

    test_value = execute_commands(commands, logger)

    logger.error(test_value)

    if test_value != 0:
        sys.exit(1)
    else:
        sys.exit(0)
