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
import sys

import shm

class Parser:
    
    def __init__(self): 
        parser = argparse.ArgumentParser(
        description =  'FastDDS Tool',
        usage = ''' fastdds <command> [<args>]

        Available commands:

            shm     shared-memory commands
        ''')

        parser.add_argument('command', help = 'Command to run')
        args = parser.parse_args(sys.argv[1:2])

        if not hasattr(self, args.command):
            print('Invalid command')
        else:
            getattr(self, args.command)()

    def shm(self):
        shm.Parser(sys.argv[2:])
  
if __name__ == '__main__':

    Parser()