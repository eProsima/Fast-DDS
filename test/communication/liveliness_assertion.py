# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

import sys, os, subprocess

publisher_command = os.environ.get("SIMPLE_COMMUNICATION_PUBLISHER_BIN")
assert publisher_command
subscriber_command = os.environ.get("SIMPLE_COMMUNICATION_SUBSCRIBER_BIN")
assert subscriber_command

subscriber_proc = subprocess.Popen([subscriber_command, "--seed", str(os.getpid()), "--notexit"])
publisher_proc = subprocess.Popen([publisher_command, "--seed", str(os.getpid()), "--exit_on_lost_liveliness"], stdout=subprocess.PIPE)

while True:
    line = publisher_proc.stdout.readline()
    if line.strip().decode('utf-8') == 'Subscriber matched':
        break

subscriber_proc.kill()
publisher_proc.communicate()
retvalue = publisher_proc.returncode

sys.exit(retvalue)
