# Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

import sys, os, subprocess, glob

script_dir = os.path.dirname(os.path.realpath(__file__))

publisher_command = os.environ.get("DDS_SIMPLE_COMMUNICATION_PUBLISHER_BIN")
if not publisher_command:
    publisher_files = glob.glob(os.path.join(script_dir, "**/DDSCommunicationPublisher*"), recursive=True)
    publisher_command = next(iter(publisher_files), None)
assert publisher_command
subscriber_command = os.environ.get("DDS_SIMPLE_COMMUNICATION_SUBSCRIBER_BIN")
if not subscriber_command:
    subscriber_files = glob.glob(os.path.join(script_dir, "**/DDSCommunicationSubscriber*"), recursive=True)
    subscriber_command = next(iter(subscriber_files), None)
assert subscriber_command
xml_file = os.environ.get("XML_FILE")
if xml_file:
    real_xml_file = os.path.join(script_dir, xml_file)
else:
    real_xml_file = os.path.join(script_dir, "liveliness_assertion.360_profile.xml")

subscriber_proc = subprocess.Popen([subscriber_command, "--seed", str(os.getpid()), "--notexit",
    "--xmlfile", real_xml_file], stdout=subprocess.PIPE)
publisher_proc = subprocess.Popen([publisher_command, "--seed", str(os.getpid()), "--exit_on_lost_liveliness",
    "--xmlfile", real_xml_file])

while True:
    line = subscriber_proc.stdout.readline()
    if line.strip().decode('utf-8') == 'Subscriber recovered liveliness':
        print("Subscriber recovered liveliness")
        break

publisher_proc.kill()
subscriber_proc.communicate()
retvalue = subscriber_proc.returncode

if retvalue != 0:
    print("Test failed: " + str(retvalue))
else:
    print("Test successed")

sys.exit(retvalue)
