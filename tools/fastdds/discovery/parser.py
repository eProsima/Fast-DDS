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
    fastdds discovery verb parser.

    The parser just forward the sub-commands to the fast-discovery-server
    tool application.

"""

import os
import re
import subprocess
import sys
from pathlib import Path


class Parser:
    """Discovery server tool parser."""

    __help_message = 'fastdds discovery <discovery-args>\n\n'

    def __init__(self, argv):
        """
        Parse the sub-command and dispatch to the appropriate handler.

        Shows usage if no sub-command is specified.

        """
        tool_path = str(self.__find_tool_path().resolve())

        try:
            result = subprocess.run(
                [tool_path],
                stdout=subprocess.PIPE,
                universal_newlines=True
            )

            if result.returncode != 0:
                sys.exit(result.returncode)

            if (
                    len(argv) == 0 or
                    (len(argv) == 1 and argv[0] == '-h') or
                    (len(argv) == 1 and argv[0] == '--help')
               ):
                print(self.__edit_tool_help(result.stdout))
            else:
                # Call the tool
                result = subprocess.run([tool_path] + argv)
                if result.returncode != 0:
                    sys.exit(result.returncode)

        except KeyboardInterrupt as e:
            # it lets the subprocess to handle the exception
            pass

        except BaseException as e:
            self.__help_message += str(e)
            self.__help_message += '\n fast-discovery-server tool not found!'
            print(self.__help_message)
            sys.exit(1)

    def __find_tool_path(self):
        """
        Calculate the path to the fast-discovery-server tool.

        returns str:
            Full path to the executable

        """
        tool_path = Path(os.path.dirname(os.path.realpath(__file__)))
        # We asume the installion path is relative to our installation path
        tool_path = tool_path / '../../../bin'
        if os.name == 'posix':
            ret = tool_path / 'fast-discovery-server'
            if not os.path.exists(ret):
                print('fast-discovery-server tool not installed')
                sys.exit(1)
        elif os.name == 'nt':
            ret = tool_path / 'fast-discovery-server.exe'
            if not os.path.exists(ret):
                ret = tool_path / 'fast-discovery-server.bat'
                if not os.path.exists(ret):
                    print('fast-discovery-server tool not installed')
                    sys.exit(1)
        else:
            print(f'{os.name} not supported')
            sys.exit(1)

        return ret

    def __edit_tool_help(self, usage_text):
        """Find and replace the tool-name by fastdds discovery."""
        m = re.search('Usage: ([a-zA-Z0-9-\\.]*)\\s', usage_text)
        if m:
            tool_name = m.group(1)
            return re.sub(tool_name, 'fastdds discovery', usage_text)

        return usage_text
