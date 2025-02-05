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
Sub-Command Clean implementation.

This sub-command finds and remove unused shared-memory files.

"""

import os
import platform
import re
from pathlib import Path

if os.name == 'posix':
    import fcntl
elif os.name == 'nt':
    import win32con
    import win32file
    import pywintypes
    import msvcrt


class Clean:
    """This command searches and deletes zombie SHM ports / segments."""

    def run(self, force: bool):
        """Execute the clean."""
        self.__ports_in_use = 0
        self.__segments_in_use = 0

        zombie_segments = self.__clean_zombie_segments()
        zombie_ports = self.__clean_zombie_ports()
        if force:
            zombie_datasharing_segments = self.__clean_zombie_datasharing_segments()
            print(f'Datasharing segments: {zombie_datasharing_segments}')

        print('shm.clean:')
        print(self.__ports_in_use, 'ports in use')
        print(self.__segments_in_use, 'segments in use')
        # each port has 3 files
        print(int(len(zombie_ports) / 3), 'zombie ports cleaned')
        # each segment has 2 files
        print(int(len(zombie_segments) / 2), 'zombie segments cleaned')
        if force:
            print(int(len(zombie_datasharing_segments)), 'datasharing segments cleaned')

    def __shm_dir(self):
        """
        Calculate the shm directory.

        returns Path:
            The path to the platform specific the SHM directory

        """
        # Windows
        if os.name == 'nt':
            shm_path = Path('c:\\programdata\\eprosima\\'
                            'fastdds_interprocess\\').resolve()
        elif os.name == 'posix':
            # MAC
            if platform.mac_ver()[0] != '':
                shm_path = Path('/private/tmp/boost_interprocess/').resolve()
            # Linux
            else:
                shm_path = Path('/dev/shm/').resolve()
        else:
            raise RuntimeError(f'{os.name} not supported')

        return shm_path

    def __list_dir(self):
        """Return a list of files in the default SHM dir."""
        try:
            return os.listdir(self.__shm_dir())
        except FileNotFoundError:
            return []

    def __clean_zombie_segments(self):
        """
        Find & delete zombie segments in the default SHM dir.

        returns list(str):
            The deleted file names

        """
        segment_lock_re = re.compile('^fastdds_(\\d|[a-z]){16}(_el|_sl)')

        # Each segment has an "_el" lock file that is locked if the segment
        # is open and the owner process is alive
        segment_locks = [
            str(self.__shm_dir() / file_name) for file_name in
            self.__list_dir() if segment_lock_re.match(file_name)]
        zombie_files = []

        # Check is_file_locked for each lock file
        for f in segment_locks:
            if not self.__is_file_locked(f):
                # Not locked so delete lock file & segment file
                segment_file = f
                lock_file = f[:-3]
                self.__remove_file(segment_file)
                self.__remove_file(lock_file)
                f = [segment_file, lock_file]
                zombie_files += f
            else:
                self.__segments_in_use += 1

        return zombie_files

    def __clean_zombie_ports(self):
        """
        Find & delete zombie ports in the default SHM dir.

        returns list(str):
            the deleted file names

        """
        port_lock_re = re.compile('^fastdds_port\\d{,5}(_el|_sl)')
        # Each port has an "_el | _sl" lock file that is locked if the port
        # is open and the owner process is alive
        port_locks = [
            file_name for file_name in self.__list_dir() if port_lock_re.match(
                file_name)]
        zombie_files = []

        # Check is_file_locked for each lock file
        for file in port_locks:
            if not self.__is_file_locked(self.__shm_dir() / file):
                # Not locked so delete lock file, segment file and
                # port_mutex_file
                port_lock_file = self.__shm_dir() / file
                port_segment_file = self.__shm_dir() / file[:-3]
                port_mutex_file = self.__shm_dir() / self.__port_mutex_name(
                    file[:-3])
                self.__remove_file(port_segment_file)
                self.__remove_file(port_lock_file)
                self.__remove_file(port_mutex_file)
                zombie_files += [
                    port_lock_file, port_segment_file, port_mutex_file]
            else:
                self.__ports_in_use += 1

        return [self.__shm_dir() / file_name for file_name in zombie_files]

    def __port_mutex_name(self, port_file_name):
        """
        Return the mutex object filename for the given port.

        param port_file_name str:
            port segment file_name, the mutex name is deduced from it.

        returns str:
            mutex file name

        """
        if os.name == 'posix':
            return ''.join(['sem.', port_file_name, '_mutex'])
        else:
            return ''.join([port_file_name, '_mutex'])

    def __clean_zombie_datasharing_segments(self):
        """
        Find & delete datasharing segments in the default SHM dir.

        returns list(str):
            the deleted file names

        """
        segment_lock_re = re.compile(r'fast_datasharing_([0-9a-f\.]){35}_([\d\.]{7})')
        segments_locks = [
            file_name for file_name in self.__list_dir() if segment_lock_re.match(
                file_name)]
        zombie_files = []

        # Check is_file_locked for each lock file
        for file in segments_locks:
            file_name = self.__shm_dir() / file
            self.__remove_file(file_name)
            zombie_files.append(file)

        return [file_name for file_name in zombie_files]

    def __remove_file(self, file):
        """
        Delete a file.

        Always return void, even if the function fails.

        param file str:
            The complete file_path

        """
        try:
            os.remove(file)
        except OSError:
            pass

    def __is_file_locked(self, file):
        """Return whether a file is locked or not.

        :param file: str with the complete file_path
        :return: bool

        """
        try:
            with open(file, 'ab') as fd:
                if os.name == 'posix':
                    # Lock file in Exclusive mode. Fail if the file is locked.
                    fcntl.flock(fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
                elif os.name == 'nt':
                    # Get WIN32 handle to the file
                    h_file = win32file._get_osfhandle(fd.fileno())
                    # Lock file in Exclusive mode. Fail if the file is locked.
                    mode = win32con.LOCKFILE_EXCLUSIVE_LOCK | msvcrt.LK_NBLCK
                    overlapped = pywintypes.OVERLAPPED()
                    win32file.LockFileEx(h_file, mode, 0, -0x10000, overlapped)
            return False
        except OSError:
            return True
