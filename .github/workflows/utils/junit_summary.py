# Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

"""Script to parse the jUnit test results and create a summary."""

import argparse
import xml.etree.ElementTree as ET

DESCRIPTION = """Script to parse the jUnit test results and create a summary"""
USAGE = ('python3 junit_summary.py')


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
        '-j',
        '--junit-report',
        type=str,
        required=True,
        help='Path to junit report file.'
    )
    required_args.add_argument(
        '-o',
        '--output-file',
        type=str,
        required=True,
        help='Path to output file.'
    )

    return parser.parse_args()

def junit_report_to_dict(junit_report):
    """Convert a jUnit report to a dictionary."""
    result = {
        'passed_tests': [],
        'failed_tests': [],
        'disabled_tests': [],
        'skipped_tests': [],
    }
    tree = ET.parse(junit_report)
    root = tree.getroot()
    result['tests'] = root.attrib['tests']
    result['failures'] = root.attrib['failures']
    result['disabled'] = root.attrib['disabled']
    result['skipped'] = root.attrib['skipped']
    result['time'] = root.attrib['time']
    result['timestamp'] = root.attrib['timestamp']

    for child in root:
        if child.tag == 'testcase':
            if child.attrib['status'] == "run":
                result['passed_tests'].append(child.attrib['name'])

            elif child.attrib['status'] == "fail":
                result['failed_tests'].append(child.attrib['name'])

            elif child.attrib['status'] == "ignored":
                result['disabled_tests'].append(child.attrib['name'])

            elif child.attrib['status'] == "notrun":
                result['skipped_tests'].append(child.attrib['name'])

    return result

def create_md_summary(results_dict):
    """Create Markdown summary from results."""
    # Test summary
    summary = '## Test summary\n'

    # Table header
    summary += '|Total number of tests|Test failures|Disabled test|Skipped test|Spent time|Timestamp|\n'
    summary += '|-|-|-|-|-|-|\n'

    # Entry
    summary += f'|{results_dict["tests"]}'
    summary += f'|{results_dict["failures"]}'
    summary += f'|{results_dict["disabled"]}'
    summary += f'|{results_dict["skipped"]}'
    summary += f'|{results_dict["time"]}'
    summary += f'|{results_dict["timestamp"]}'
    summary += '|\n'

    # Failed tests list
    if len(results_dict['failed_tests']) != 0:
        summary += '\n## Failed tests\n'
        for failed_test in results_dict['failed_tests']:
            summary += f'* {failed_test}\n'

    # Disabled tests list
    if len(results_dict['disabled_tests']) != 0:
        summary += '\n## Disabled tests\n'
        for failed_test in results_dict['disabled_tests']:
            summary += f'* {failed_test}\n'

    # Skipped tests list
    if len(results_dict['skipped_tests']) != 0:
        summary += '\n## Skipped tests\n'
        for failed_test in results_dict['skipped_tests']:
            summary += f'* {failed_test}\n'

    return summary


if __name__ == '__main__':
    # Parse arguments
    args = parse_options()
    results = junit_report_to_dict(args.junit_report)

    # Write output
    with open(args.output_file, 'a') as file:
        file.write(create_md_summary(results))

    # Exit code is the number of failed tests
    exit(results['failures'])
