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
        help='Path to file with ASAN, TSAN or UBSAN specific errors.'
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
        help='Sanitizer to use. [ASAN|TSAN|UBSAN] no case sensitive.'
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


# Each UBSan finding starts with a source location followed by " runtime error: <description>".
# We group findings by "<basename>:<line>:<col>: <description>" so that the same UB at the
# same site collapses into a single row regardless of repetition or ctest line prefixes
_UBSAN_LINE_RE = re.compile(
    r'(?P<path>\S+?):(?P<line>\d+):(?P<col>\d+):\s*runtime error:\s*(?P<desc>.+?)\s*$'
)
# Strip GitHub Actions ISO-8601 timestamp prefix and/or a ctest line prefix ("<test-id>: ").
_GH_TIMESTAMP_RE = re.compile(r'^\d{4}-\d{2}-\d{2}T\S+\s+')
_CTEST_PREFIX_RE = re.compile(r'^\d+:\s*')
# Hex addresses ("0x55ade3179480") differ across runs but represent the same logical UB;
# normalize them to "0x..." so identical findings at the same site collapse into one row.
_HEX_ADDRESS_RE = re.compile(r'0x[0-9a-fA-F]+')


def ubsan_line_splitter(
        line: str):
    stripped = _GH_TIMESTAMP_RE.sub('', line.rstrip('\r\n'))
    stripped = _CTEST_PREFIX_RE.sub('', stripped)
    match = _UBSAN_LINE_RE.search(stripped)
    if not match:
        # Not an UBSan finding header
        return None
    basename = match.group('path').rsplit('/', 1)[-1]
    desc = _HEX_ADDRESS_RE.sub('0x...', match.group('desc'))
    return f"{basename}:{match.group('line')}:{match.group('col')}: {desc}"


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

    # Open with newline='' so Python's universal-newline mode does not split a single
    # grep-emitted line on bare '\r' characters.
    errors = {}
    with open(errors_file_path, 'r', newline='') as file:
        for line in file:
            error_id = line_splitter(line)
            if error_id is None:
                continue
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

    # Select the splitter and report title for the requested sanitizer.
    sanitizer = args.sanitizer.lower()
    splitters = {
        'asan': (asan_line_splitter, 'ASAN'),
        'tsan': (tsan_line_splitter, 'TSAN'),
        'ubsan': (ubsan_line_splitter, 'UBSAN'),
    }
    if sanitizer not in splitters:
        raise SystemExit(
            f"Unsupported sanitizer '{args.sanitizer}'. Expected one of: "
            f"{', '.join(s.upper() for s in splitters)}.")
    line_splitter, title_prefix = splitters[sanitizer]
    file_title = f'{title_prefix} Errors Summary'

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
