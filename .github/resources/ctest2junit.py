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

"""Script to transform CTest native XML reports to jUnit if they do not exist"""

import argparse

from lxml import etree
import os

DESCRIPTION = """Script to transform CTest native XML reports to jUnit"""
USAGE = ('python3 ctest2junit.py')


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
        '-b',
        '--build-dir',
        type=str,
        required=True,
        help='Path to build directory'
    )
    required_args.add_argument(
        '-x',
        '--xslt',
        type=str,
        required=True,
        help='Path to XSLT translation file.'
    )
    required_args.add_argument(
        '-o',
        '--output-junit',
        type=str,
        required=True,
        help='Path to output jUnit file. If the file exists, the script takes no action'
    )
    required_args.add_argument(
        '-t',
        '--timestamp',
        type=str,
        required=True,
        help='Timestamp'
    )
    return parser.parse_args()


def find_ctest_report(build_dir):
    """
    Find the last ctest report in a given CMake build directory.

    :param build_dir: The CMake build directory

    :return: The path to latests CTest report on build_dir if any; else None
    """
    ret = None
    testing_dir = os.path.join(build_dir, 'Testing')

    # Check if build_dir/Testing exists
    if os.path.exists(testing_dir):
        tag_file = os.path.join(testing_dir, 'TAG')

        # CTest generates the reports if build_dir/Testing/{label}/Test.xml,
        # where {label} is specified in the first line of a file build_dir/Testing/TAG
        # Look for that build_dir/Testing/{label}/Test.xml.
        if os.path.isfile(tag_file):
            # Extract the {label} from the TAG file
            with open(tag_file, 'r') as f:
                tag_content = f.readline().strip()
                test_dir = os.path.join(testing_dir, tag_content)

                # Check that Test.xml exists
                test_xml = os.path.join(test_dir, 'Test.xml')

                if os.path.isfile(test_xml):
                    ret = test_xml

    return ret


def translate(original_xml, xsl_file, timestamp):
    """
    Translate an XML from one spec to another using an XSLT file.

    :param original_xml: The XML to translate
    :param xsl_file: The XSLT transformation file

    :return: A stream containing the translated XML
    """
    var = timestamp
    xml = etree.parse(original_xml)
    xslt = etree.parse(xsl_file)
    transform = etree.XSLT(xslt)

    try:
        # Apply the transformation with the new context
        result_tree = transform(xml, var1=etree.XSLT.strparam(var))
        return str(result_tree)
    except Exception as e:
        for error in transform.error_log:
            print(error.message, error.line)
        raise e


def write_to_file(stream, filename):
    """
    Write a stream to a file.

    :param stream: The stream to write
    :param filename: The destination file
    """
    with open(filename, 'w') as f:
        f.write(stream)


if __name__ == '__main__':

    args = parse_options()
    print(args.timestamp)
    exit_code = 0

    if os.path.isfile(args.output_junit):
        print(f'File {args.output_junit} already exists. No action taken.')

    else:
        exit_code = 1
        ctest_report = find_ctest_report(args.build_dir)

        if ctest_report:
            junit = translate(ctest_report, args.xslt, args.timestamp)
            write_to_file(junit, args.output_junit)
            exit_code = 0

    exit(exit_code)
