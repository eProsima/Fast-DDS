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
import argparse
import os
import platform
import re
import subprocess
import sys
import signal

from discovery.fastdds_daemon.node.daemon_node import (
    spawn_daemon,
    shutdown_daemon,
    is_daemon_running
    )

from discovery.fastdds_daemon.xmlrpc_local import local_client as client_cli

DOMAIN_ENV_VAR = "ROS_DOMAIN_ID"
EASY_MODE_ENV_VAR = "ROS2_EASY_MODE"

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

int_to_command = {
    0 : Command.AUTO,
    1 : Command.START,
    2 : Command.STOP,
    3 : Command.ADD,
    4 : Command.SET,
    5 : Command.LIST,
    6 : Command.INFO,
    42 : Command.SERVER
}

def get_sig_idx(sig) -> int:
    if sig == signal.SIGINT:
        return 0
    elif sig == signal.SIGTERM:
        return 1
    elif sig == signal.SIGKILL:
        return 2
    else:
        return -1


def parse_daemon_arguments(argv):
    parser = argparse.ArgumentParser(description="CLI for the Fast DDS Daemon.", add_help=False)
    parser.add_argument('-d', '--domain', type=int, help="Domain ID.")
    parser.add_argument('command', nargs='?', choices=[command.value for command in Command], help="Command to execute.")
    args, unknow_args = parser.parse_known_args(argv)
    return args, unknow_args


def parse_aux_arguments(argv):
    parser = argparse.ArgumentParser(description="CLI for the Fast DDS Daemon.", add_help=False)
    parser.add_argument('-h', '--help', action='store_true', help="Help command.")
    parser.add_argument('-e', '--examples', action='store_true', help="Examples command.")
    parser.add_argument('-v', '--version', action='store_true', help="Show Fast DDS version.")
    args, _ = parser.parse_known_args(argv)
    return args


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
                raise SystemExit(result_help.returncode)

            aux_argument = False
            # Check for help and/or examples arguments
            aux_args = parse_aux_arguments(argv)
            if aux_args.help:
                print(self.__edit_tool_text(result_help.stdout, result_help.stdout))
                aux_argument = True
            if aux_args.examples:
                result_ex = subprocess.run(
                    [tool_path, str(command_to_int[Command.SERVER]), '-e'],
                    stdout=subprocess.PIPE,
                    universal_newlines=True
                )
                print(self.__edit_tool_text(result_ex.stdout, result_help.stdout))
                aux_argument = True
            # Check for version argument
            if aux_args.version:
                result_ver = subprocess.run(
                    [tool_path, str(command_to_int[Command.SERVER]), '-v'],
                    stdout=subprocess.PIPE,
                    universal_newlines=True
                )
                print(result_ver.stdout)
                aux_argument = True
            # Allow calling -h, -e and -v in the same command
            if aux_argument:
                raise SystemExit(0)

            if not argv or (argv and argv[0] not in [command.value for command in Command]):
                # Conventional server creation
                command_int = command_to_int[Command.SERVER]
                args_for_cpp = [str(command_int)] + sys.argv[2:]
                result = subprocess.run([tool_path] + args_for_cpp)
                raise SystemExit(result.returncode)
            else:
                # Easy Mode
                if os.name == 'nt':
                    # Easy Mode not supported in Windows
                    print('Easy Mode not yet supported in Windows')
                    raise SystemExit(1)
                daemon_args, unknown_args = parse_daemon_arguments(argv)
                try:
                    command_int = command_to_int[Command(daemon_args.command)]
                except:
                    print('Unknown command')
                    raise SystemExit(1)

            # Daemon commands
            domain = 0
            if daemon_args.domain is None:
                if command_int != command_to_int[Command.STOP] and \
                        command_int != command_to_int[Command.LIST]:
                    print(f"Error: Domain ID not specified. Use -d <domain_id> to specify the domain.")
                    raise SystemExit(1)
            else:
                domain = daemon_args.domain
            args_for_cpp = [str(command_int), '-d', str(domain)]
            for unknown_arg in unknown_args:
                args_for_cpp.append(unknown_arg)

            # Use the 'stop' command without domain to stop all servers and shutdown the daemon
            if command_int == command_to_int[Command.STOP] and daemon_args.domain is None:
                if not self.__is_daemon_running():
                    print('The Fast DDS daemon is not running.')
                    raise SystemExit(0)

                print(client_cli.stop_all_request(get_sig_idx(signal.SIGTERM)))
                self.__stop_daemon()
                for p in Path(self.__shm_dir()).glob("*_servers.txt"):
                    p.unlink()

            elif command_int == command_to_int[Command.AUTO] or command_int == command_to_int[Command.START]:
                # The 'start' or 'auto' commands require an <IP>:<domain> argument
                self.__check_unknown_args(unknown_args, command_int)
                try:
                    ip, _ = unknown_args[0].split(':', 1)
                except ValueError:
                    print("Error: Invalid argument format. Expected <IP>:<domain>.")
                    raise SystemExit(1)

                easy_mode = self.__get_easy_mode_from_env()
                if easy_mode is not None:
                    # Check if the first IP address is the same as the easy mode
                    if ip != easy_mode:
                        print(f"Warning: Incompatibility detected between ROS2_EASY_MODE ({easy_mode}) \
                                and CLI argument ({ip}). CLI argument will be used.")

                self.__start_daemon(tool_path)
                output = client_cli.run_request_nb(domain, args_for_cpp, ip)
                self.__notify_output(output)

            elif command_int == command_to_int[Command.STOP]:
                if not self.__is_daemon_running():
                    print('The Fast DDS daemon is not running.')
                    raise SystemExit(0)
                # The 'stop' command does not support extra argument
                if unknown_args:
                    print(f"Unknown arguments: {unknown_args}")
                    raise SystemExit(1)
                print(client_cli.stop_request(domain, get_sig_idx(signal.SIGTERM)))

            elif command_int == command_to_int[Command.ADD]:
                # The 'add' commands require an <IP>:<domain> argument
                self.__check_unknown_args(unknown_args, command_int)
                if not self.__is_daemon_running():
                    print('The Fast DDS daemon is not running.')
                    raise SystemExit(0)
                print(client_cli.run_request_b(domain, args_for_cpp, True).strip())

            elif command_int == command_to_int[Command.SET]:
                # The 'set' commands require an <IP>:<domain> argument
                self.__check_unknown_args(unknown_args, command_int)
                if not self.__is_daemon_running():
                    print('The Fast DDS daemon is not running.')
                    raise SystemExit(0)
                print(client_cli.run_request_b(domain, args_for_cpp, True).strip())

            elif command_int == command_to_int[Command.LIST]:
                if not self.__is_daemon_running():
                    print('The Fast DDS daemon is not running. No servers to list.')
                    raise SystemExit(0)
                print(client_cli.run_request_b(domain, args_for_cpp, False))

            elif command_int == command_to_int[Command.INFO]:
                print('Info mode not implemented yet.')

            else:
                print('Fast DDS CLI Error: Unknown command')

        except KeyboardInterrupt:
            # It lets the subprocess to handle the exception
            pass

        except SystemExit as e:
            sys.exit(e.code)

        except BaseException as e:
            print(f'\n Fast DDS CLI Error: {type(e).__name__} - {e}')
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

    def __edit_tool_text(self, usage_text, raw_stdout):
        """Find and replace the tool-name by fastdds discovery."""
        m = re.search('Usage: ([a-zA-Z0-9-\\.]*)\\s', raw_stdout)
        if m:
            tool_name = m.group(1)
            return re.sub(tool_name, 'fastdds discovery', usage_text)
        return usage_text

    def __start_daemon(self, tool_path: str):
        """ Start the daemon process. """
        if spawn_daemon(tool_path, timeout=10.0, debug=False):
            print('The Fast DDS daemon has been started.')
        else:
            print('The Fast DDS daemon is already running.')

    def __stop_daemon(self):
        """ Stop the daemon process. """
        if shutdown_daemon(None, timeout=10.0):
            print('The Fast DDS daemon has been stopped.')
        else:
            print('The Fast DDS daemon is not running.')

    def __is_daemon_running(self):
        """ Checks if daemon is running. """
        if is_daemon_running():
            return True
        return False

    def __get_easy_mode_from_env(self) -> str:
        """
        Obtain the value present in 'ROS2_EASY_MODE' from the environment.
        """
        value = None
        value = os.getenv(EASY_MODE_ENV_VAR)
        return value

    def __shm_dir(self):
        """
        Calculate the shm directory.

        returns Path:
            The path to the platform specific the SHM directory

        """
        if os.name == 'posix':
            # MAC
            if platform.mac_ver()[0] != '':
                shm_path = Path('/private/tmp/boost_interprocess/').resolve()
            # Linux
            else:
                shm_path = Path('/dev/shm/').resolve()
        else:
            raise RuntimeError(f'{os.name} not supported')

        return shm_path

    def __notify_output(self, output):
        """
        Notify the output of the command to the user.
        """
        if output:
            print(output.strip())

            if 'Error starting Server' in output:
                raise SystemExit(1)  # Exit with error code
            elif 'Error: DS for Domain' in output:
                # Different error code to notify Fast DDS RTPSDomain method.
                raise SystemExit(2)
        else:
            print("No output received.")

    def __check_unknown_args(self, unknown_args, command_int):
        # The 'start' or 'auto' commands require an <IP>:<domain> argument
        if len(unknown_args) != 1:
            print(f"Error: Command '{int_to_command[command_int].value}' requires a single <IP>:<domain> argument.")
            raise SystemExit(1)
