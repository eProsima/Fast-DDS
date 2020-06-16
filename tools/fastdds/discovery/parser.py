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
import subprocess
import re

class Parser:
    """Discovery server tool parser."""

    __tool_exe_file='fast-discovery-server'

    __help_message='''fastdds discovery <discovery-args>\n\n'''

    def __init__(self, argv):
        """Parse the sub-command and dispatch to the appropiate handler.

        Shows usage if no sub-command is specified.
        """

        try:
            result = subprocess.run(
                [self.__tool_exe_file],
                stdout=subprocess.PIPE,
                universal_newlines=True
            )

            if (len(argv) == 0 or 
                (len(argv) == 1 and argv[0] == '-h') or
                (len(argv) == 1 and argv[0] == '--help')):

                print(self.edit_tool_help(result.stdout))

            else:

                # Call the tool
                result = subprocess.run(
                    [self.__tool_exe_file] + argv
                )

        except BaseException as e:
            self.__help_message+=str(e)
            self.__help_message+=" fast-discovery-server tool not found!"
            print(self.__help_message)
    
    def edit_tool_help(self, usage_text):
        """Find and replace the tool-name by fasdds discovery."""
        m = re.search("Usage: ([a-zA-Z0-9-\.]*)\s", usage_text)
        if m:
            tool_name = m.group(1)
            return re.sub(tool_name, 'fastdds discovery', usage_text)

        return usage_text
