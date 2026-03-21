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
fastdds shm sub-command.

This verb provides maintenance tasks related with Fast DDS shared-memory
transport.

usage: fastdds shm [<shm-command>]

shm-commands:

    clean     clean SHM zombie files

positional arguments:
    command     shm-command to run

optional arguments:
    -h, --help  show this help message and exit

"""

import argparse

from shm.clean import Clean


class Parser:
    """Shared-memory sub-commands parser."""

    __help_message = """fastdds shm [<shm-command>]\n\n
    shm-commands:\n\n
    \tclean     clean SHM zombie files
    """

    def __init__(self, argv):
        """Parse the sub-command and dispatch to the appropriate handler.

        Shows usage if no sub-command is specified.

        Supported sub-commands:

            clean   clean SHM zombie files

        param argv list(str):
            list containing the arguments for the command
        """
        parser = argparse.ArgumentParser(
            usage=self.__help_message,
            add_help=True
        )

        parser.add_argument('command', nargs='?', help='shm-command to run')
        parser.add_argument('-f', '--force', action='store_true', help='Force the clean of data sharing segments')

        args = parser.parse_args(argv)

        if args.command is not None:
            if args.command == 'clean':
                Clean().run(args.force)
            else:
                print('shm-command ' + args.shm_command + ' is not valid')
        else:
            parser.print_help()
