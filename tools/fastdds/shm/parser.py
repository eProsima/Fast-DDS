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

from shm.clean import Clean


class Parser:
    """Shared-memory sub-commands parser."""

    def __init__(self, argv):
        """Parse the sub-command and dispatch to the appropiate handler.

        Shows usage if no sub-command is specified.
        """
        parser = argparse.ArgumentParser(
            description='Shared-memory commands',
            usage=""" shm <sub-command> [<args>]

            shm sub-commands:

                clean     clean SHM zombie files

            """)

        parser.add_argument('command', help='Command to run')
        args = parser.parse_args(argv)

        if args.command == 'clean':
            Clean().run()
        else:
            print('shm: ' + args.command + ' sub-command is not valid')
