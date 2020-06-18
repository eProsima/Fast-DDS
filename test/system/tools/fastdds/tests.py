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

"""Test for the fastdds tool."""

import argparse
import os
import subprocess
import sys
from pathlib import Path


def setup_script_name():
    """Test script for POSIX os."""
    if os.name == 'posix':
        script_name = 'setup.bash'
    elif os.name == 'nt':
        script_name = 'setup.bat'
    else:
        print(f'{os.name} not supported')
        os.exit(1)

    return script_name


def cmd(install_path, setup_script_path=Path(), args=''):
    """Test script for POSIX os."""
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
        os.exit(1)
    return cmd


def test_fastdds_installed(install_path):
    """Test that fastdds is installed and run."""
    ret = subprocess.call(cmd(install_path), shell=True)
    if 0 != ret:
        print('test_fastdds_installed FAILED')
        sys.exit(ret)


def test_fastdds_shm(install_path):
    """Test that shm command run."""
    args = ' shm clean'
    ret = subprocess.call(cmd(
        install_path=install_path, args=args), shell=True)
    if 0 != ret:
        print('test_fastdds_shm FAILED')
        sys.exit(ret)


def test_fastdds_discovery(install_path, setup_script_path):
    """Test that discovery command run."""
    args = ' discovery'
    ret = subprocess.call(
        cmd(install_path=install_path,
            setup_script_path=setup_script_path,
            args=args),
        shell=True)

    if 0 != ret:
        print('test_fastdds_discovery FAILED')
        sys.exit(ret)


def get_paths(install_path):
    """Adjust the install path when --merge-install has been used."""
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
            print(f'setup_script NOT FOUND')
            sys.exit(1)

    return setup_script_path, tool_install_path / 'fastdds'


if __name__ == '__main__':

    parser = argparse.ArgumentParser(
            usage='test.py <install_path>',
        )

    parser.add_argument('install_path',
                        help='FastDDS install path')

    args = parser.parse_args()

    setup_script_path, tool_path = get_paths(Path(args.install_path))

    print(f'setup_script_path: {setup_script_path.resolve()}\n'
          f'tool_path: {tool_path.resolve()}\n')

    test_fastdds_installed(tool_path)
    test_fastdds_discovery(tool_path, setup_script_path)
    test_fastdds_shm(tool_path)
