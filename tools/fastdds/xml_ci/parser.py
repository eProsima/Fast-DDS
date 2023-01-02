# Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

"""
fastdds xml sub-command.

This verb validates the Fast DDS XML profiles files.

usage: fastdds xml [<xml-command>]

xml-commands:

    validate    validate XML profiles files using an XSD schema

positional arguments:
    command     xml-command to run

optional arguments:
    -h, --help  show this help message and exit
"""

import argparse
import os.path
import platform

from xml_ci.validate import Validate


class XMLParser:
    """XML sub-commands parser."""

    def __init__(self, argv):
        """Parse the sub-command and dispatch to the appropriate handler.

        Shows usage if no sub-command is specified.

        Supported sub-commands:

            validate   validate XML profiles files using an XSD schema

        param argv list(str):
            list containing the arguments for the command
        """
        parser = argparse.ArgumentParser(
            formatter_class=argparse.ArgumentDefaultsHelpFormatter,
            description="""
                Script for the validation of the XML files that define the
                profiles of the DDS entities in Fast DDS.
            """,
            usage='fastdds xml [<xml-command>]',
        )
        parser.add_argument(
            'xml_command',
            nargs='*',
            help='validate XML profiles files using an XSD schema'
        )
        parser.add_argument(
            '-d',
            '--debug',
            action='store_true',
            help='print debug info'
        )
        parser.add_argument(
            '-x',
            '--xsd-file',
            required=True,
            help='Fast DDS XSD schema for validation'
        )
        args = parser.parse_args(argv)

        # args.xsd_file = self.__xsd_dir()

        if not os.path.isfile(args.xsd_file):
            print(f'The XSD schema does not exist: {args.xsd_file}')
            exit(1)

        if args.xml_command:
            if args.xml_command[0] == 'validate':
                args.xml_command.pop(0)
                Validate(args.xsd_file).run(args.xml_command)
            else:
                print(f'xml-command "{args.xml_command[0]}" is not valid')
        else:
            parser.print_help()
            exit(1)

    def __xsd_dir(self):
        """
        Calculate the xsd directory.

        :return: The path to the platform specific the XSD directory

        """
        # Windows
        if os.name == 'nt':
            xsd_path = 'c:\\path\\to\\fastRTPS_profiles.xsd\\'
        elif os.name == 'posix':
            # MAC
            if platform.mac_ver()[0] != '':
                xsd_path = 'path/to/fastRTPS_profiles.xsd'
            # Linux
            else:
                xsd_path = 'path/to/fastRTPS_profiles.xsd'
        else:
            raise RuntimeError(f'{os.name} not supported')

        return xsd_path
