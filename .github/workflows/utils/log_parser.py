# Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

"""Script to parse the colcon test output file."""

import argparse
import re

from tomark import Tomark

DESCRIPTION = """Script to read a test log and return a summary table"""
USAGE = ('python3 log_parser.py')


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

    required_args = parser.add_argument_group('required arguments')
    required_args.add_argument(
        '-i',
        '--log-file',
        type=str,
        required=True,
        help='Path to test log file.'
    )
    required_args.add_argument(
        '-e',
        '--specific-error-file',
        type=str,
        required=True,
        help='Path to file with ASAN or TSAN specific errors.'
    )
    required_args.add_argument(
        '-o',
        '--output-file',
        type=str,
        required=True,
        help='Path to output file.'
    )

    parser.add_argument(
        '-s',
        '--sanitizer',
        type=str,
        required=True,
        help='Sanitizer to use. [ASAN|TSAN] no case sensitive.'
    )

    return parser.parse_args()


def failure_test_list(
        log_file_path: str):

    # failed tests
    saved_lines = []
    with open(log_file_path, 'r') as file:
        for line in reversed(file.readlines()):
            saved_lines.append(line)
            if (re.search('.*The following tests FAILED:.*', line)):
                break

    # Exit if no test failed
    if (not saved_lines):
        return {}

    failed_tests = []
    for test in saved_lines:
        if (re.search(r'\d* - .* \(.+\)', test)):
            split_test = test.strip().split()
            failed_tests.insert(
                0,
                dict({
                    'ID': split_test[-4],
                    'Name': split_test[-2],
                    'Type': test[test.find('(')+1:test.find(')')]
                }))
    return failed_tests


def asan_line_splitter(
        line: str):
    return line[line.find('==ERROR: '):].strip()


def tsan_line_splitter(
        line: str):
    return line[line.find('WARNING: ThreadSanitizer: '):line.find(' (pid=')].strip()


def common_specific_errors_list(
        errors_file_path: str,
        line_splitter):
    return [
        {'Error': k, 'Repetitions': v}
        for k, v
        in common_specific_errors_dict(
            errors_file_path,
            line_splitter).items()]


def common_specific_errors_dict(
        errors_file_path: str,
        line_splitter):

    # failed tests
    errors = {}
    with open(errors_file_path, 'r') as file:
        for line in file.readlines():
            error_id = line_splitter(line)
            if error_id in errors:
                errors[error_id] += 1
            else:
                errors[error_id] = 1

    return errors


def print_list_to_markdown(
        title: str,
        result: list,
        output_file_path: str):

    # Convert python dict to markdown table
    md_table = Tomark.table(result)
    print(md_table)

    # Save table of failed test to output summary file
    with open(output_file_path, 'a') as file:
        file.write(f'\n##{title}\n')
        file.write(f'\n{md_table}')


def main():

    # Parse arguments
    args = parse_options()

    # Get specific ASAN or TSAN variables
    asan = args.sanitizer.lower() == 'asan'
    line_splitter = (asan_line_splitter if asan else tsan_line_splitter)
    file_title = ('ASAN' if asan else 'TSAN') + ' Errors Summary'

    # Execute specific errors parse
    specific_errors = common_specific_errors_list(
        errors_file_path=args.specific_error_file,
        line_splitter=line_splitter)
    print_list_to_markdown(
        title=file_title,
        result=specific_errors,
        output_file_path=args.output_file)

    # Execute failed tests
    tests_failed = failure_test_list(
        log_file_path=args.log_file)
    print_list_to_markdown(
        title='Tests failed',
        result=tests_failed,
        output_file_path=args.output_file)


if __name__ == '__main__':
    main()
