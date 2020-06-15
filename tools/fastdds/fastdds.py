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

import argparse
import sys

from shm.parser import Parser as ShmParser


class FastDDSParser:
    """FastDDS tool parser."""

    __help_message='''fastdds <command> [<command-args>]\n\n
    Commands:\n\n
    \tshm     shared-memory commands\n
    fastdds <command> [-h] shows command usage
    '''

    def __init__(self):
        """Parse sys.argv[1:2].

        Parses the <command> and dispatch to the appropiate handler.
        Shows usage if no command is specified.
        """
        parser = argparse.ArgumentParser(
            usage=self.__help_message,
            add_help=True
        )

        parser.add_argument('command'
            , nargs='?'
            , help='Command to run'
        )

        args = parser.parse_args(sys.argv[1:2])

        if not args.command is None:
            if not hasattr(self, args.command):
                print('Invalid command')
            else:
                getattr(self, args.command)()
        else:
            parser.print_help()

    def shm(self):
        """Shared-memory command handler."""
        ShmParser(sys.argv[2:])


if __name__ == '__main__':

    FastDDSParser()
