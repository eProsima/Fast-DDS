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
import os
import subprocess
import sys


def cmd_setup_posix(install_path, prev_dir=False):
    """Check setup.bash existance for POSIX os."""
    return (f'bash -c ". \\"{install_path}/../setup.bash\\""'
            if prev_dir else f'bash -c ". \\"{install_path}/setup.bash\\""')


def cmd_posix(install_path, args=''):
    """Test script for POSIX os."""
    return 'bash -c ". \\"{}/setup.bash\\" && fastdds {}"'.format(
        install_path,
        args
    )


def cmd_setup_windows(install_path, prev_dir=False):
    """Check setup.bash existance for windows os."""
    return (f'"{install_path}\\..\\setup.bat"'
            if prev_dir else f'"{install_path}\\setup.bat"')


def cmd_windows(install_path, args=''):
    """Test script for Windows os."""
    return '"{}\\setup.bat" && fastdds {}'.format(
        install_path,
        args)


def test_fastdds_installed(install_path):
    """Test that fastdds is installed and run."""
    if os.name == 'posix':
        cmd = cmd_posix(install_path)
    elif os.name == 'nt':
        cmd = cmd_windows(install_path)
    else:
        print('OS not supported')
        sys.exit(1)

    ret = subprocess.call(cmd, shell=True)
    if 0 != ret:
        print('test_fastdds_installed FAILED')
        sys.exit(ret)


def test_fastdds_shm(install_path):
    """Test that shm command run."""
    if os.name == 'posix':
        cmd = cmd_posix(install_path, 'shm clean')
    elif os.name == 'nt':
        cmd = cmd_windows(install_path, 'shm clean')
    else:
        print('OS not supported')
        sys.exit(1)

    ret = subprocess.call(cmd, shell=True)
    if 0 != ret:
        print('test_fastdds_shm FAILED')
        sys.exit(ret)


def test_fastdds_discovery(install_path):
    """Test that discovery-server tool is installed."""
    if os.name == 'posix':
        cmd = cmd_posix(install_path, 'discovery')
    elif os.name == 'nt':
        cmd = cmd_windows(install_path, 'discovery')
    else:
        print('OS not supported')
        sys.exit(1)

    ret = subprocess.call(cmd, shell=True)
    if 0 != ret:
        print('test_fastdds_discovery FAILED')
        sys.exit(ret)


def adjust_install_path(install_path):
    """Adjust the install path when --merge-install has been used."""
    if os.name == 'posix':
        cmd_setup = cmd_setup_posix
    elif os.name == 'nt':
        cmd_setup = cmd_setup_windows
    else:
        print('OS not supported')

    if 0 != subprocess.call(cmd_setup(install_path), shell=True):
        # Check ../
        if 0 == subprocess.call(cmd_setup(install_path, True), shell=True):
            adjusted_install_path = install_path + '/../'
        else:
            print('setup.bash not found')
            sys.exit(1)
    # The path is valid
    else:
        adjusted_install_path = install_path

    return adjusted_install_path


if __name__ == '__main__':

    parser = argparse.ArgumentParser(
            usage='test.py <install_path>',
        )

    parser.add_argument('install_path',
                        help='FastDDS install path')

    args = parser.parse_args()

    path = adjust_install_path(args.install_path)

    test_fastdds_installed(path)
    test_fastdds_discovery(path)
    test_fastdds_shm(path)
