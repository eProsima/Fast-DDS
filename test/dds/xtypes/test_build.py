import json
import os
import shutil
import subprocess
import sys
import time

# Define the directory of the script
script_dir = os.path.dirname(os.path.realpath(__file__))


def read_json(file_name):
    """Read json file with test definition."""
    structure_dic = {}
    with open(file_name) as json_file:
        structure_dic = json.load(json_file)
    return structure_dic


def define_args(participant):
    """Use dictionary to get command args for each participant."""
    args = []
    args.extend([f"kind={participant.get('kind')}"])
    args.extend([f"samples={participant.get('samples', 10)}"])
    args.extend([f"timeout={participant.get('timeout', 10)}"])
    args.extend([f"expected_matches={participant.get('expected_matches', 1)}"])

    # Check if 'known_types' key exists and is a list
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


def define_commands(test_cases):
    """Create commands for each participant adding executable to args."""
    executable_path = find_executable('DDSXtypesCommunication')
    if not executable_path:
        print("ERROR: Executable 'DDSXtypesCommunication' not found in the system PATH or is not executable.")
        sys.exit(1)

    all_commands = []
    for test_case in test_cases:
        # For each test case, create commands for all participants
        commands = [[executable_path] + define_args(participant) for participant in test_case['participants']]
        all_commands.extend(commands)

    return all_commands


def execute_commands(commands):
    """Execute each process with the given commands."""
    processes = []

    for command in commands:
        processes.append(subprocess.Popen(command))
        time.sleep(0.1)  # Delay for consistency

    ret_value = 0

    for proc in processes:
        proc.communicate()  # Wait for the process to finish
        ret_value += proc.returncode

    return ret_value


if __name__ == '__main__':
    args = sys.argv[1:]

    # Check if the number of arguments is correct
    if len(args) != 1:
        ('ARGUMENTS ERROR: 1 argument required: path to .json file with test definition')
        sys.exit(1)

    # Read test cases from the provided JSON file
    test_cases = read_json(args[0])['test_cases']

    total_test_value = 0
    successful_cases = []
    failling_cases = []

    for test_case in test_cases:
        # Define commands for each test case
        commands = define_commands([test_case])
        # Execute the commands and get the return value
        test_value = execute_commands(commands)
        total_test_value += test_value
        if test_value == 0:
            successful_cases.append(f"Test {test_case.get('TestCase')}")
        else:
            failling_cases.append(f"Test {test_case.get('TestCase')}")

    # Print the results
    if successful_cases.__len__() > 0:
        print("---------- SUCCESSFUL CASES ----------")
        for successful_test in successful_cases:
            print(successful_test)

    if failling_cases.__len__() > 0:
        print("----------- FAILLING CASES -----------")
        for failed_test in failling_cases:
            print(failed_test)

    # Exit with appropriate exit code based on total test value
    if total_test_value != 0:
        sys.exit(1)
    else:
        sys.exit(0)
