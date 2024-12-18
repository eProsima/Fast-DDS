#!/usr/bin/env python3

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
fastdds CLI tool.

This CLI tool provide a set commands and sub-commands to perform, Fast DDS
related, maintenance / configuration tasks.

usage: fastdds <command> [<command-args>]

    Commands:

        discovery     Server-Client discovery auxiliary generator

        shm           Shared-memory commands

        xml           XML commands

    fastdds <command> [-h] shows command usage


positional arguments:
command     Command to run

optional arguments:
-h, --help  show this help message and exit

"""

import argparse
import sys

from discovery.parser import Parser as DiscoveryParser

from shm.parser import Parser as ShmParser


class FastDDSParser:
    """FastDDS tool parser."""

    __required_python_version = (3, 5)

    __help_message = """fastdds <command> [<command-args>]\n\n
    Commands:\n\n
    \tdiscovery     Server-Client discovery auxiliary generator\n
    \tshm           Shared-memory commands\n
    \txml           XML commands\n
    fastdds <command> [-h] shows command usage
    """

    def __init__(self):
        """Parse sys.argv[1:2].

        Parses the <command> and dispatches to the appropriate handler.
        Shows usage if no command is specified.
        """
        self.__check_python_version()

        parser = argparse.ArgumentParser(
            usage=self.__help_message,
            add_help=True
        )

        parser.add_argument('command',
                            nargs='?',
                            help='Command to run')
        parser.add_argument('-v', '--version', action='store_true', help='Print Fast DDS version')

        args = parser.parse_args(sys.argv[1:2])

        if args.command is not None:
            if not hasattr(self, args.command):
                print(f"Invalid command: '{args.command}'. Use 'fastdds -h' for help.")
            else:
                getattr(self, args.command)()
        elif args.version:
            DiscoveryParser(['-v'])
        else:
            parser.print_help()

    def __check_python_version(self):
        """Assert python version is valid."""
        req_v = self.__required_python_version
        v = sys.version_info
        if not (
                    ((v[0] == req_v[0]) and (v[1] >= req_v[1])) or
                    (v[0] > req_v[0])
               ):
            print('fastdds: Invalid Python version. {}.{}.x or greater'
                  ' is required'.format(req_v[0], req_v[1]))
            sys.exit(1)

    def shm(self):
        """Shared-memory command handler."""
        ShmParser(sys.argv[2:])

    def discovery(self):
        """Discovery server command handler."""
        DiscoveryParser(sys.argv[2:])

    def xml(self):
        """
        XML validation command handler.

        New python dependency (XMLSchema) included in 2.10.X
        Check it is installed, and report installation command if it is not.
        """
        try:
            from xml_ci.parser import XMLParser
            XMLParser(sys.argv[2:])
        except ImportError:
            sys.exit(1)

if __name__ == '__main__':

    FastDDSParser()
