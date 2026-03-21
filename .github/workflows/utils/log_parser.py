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
    with open(log_file_path, 'r', encoding='utf-8', errors='replace') as file:
        for line in reversed(file.readlines()):
            saved_lines.append(line)
            if (re.search('.*The following tests FAILED:.*', line)):
                break

    failed_tests = []
    for test in saved_lines:
        if (re.search(r'\d* - .* \(.+\)', test)):

            # Remove strange chars and break lines and separate by space
            split_test = test.strip().split()

            # NOTE: if there are spaces in the test failure (e.g. Subprocess Aborted)
            # the parse is more difficult as the values to take from split are different
            # Thus, it uses this variable to get the correct # of spaces in error name
            n_spaces_in_error = len(test[test.find('(')+1:test.find(')')].split()) - 1

            # Insert as failed test (in first place, because we are reading them inversed)
            # the new test failed with the name and id taken from split line
            failed_tests.insert(
                0,
                dict({
                    'ID': split_test[- 4 - n_spaces_in_error],
                    'Name': split_test[- 2 - n_spaces_in_error],
                    'Type': test[test.find('(')+1:test.find(')')]
                }))

    n_errors = len(failed_tests)

    if len(failed_tests) == 0:
        failed_tests.insert(
            0,
            dict({
                    'ID': '-1',
                    'Name': 'No tests failed',
                    'Type': 'Hooray'
                }))

    return failed_tests, n_errors


def _common_line_splitter(
        line: str,
        text_to_split_start: str,
        text_to_split_end: str = None) -> str:
    start_index = line.find(text_to_split_start)
    if start_index == -1:
        return line
    elif text_to_split_end:
        end_index = line.find(text_to_split_end)
        if end_index != -1:
            return line[(start_index
                        + len(text_to_split_start)):
                        end_index].strip()
    return line[(start_index + len(text_to_split_start)):].split(None, 1)[0]


def asan_line_splitter(
        line: str):
    return _common_line_splitter(
        line=line,
        text_to_split_start='==ERROR: ')


def tsan_line_splitter(
        line: str):
    return _common_line_splitter(
        line=line,
        text_to_split_start='WARNING: ThreadSanitizer: ',
        text_to_split_end=' (pid=')


def common_specific_errors_list(
        errors_file_path: str,
        line_splitter):
    result = [
        {'Error': k, 'Repetitions': v}
        for k, v
        in common_specific_errors_dict(
            errors_file_path,
            line_splitter).items()]

    n_errors = len(result)

    if len(result) == 0:
        result.append({'Error': 'No errors', 'Repetitions': '0'})

    return result, n_errors


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
        file.write(f'\n## {title}\n')
        file.write(f'\n{md_table}')


def main():

    # Parse arguments
    args = parse_options()

    # Get specific ASAN or TSAN variables
    asan = args.sanitizer.lower() == 'asan'
    line_splitter = (asan_line_splitter if asan else tsan_line_splitter)
    file_title = ('ASAN' if asan else 'TSAN') + ' Errors Summary'

    # Execute specific errors parse
    specific_errors, n_errors = common_specific_errors_list(
        errors_file_path=args.specific_error_file,
        line_splitter=line_splitter)
    print_list_to_markdown(
        title=file_title,
        result=specific_errors,
        output_file_path=args.output_file)

    # Execute failed tests
    tests_failed, _ = failure_test_list(
        log_file_path=args.log_file)
    print_list_to_markdown(
        title='Tests failed',
        result=tests_failed,
        output_file_path=args.output_file)

    return n_errors


if __name__ == '__main__':
    exit(1 if main() else 0)
