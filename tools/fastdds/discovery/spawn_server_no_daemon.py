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

sys.path.insert(0, str(Path('/home/carlos/fastdds_ws/install/fastdds/tools/fastdds', 'tools/fastdds/discovery')))
sys.path.insert(0, str(Path('/home/carlos/fastdds_ws/install/fastdds/tools/fastdds', 'tools/fastdds')))

from discovery.fastdds_daemon.node.daemon_node import (
    spawn_daemon,
    )

def __start_daemon(self, tool_path: str):
    """ Start the daemon process. """
    if spawn_daemon(tool_path, timeout=10.0, debug=False):
        print('The Fast DDS daemon has been started.')
    else:
        print('The Fast DDS daemon is already running.')

if __name__ == '__main__':
    __start_daemon()