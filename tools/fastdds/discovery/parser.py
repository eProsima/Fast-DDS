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

from enum import Enum
from pathlib import Path
import os
import re
import subprocess
import sys

class Command(Enum):
    SERVER = "server" # Does not need to be specified
    AUTO = "auto"
    START = "start"
    STOP = "stop"
    ADD = "add"
    SET = "set"
    LIST = "list"
    INFO = "info"
    UNKNOWN = "unknown"

# This map is used to convert the string command to an integer used in the cpp tool
command_to_int = {
    Command.AUTO: 0,
    Command.START: 1,
    Command.STOP: 2,
    Command.ADD: 3,
    Command.SET: 4,
    Command.LIST: 5,
    Command.INFO: 6,
    Command.SERVER: 42
}

class Parser:
    """Discovery server tool parser."""

    def __init__(self, argv):
        """
        Parse the sub-command and dispatch to the appropriate handler.

        Creates a default UDP server if no sub-command is specified.

        """
        tool_path = str(self.__find_tool_path().resolve())

        try:
            result_help = subprocess.run(
                [tool_path, str(command_to_int[Command.SERVER]), '-h'],
                stdout=subprocess.PIPE,
                universal_newlines=True
            )

            # Check if the tool is available
            if result_help.returncode != 0:
                sys.exit(result.returncode)

            m = re.search('Usage: ([a-zA-Z0-9-\\.]*)\\s', result_help.stdout)
            tool_name = ''
            if m:
                tool_name = m.group(1)

            help_or_examples = False
            # Check for help and/or examples arguments
            for arg in argv:
                if arg in ['-h', '--help']:
                    print(self.__edit_tool_text(result_help.stdout, tool_name))
                    help_or_examples = True
            for arg in argv:
                if arg in ['-e', '--examples']:
                    result_ex = subprocess.run(
                        [tool_path, str(command_to_int[Command.SERVER]), '-e'],
                        stdout=subprocess.PIPE,
                        universal_newlines=True
                    )
                    print(self.__edit_tool_text(result_ex.stdout, tool_name))
                    help_or_examples = True
            if help_or_examples:
                sys.exit(0)

            if (
                (len(argv) == 1 and argv[0] == '-v') or
                (len(argv) == 1 and argv[0] == '--version')
                ):
                result = subprocess.run([tool_path, str(command_to_int[Command.SERVER]), '-v'])
                sys.exit(result.returncode)

            #  Check for multiple keywords error
            if self.__check_for_duplicate_commands(argv):
                sys.exit(1)

            try:
                command_int = command_to_int[Command(sys.argv[2].lower())]
            except:
                # Default value
                command_int = command_to_int[Command.SERVER]

            # Prepare arguments for the C++ program
            args_for_cpp = []
            if command_int == command_to_int[Command.SERVER]:
                args_for_cpp = [str(command_int)] + sys.argv[2:]
            else:
                args_for_cpp = [str(command_int)] + sys.argv[3:]

            # Call the tool
            result = subprocess.run([tool_path] + args_for_cpp)
            if result.returncode != 0:
                sys.exit(result.returncode)

        except KeyboardInterrupt:
            # It lets the subprocess to handle the exception
            pass

        except SystemExit as e:
            sys.exit(e.code)

        except BaseException as e:
            print('\n fast-discovery-server tool not found!')
            sys.exit(1)

    def __find_tool_path(self):
        """
        Calculate the path to the fast-discovery-server tool.

        returns str:
            Full path to the executable

        """
        tool_path = Path(os.path.dirname(os.path.realpath(__file__)))
        # We asume the installation path is relative to Fast DDS' installation path
        tool_path = tool_path / '../../../bin'
        if os.name == 'posix':
            ret = tool_path / 'fast-discovery-server'
            if not os.path.exists(ret):
                print('fast-discovery-server tool not installed')
                sys.exit(1)
        elif os.name == 'nt':
            ret = tool_path / 'fast-discovery-server.exe'
            if not os.path.exists(ret):
                exe_files = [f for f in tool_path.glob('*.exe') if re.match(r'fast-discovery-server.*\.exe$', f.name)]
                if len(exe_files) == 0:
                    print("Unable to find fast-discovery-server tool. Check installation")
                elif len(exe_files) == 1:
                    ret = exe_files[0]
                else:
                    print('Multiple candidates for fast-discovery-server.exe. Check installation')
                    sys.exit(1)
        else:
            print(f'{os.name} not supported')
            sys.exit(1)

        return ret

    def __edit_tool_text(self, usage_text, tool_name):
        """Find and replace the tool-name by fastdds discovery."""
        if tool_name != '':
            return re.sub(tool_name, 'fastdds discovery', usage_text)
        return usage_text

    def __check_for_duplicate_commands(self, args):
        """Check for multiple commands (@Command.values) in the arguments."""
        valid_commands = {cmd.value for cmd in Command}
        found_command = ''

        for arg in args:
            if arg in valid_commands:
                if found_command != '':
                    print(f"Error: Cannot use more than one command at the same time: '{arg}' - '{found_command}'")
                    return True
                found_command = arg
        return False

