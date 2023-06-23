# Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

# This test has implemented due readmine task #18950. Best effort datawriter should not use
#  UDP transport. Two tests has been introduced to test this functionality: this one would
#  check that the best effort datawriter does not use UDP.
import sys, os, subprocess, glob

script_dir = os.path.dirname(os.path.realpath(__file__))

publisher_command = os.environ.get("SIMPLE_COMMUNICATION_PUBLISHER_BIN")
if not publisher_command:
    publisher_files = glob.glob(os.path.join(script_dir, "**/SimpleCommunicationPublisher*"), recursive=True)
    pf = iter(publisher_files)
    publisher_command = next(pf, None)
    while publisher_command and (not os.path.isfile(publisher_command) or not os.access(publisher_command,
        os.X_OK)):
        publisher_command = next(pf, None)
assert publisher_command
subscriber_command = os.environ.get("SIMPLE_COMMUNICATION_SUBSCRIBER_BIN")
if not subscriber_command:
    subscriber_files = glob.glob(os.path.join(script_dir, "**/SimpleCommunicationSubscriber*"), recursive=True)
    pf = iter(subscriber_files)
    subscriber_command = next(pf, None)
    while subscriber_command and (not os.path.isfile(subscriber_command) or not os.access(subscriber_command,
        os.X_OK)):
        subscriber_command = next(pf, None)
assert subscriber_command

real_xml_file_pub = None
real_xml_file_sub = None
xml_file = os.environ.get("XML_FILE")
if xml_file:
    real_xml_file = os.path.join(script_dir, xml_file)
else:
    real_xml_file = os.path.join(script_dir, "simple_besteffort_shm_profile.xml")


subscriber_proc = subprocess.Popen([subscriber_command, "--seed", str(os.getpid()), "--xmlfile", real_xml_file],
                                   stdout=subprocess.PIPE)
publisher_proc = subprocess.Popen([publisher_command, "--seed", str(os.getpid()), "--xmlfile", real_xml_file])

subscriber_proc.communicate()
retvalue = subscriber_proc.returncode
if retvalue == 0:
    retvalue = 1  # Expected to fail

publisher_proc.kill()

sys.exit(retvalue)
