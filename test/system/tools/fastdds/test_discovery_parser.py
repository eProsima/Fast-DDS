# Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

import unittest
from unittest.mock import patch, MagicMock
import sys
import os
from pathlib import Path

# Add the path to the parser module and the cpp tool to sys.path
sys.path.insert(0, str(Path(os.getenv('TOOL_PATH'), 'tools/fastdds/discovery')))
sys.path.insert(0, str(Path(os.getenv('TOOL_PATH'), 'tools/fastdds')))
sys.path.insert(0, str(Path(os.getenv('TOOL_PATH'), 'bin')))
# Import the Parser class from the parser module
from parser import Parser, command_to_int, Command


class TestDiscoveryParser(unittest.TestCase):
    def __init__(self, methodName = "test_fastdds_daemon"):
        super().__init__(methodName)
        # Attribute to check the command sent to the RPC server
        self.check_command = ''
        # Attribute to check the domain sent to the RPC server to act as index
        self.domain = 0
        # Attribute to check the value of the third attr (easy_mode or check_server) sent to the RPC server
        self.third_attr = ''

    def side_effect_rpc(self, *args, **kwargs):
        domain = args[0]
        used_cmd = args[1]
        third_attr = args[2]
        assert(domain == self.domain)
        assert(len(used_cmd) == len(self.check_command))
        assert(third_attr == self.third_attr)
        for i in range(len(self.check_command)):
            assert(used_cmd[i] == self.check_command[i])
        return 'Mocked request'

    def set_env_values(self, env_var_name, value):
        os.environ[env_var_name] = value

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.shutdown_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_all_request')
    def test_parser_shutdown_when_on(self, mock_rpc_stopall, mock_rpc_nbrequest, mock_rpc_brequest, mock_shutdown, mock_is_running, mock_exit):
        mock_rpc_stopall.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_shutdown.return_value = True
        mock_is_running.return_value = True

        argv = ['stop']
        parser = Parser(argv)

        mock_is_running.assert_called_once()
        mock_shutdown.assert_called_once()
        mock_rpc_stopall.assert_called_once()
        mock_rpc_nbrequest.assert_not_called()
        mock_rpc_brequest.assert_not_called()
        mock_exit.assert_not_called()

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.shutdown_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_all_request')
    def test_parser_shutdown_when_off(self, mock_rpc_stopall, mock_rpc_nbrequest, mock_rpc_brequest, mock_shutdown, mock_is_running, mock_exit):
        mock_rpc_stopall.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_shutdown.return_value = False
        mock_is_running.return_value = False

        argv = ['stop']
        try:
            parser = Parser(argv)
        except SystemExit as e:
            # Expecting to fail with exit code 0
            self.assertEqual(e.code, 0)

        mock_is_running.assert_called_once()
        mock_shutdown.assert_not_called()
        mock_rpc_stopall.assert_not_called()
        mock_rpc_nbrequest.assert_not_called()
        mock_rpc_brequest.assert_not_called()
        mock_exit.assert_called_once()

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.spawn_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_all_request')
    def test_parser_auto(self, mock_rpc_stopall, mock_rpc_nbrequest, mock_rpc_brequest, mock_spawn, mock_is_running, mock_exit):
        mock_rpc_stopall.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_spawn.return_value = True

        self.check_command = [str(command_to_int[Command.AUTO]), '-d', '0']
        mock_rpc_nbrequest.side_effect = self.side_effect_rpc

        argv = ['auto']
        parser = Parser(argv)

        mock_is_running.assert_not_called()
        mock_spawn.assert_called_once()
        mock_rpc_stopall.assert_not_called()
        mock_rpc_nbrequest.assert_called_once()
        mock_rpc_brequest.assert_not_called()
        mock_exit.assert_not_called()

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.spawn_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_all_request')
    def test_parser_auto_domain_arg(self, mock_rpc_stopall, mock_rpc_nbrequest, mock_rpc_brequest, mock_spawn, mock_is_running, mock_exit):
        mock_rpc_stopall.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_spawn.return_value = True

        self.domain = 7
        self.check_command = [str(command_to_int[Command.AUTO]), '-d', '7']
        mock_rpc_nbrequest.side_effect = self.side_effect_rpc

        argv = ['auto', '-d', '7']
        parser = Parser(argv)
        print('End of test')

        mock_is_running.assert_not_called()
        mock_spawn.assert_called_once()
        mock_rpc_stopall.assert_not_called()
        mock_rpc_nbrequest.assert_called_once()
        mock_rpc_brequest.assert_not_called()
        mock_exit.assert_not_called()

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.spawn_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_all_request')
    def test_parser_auto_domain_env(self, mock_rpc_stopall, mock_rpc_nbrequest, mock_rpc_brequest, mock_spawn, mock_is_running, mock_exit):
        mock_rpc_stopall.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_spawn.return_value = True

        self.set_env_values('ROS_DOMAIN_ID', '42')
        self.domain = 42
        self.check_command = [str(command_to_int[Command.AUTO]), '-d', '42']
        mock_rpc_nbrequest.side_effect = self.side_effect_rpc

        argv = ['auto']
        parser = Parser(argv)

        mock_is_running.assert_not_called()
        mock_spawn.assert_called_once()
        mock_rpc_stopall.assert_not_called()
        mock_rpc_nbrequest.assert_called_once()
        mock_rpc_brequest.assert_not_called()
        mock_exit.assert_not_called()

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.spawn_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_all_request')
    def test_parser_auto_easy_mode_domain_env(self, mock_rpc_stopall, mock_rpc_nbrequest, mock_rpc_brequest, mock_spawn, mock_is_running, mock_exit):
        mock_rpc_stopall.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_spawn.return_value = True

        self.set_env_values('ROS2_EASY_MODE', '127.0.0.1')
        self.domain = 42
        self.third_attr = '127.0.0.1'
        self.check_command = [str(command_to_int[Command.AUTO]), '-d', '42', '127.0.0.1:42']
        mock_rpc_nbrequest.side_effect = self.side_effect_rpc

        # The parser is not responsible of adding the ROS2_EASY_MODE argument to the command. This is done in Fast DDS.
        # The parser only checks if the ROS2_EASY_MODE variable is set to pass it to the RPC server as third argument
        argv = ['auto', '-d', '42', '127.0.0.1:42']
        parser = Parser(argv)

        mock_is_running.assert_not_called()
        mock_spawn.assert_called_once()
        mock_rpc_stopall.assert_not_called()
        mock_rpc_nbrequest.assert_called_once()
        mock_rpc_brequest.assert_not_called()
        mock_exit.assert_not_called()

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.spawn_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_all_request')
    def test_parser_auto_ros_static_peers(self, mock_rpc_stopall, mock_rpc_nbrequest, mock_rpc_brequest, mock_spawn, mock_is_running, mock_exit):
        mock_rpc_stopall.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_spawn.return_value = True

        self.set_env_values('ROS_STATIC_PEERS', '127.0.0.1:1')
        self.check_command = [str(command_to_int[Command.AUTO]), '-d', '0', '127.0.0.1:1']
        mock_rpc_nbrequest.side_effect = self.side_effect_rpc

        argv = ['auto']
        parser = Parser(argv)

        mock_is_running.assert_not_called()
        mock_spawn.assert_called_once()
        mock_rpc_stopall.assert_not_called()
        mock_rpc_nbrequest.assert_called_once()
        mock_rpc_brequest.assert_not_called()
        mock_exit.assert_not_called()

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.spawn_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_all_request')
    def test_parser_start(self, mock_rpc_stopall, mock_rpc_nbrequest, mock_rpc_brequest, mock_spawn, mock_is_running, mock_exit):
        mock_rpc_stopall.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_spawn.return_value = True

        self.check_command = [str(command_to_int[Command.START]), '-d', '0']
        mock_rpc_nbrequest.side_effect = self.side_effect_rpc

        argv = ['start']
        parser = Parser(argv)

        mock_is_running.assert_not_called()
        mock_spawn.assert_called_once()
        mock_rpc_stopall.assert_not_called()
        mock_rpc_nbrequest.assert_called_once()
        mock_rpc_brequest.assert_not_called()
        mock_exit.assert_not_called()

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.spawn_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_all_request')
    def test_parser_start_ros_static_peers(self, mock_rpc_stopall, mock_rpc_nbrequest, mock_rpc_brequest, mock_spawn, mock_is_running, mock_exit):
        mock_rpc_stopall.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_spawn.return_value = True

        self.set_env_values('ROS_STATIC_PEERS', '127.0.0.1:1;127.0.0.1:2')
        self.check_command = [str(command_to_int[Command.START]), '-d', '0', '127.0.0.1:1;127.0.0.1:2']
        mock_rpc_nbrequest.side_effect = self.side_effect_rpc

        argv = ['start']
        parser = Parser(argv)

        mock_is_running.assert_not_called()
        mock_spawn.assert_called_once()
        mock_rpc_stopall.assert_not_called()
        mock_rpc_nbrequest.assert_called_once()
        mock_rpc_brequest.assert_not_called()
        mock_exit.assert_not_called()

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.spawn_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_all_request')
    def test_parser_start_with_arg(self, mock_rpc_stopall, mock_rpc_nbrequest, mock_rpc_brequest, mock_spawn, mock_is_running, mock_exit):
        mock_rpc_stopall.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_spawn.return_value = True

        self.check_command = [str(command_to_int[Command.START]), '-d', '0', '127.0.0.1:4;127.0.0.1:2']
        mock_rpc_nbrequest.side_effect = self.side_effect_rpc

        argv = ['start', '127.0.0.1:4;127.0.0.1:2']
        parser = Parser(argv)

        mock_is_running.assert_not_called()
        mock_spawn.assert_called_once()
        mock_rpc_stopall.assert_not_called()
        mock_rpc_nbrequest.assert_called_once()
        mock_rpc_brequest.assert_not_called()
        mock_exit.assert_not_called()

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.shutdown_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_request')
    def test_parser_stop_when_off(self, mock_rpc_stop_once, mock_rpc_nbrequest, mock_rpc_brequest, mock_shutdown, mock_is_running, mock_exit):
        mock_rpc_stop_once.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_is_running.return_value = False
        mock_shutdown.return_value = True

        self.check_command = [str(command_to_int[Command.STOP]), '-d', '0']
        mock_rpc_brequest.side_effect = self.side_effect_rpc

        argv = ['stop', '-d', '0']
        try:
            parser = Parser(argv)
        except SystemExit as e:
            # Expecting to fail with exit code 0
            self.assertEqual(e.code, 0)

        mock_is_running.assert_called_once()
        mock_shutdown.assert_not_called()
        mock_rpc_stop_once.assert_not_called()
        mock_rpc_nbrequest.assert_not_called()
        mock_rpc_brequest.assert_not_called()
        mock_exit.assert_called_once()

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.shutdown_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_request')
    def test_parser_stop_when_on(self, mock_rpc_stop_once, mock_rpc_nbrequest, mock_rpc_brequest, mock_shutdown, mock_is_running, mock_exit):
        mock_rpc_stop_once.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_is_running.return_value = True
        mock_shutdown.return_value = True

        self.check_command = [str(command_to_int[Command.STOP]), '-d', '0']
        mock_rpc_brequest.side_effect = self.side_effect_rpc

        argv = ['stop', '-d', '0']
        parser = Parser(argv)

        mock_is_running.assert_called_once()
        mock_shutdown.assert_not_called()
        mock_rpc_stop_once.assert_called_once()
        mock_rpc_nbrequest.assert_not_called()
        mock_rpc_brequest.assert_not_called()
        mock_exit.assert_not_called()

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.shutdown_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_request')
    def test_parser_stop_whith_unknown_args(self, mock_rpc_stop_once, mock_rpc_nbrequest, mock_rpc_brequest, mock_shutdown, mock_is_running, mock_exit):
        mock_rpc_stop_once.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_is_running.return_value = True
        mock_shutdown.return_value = True

        self.check_command = [str(command_to_int[Command.STOP]), '-d', '0']
        mock_rpc_brequest.side_effect = self.side_effect_rpc

        argv = ['stop', '-d', '0', 'extra_arg']
        parser = Parser(argv)

        mock_is_running.assert_called_once()
        mock_shutdown.assert_not_called()
        mock_rpc_stop_once.assert_not_called()
        mock_rpc_nbrequest.assert_not_called()
        mock_rpc_brequest.assert_not_called()
        mock_exit.assert_called_once()

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.spawn_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_all_request')
    def test_parser_list_when_off(self, mock_rpc_stopall, mock_rpc_nbrequest, mock_rpc_brequest, mock_spawn, mock_is_running, mock_exit):
        mock_rpc_stopall.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_is_running.return_value = False
        mock_spawn.return_value = True

        self.third_attr = False
        self.check_command = [str(command_to_int[Command.LIST]), '-d', '0']
        mock_rpc_brequest.side_effect = self.side_effect_rpc

        argv = ['list']
        try:
            parser = Parser(argv)
        except SystemExit as e:
            # Expecting to fail with exit code 0
            self.assertEqual(e.code, 0)

        mock_is_running.assert_called_once()
        mock_spawn.assert_not_called()
        mock_rpc_stopall.assert_not_called()
        mock_rpc_nbrequest.assert_not_called()
        mock_rpc_brequest.assert_not_called()
        mock_exit.assert_called_once()

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.spawn_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_all_request')
    def test_parser_list_when_on(self, mock_rpc_stopall, mock_rpc_nbrequest, mock_rpc_brequest, mock_spawn, mock_is_running, mock_exit):
        mock_rpc_stopall.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_is_running.return_value = True
        mock_spawn.return_value = True

        self.third_attr = False
        self.check_command = [str(command_to_int[Command.LIST]), '-d', '0']
        mock_rpc_brequest.side_effect = self.side_effect_rpc

        argv = ['list']
        parser = Parser(argv)

        mock_is_running.assert_called_once()
        mock_spawn.assert_not_called()
        mock_rpc_stopall.assert_not_called()
        mock_rpc_nbrequest.assert_not_called()
        mock_rpc_brequest.assert_called_once()
        mock_exit.assert_not_called()

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.spawn_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_all_request')
    def test_parser_add_when_off(self, mock_rpc_stopall, mock_rpc_nbrequest, mock_rpc_brequest, mock_spawn, mock_is_running, mock_exit):
        mock_rpc_stopall.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_is_running.return_value = False
        mock_spawn.return_value = True

        self.third_attr = True
        self.check_command = [str(command_to_int[Command.ADD]), '-d', '0']
        mock_rpc_brequest.side_effect = self.side_effect_rpc

        argv = ['add']
        try:
            parser = Parser(argv)
        except SystemExit as e:
            # Expecting to fail with exit code 0
            self.assertEqual(e.code, 0)

        mock_is_running.assert_called_once()
        mock_spawn.assert_not_called()
        mock_rpc_stopall.assert_not_called()
        mock_rpc_nbrequest.assert_not_called()
        mock_rpc_brequest.assert_not_called()
        mock_exit.assert_called_once()

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.spawn_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_all_request')
    def test_parser_add_when_on(self, mock_rpc_stopall, mock_rpc_nbrequest, mock_rpc_brequest, mock_spawn, mock_is_running, mock_exit):
        mock_rpc_stopall.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_is_running.return_value = True
        mock_spawn.return_value = True

        self.third_attr = True
        self.check_command = [str(command_to_int[Command.ADD]), '-d', '0']
        mock_rpc_brequest.side_effect = self.side_effect_rpc

        argv = ['add']
        parser = Parser(argv)

        mock_is_running.assert_called_once()
        mock_spawn.assert_not_called()
        mock_rpc_stopall.assert_not_called()
        mock_rpc_nbrequest.assert_not_called()
        mock_rpc_brequest.assert_called_once()
        mock_exit.assert_not_called()

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.spawn_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_all_request')
    def test_parser_set_when_off(self, mock_rpc_stopall, mock_rpc_nbrequest, mock_rpc_brequest, mock_spawn, mock_is_running, mock_exit):
        mock_rpc_stopall.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_is_running.return_value = False
        mock_spawn.return_value = True

        self.third_attr = True
        self.check_command = [str(command_to_int[Command.SET]), '-d', '0']
        mock_rpc_brequest.side_effect = self.side_effect_rpc

        argv = ['set']
        try:
            parser = Parser(argv)
        except SystemExit as e:
            # Expecting to fail with exit code 0
            self.assertEqual(e.code, 0)

        mock_is_running.assert_called_once()
        mock_spawn.assert_not_called()
        mock_rpc_stopall.assert_not_called()
        mock_rpc_nbrequest.assert_not_called()
        mock_rpc_brequest.assert_not_called()
        mock_exit.assert_called_once()

    @patch('parser.sys.exit')
    @patch('parser.is_daemon_running')
    @patch('parser.spawn_daemon')
    @patch('parser.client_cli.run_request_b')
    @patch('parser.client_cli.run_request_nb')
    @patch('parser.client_cli.stop_all_request')
    def test_parser_set_when_on(self, mock_rpc_stopall, mock_rpc_nbrequest, mock_rpc_brequest, mock_spawn, mock_is_running, mock_exit):
        mock_rpc_stopall.return_value = 'Mocked request'
        mock_rpc_nbrequest.return_value = 'Mocked request'
        mock_rpc_brequest.return_value = 'Mocked request'
        mock_is_running.return_value = True
        mock_spawn.return_value = True

        self.third_attr = True
        self.check_command = [str(command_to_int[Command.SET]), '-d', '0']
        mock_rpc_brequest.side_effect = self.side_effect_rpc

        argv = ['set']
        parser = Parser(argv)

        mock_is_running.assert_called_once()
        mock_spawn.assert_not_called()
        mock_rpc_stopall.assert_not_called()
        mock_rpc_nbrequest.assert_not_called()
        mock_rpc_brequest.assert_called_once()
        mock_exit.assert_not_called()


if __name__ == '__main__':
    unittest.main()
