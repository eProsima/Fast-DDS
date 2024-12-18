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
Tests for the fastdds tool.

Contains a package of system test for fastdds tool

usage: test.py <install_path> <test_name>

install_path: Fast DDS base path installation

test_name: Test to run.

Available tests:

    test_fastdds_installed
    test_fastdds_version
    test_fastdds_discovery
    test_fastdds_discovery_run
    test_fastdds_shm
    test_fastdds_xml_validate
    test_ros_discovery

"""

import argparse
import os
import subprocess
import signal
import sys
from pathlib import Path
try: # Try catch for new python dependency
    import psutil
except ImportError:
    print(
        'psutil module not found. '
        'Try to install running "pip install psutil"')
    sys.exit(1)


def setup_script_name():
    """
    Test script for POSIX os.

    Return the name of the setup script file. This setup script is
    required by some tests to prepare the test environment

    """
    if os.name == 'posix':
        script_name = 'setup.bash'
    elif os.name == 'nt':
        script_name = 'setup.bat'
    else:
        print(f'{os.name} not supported')
        sys.exit(1)

    return script_name


def cmd(install_path, setup_script_path=Path(), args=''):
    """
    Build the command line to run for the current platform.

    install_path Path:
        Fast DDS instalation path.

    setup_script_path Path:
        Path to the setup script if necessary.

    args str:
        Extra argumens for the command (sub-verb)

    """
    tool_path = str(install_path.resolve())
    if os.name == 'posix':
        if str(setup_script_path) == '.':
            cmd = f'bash -c "\\"{tool_path}\\" {args}"'
        else:
            cmd = (f'bash -c ". \\"{setup_script_path}\\" && \\"{tool_path}\\"'
                   f' {args}"')
    elif os.name == 'nt':
        if str(setup_script_path) == '.':
            cmd = f'"{tool_path}" {args}'
        else:
            cmd = f'"{setup_script_path}" && "{tool_path}" {args}'
    else:
        print(f'{os.name} not supported')
        sys.exit(1)
    return cmd


def test_fastdds_installed(install_path):
    """Test that fastdds is installed and runs."""
    ret = subprocess.call(cmd(install_path), shell=True)
    if 0 != ret:
        print('test_fastdds_installed FAILED')
        sys.exit(ret)


def test_fastdds_shm(install_path):
    """Test that shm command runs."""
    args = ' shm clean'
    ret = subprocess.call(cmd(
        install_path=install_path, args=args), shell=True)
    if 0 != ret:
        print('test_fastdds_shm FAILED')
        sys.exit(ret)


def test_fastdds_discovery(install_path, setup_script_path):
    """Test that discovery command runs."""
    args = ' discovery -h'
    ret = subprocess.call(
        cmd(install_path=install_path,
            setup_script_path=setup_script_path,
            args=args),
        shell=True)

    if 0 != ret:
        print('test_fastdds_discovery FAILED')
        sys.exit(ret)


def test_fastdds_discovery_run(install_path, setup_script_path):
    """Test that discovery command runs."""
    args = ' discovery -l 127.0.0.1'
    try:
        test_timeout = 10
        process = subprocess.Popen(
            cmd(install_path=install_path,
                setup_script_path=setup_script_path,
                args=args),
            shell=True)
        ret = process.wait(timeout=test_timeout)
        # Manually set the error return code because we need the process to timeout
        ret = 3
    except subprocess.TimeoutExpired:
        print(f'Timeout {test_timeout} expired. Test successful')
        try:
            # Need to kill all child processes to properly end the test 
            parent = psutil.Process(process.pid)
            for child in parent.children(recursive=True):
                child.terminate()
            parent.terminate()
            ret = 0
        except Exception as e:
            print(f"Error while ending child processes: {e}")

    if 0 != ret:
        print('test_fastdds_discovery_run FAILED')
        sys.exit(ret)


def test_ros_discovery(install_path, setup_script_path):
    """Test that discovery command runs."""
    args = ' -h'
    ret = subprocess.call(
        cmd(install_path=install_path,
            setup_script_path=setup_script_path,
            args=args),
        shell=True)

    if 0 != ret:
        print('test_fastdds_discovery FAILED')
        sys.exit(ret)


def test_fastdds_xml_validate(install_path):
    """Test that xml validate command runs."""
    args = ' xml validate'
    ret = subprocess.call(cmd(
        install_path=install_path, args=args), shell=True)
    if 0 != ret:
        print('test_fastdds_xml_validate FAILED')
        sys.exit(ret)


def get_paths(install_path):
    """
    Adjust the install path when --merge-install has been used.

    param install_path Path:
        Path to the Fast DDS installation path

    return Path:
        Adjusted path to the installation path where fastdds tool
        is installed

    """
    tool_install_path = install_path / 'bin'

    if not os.path.exists(tool_install_path.resolve()):
        tool_install_path = tool_install_path / '..' / 'bin'
        if not os.path.exists(tool_install_path.resolve()):
            print(f'{tool_install_path} NOT FOUND')
            sys.exit(1)

    setup_script_path = install_path / setup_script_name()
    if not os.path.exists(str(setup_script_path.resolve())):
        setup_script_path = install_path / '..' / setup_script_name()
        if not os.path.exists(str(setup_script_path.resolve())):
            print('setup_script NOT FOUND')
            sys.exit(1)

    return setup_script_path, tool_install_path


if __name__ == '__main__':

    parser = argparse.ArgumentParser(
            usage='test.py <install_path> <test_name>',
        )

    parser.add_argument('install_path',
                        help='FastDDS install path')

    parser.add_argument('test_name',
                        help='Test to run')

    args = parser.parse_args()

    setup_script_path, tool_path = get_paths(Path(args.install_path))

    fastdds_tool_path = tool_path / 'fastdds'
    ros_disc_tool_path = tool_path / 'ros-discovery'

    # Tests dictionary
    tests = {
        'test_fastdds_installed':
        lambda: test_fastdds_installed(fastdds_tool_path),
        'test_fastdds_discovery': lambda: test_fastdds_discovery(
            fastdds_tool_path, setup_script_path),
        'test_fastdds_discovery_run': lambda: test_fastdds_discovery_run(
            fastdds_tool_path, setup_script_path),
        'test_ros_discovery':
        lambda: test_ros_discovery(ros_disc_tool_path, setup_script_path),
        'test_fastdds_shm': lambda: test_fastdds_shm(fastdds_tool_path),
        'test_fastdds_xml_validate':
        lambda: test_fastdds_xml_validate(fastdds_tool_path)
    }

    tests[args.test_name]()
