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

if os.name == 'posix':
    import fcntl
elif os.name == 'nt':
    import win32con
    import win32file
    import pywintypes
    import winerror
    import msvcrt

import re

class Parser:

     def __init__(self, argv): 
        parser = argparse.ArgumentParser(
            description =  'Shared-memory commands',
            usage = ''' shm <sub-command> [<args>]

            shm sub-commands:

                clean     clean SHM zombie files

            ''')

        parser.add_argument('command', help = 'Command to run')
        args = parser.parse_args(argv)

        if args.command == 'clean':
            Clean().clean()
        else:
            print('shm: ' + args.command + ' sub-command is not valid')

class Clean:
    
    def shm_dir(self):
        if os.name == 'nt':
            return "c:\\programdata\\eprosima\\fastrtps_interprocess\\"
        else:
            return "/dev/shm/"

    def clean(self):
        self.ports_in_use = 0
        self.segments_in_use = 0

        zombie_segments = self.clean_zombie_segments()
        # for zs in zombie_segments:
        #     print(zs)

        zombie_ports = self.clean_zombie_ports()
        # for zp in zombie_ports:
        #     print(zp)

        print("shm.clean:")
        print(self.ports_in_use, "ports in use")
        print(self.segments_in_use, "segments in use")
        print(int(len(zombie_ports) / 3), "zombie ports cleaned.")
        print(int(len(zombie_segments) / 2), "zombie segments cleaned.")
        
    def clean_zombie_segments(self):
        segment_lock_re = re.compile('^fastrtps_(\d|[a-z]){16}_el|_sl')
        segment_locks = [self.shm_dir() + file_name 
            for file_name in self.list_dir() if segment_lock_re.match(file_name)]
        zombie_files = []
        for file in segment_locks:
            if not self.is_file_locked(file):
                segment_file = file
                lock_file = file[:-3] 
                self.remove_file(segment_file)
                self.remove_file(lock_file)
                f = [segment_file, lock_file]
                zombie_files += f
            else:
                self.segments_in_use += 1

        return zombie_files

    def list_dir(self):
        try:
            return os.listdir(self.shm_dir())
        except:
            return list()

    def clean_zombie_ports(self):
        port_lock_re = re.compile('^fastrtps_port\d{,5}_el|_sl')
        port_locks = [file_name 
            for file_name in self.list_dir() if port_lock_re.match(file_name)]
        zombie_files = []
        for file in port_locks:
            if not self.is_file_locked(self.shm_dir() + file):
                port_lock_file = self.shm_dir() + file
                port_segment_file = self.shm_dir() + file[:-3]
                port_mutex_file = self.shm_dir() + self.port_mutex_name(file[:-3])
                self.remove_file(port_segment_file)
                self.remove_file(port_lock_file)
                self.remove_file(port_mutex_file)
                zombie_files += [port_lock_file, port_segment_file, port_mutex_file]
            else:
                self.ports_in_use += 1

        return [self.shm_dir() + file_name for file_name in zombie_files]

    def port_mutex_name(self, file_name):
        if os.name == 'posix':
            return ''.join(['sem.', file_name,'_mutex'])
        else:
            return ''.join([file_name,'_mutex'])

    def remove_file(self, file):
        try:
            os.remove(file)
        except:
            pass

    def fastrtps_files(self):
        return [self.shm_dir() + file_name for file_name in os.listdir(self.shm_dir())]

    def is_file_locked(self, file):
        try:
            fd = open(file, "ab")
            if os.name == 'posix':
                fcntl.flock(fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
            elif os.name == 'nt':
                h_file = win32file._get_osfhandle(fd.fileno())
                mode = win32con.LOCKFILE_EXCLUSIVE_LOCK | msvcrt.LK_NBLCK 
                overlapped = pywintypes.OVERLAPPED()
                win32file.LockFileEx(h_file, mode, 0, -0x10000, overlapped)
            return False
        except :
            return True