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

import sys, os, subprocess, glob

script_dir = os.path.dirname(os.path.realpath(__file__))

publisher_command = os.environ.get("SIMPLE_COMMUNICATION_PUBLISHER_BIN")
if not publisher_command:
    publisher_files = glob.glob(os.path.join(script_dir, "**/CommunicationPublisher*"), recursive=True)
    pf = iter(publisher_files)
    publisher_command = next(pf, None)
    while publisher_command and (not os.path.isfile(publisher_command) or not os.access(publisher_command,
        os.X_OK)):
        publisher_command = next(pf, None)
assert publisher_command
subscriber_command = os.environ.get("SIMPLE_COMMUNICATION_SUBSCRIBER_BIN")
if not subscriber_command:
    subscriber_files = glob.glob(os.path.join(script_dir, "**/CommunicationSubscriber*"), recursive=True)
    pf = iter(subscriber_files)
    subscriber_command = next(pf, None)
    while subscriber_command and (not os.path.isfile(subscriber_command) or not os.access(subscriber_command,
        os.X_OK)):
        subscriber_command = next(pf, None)
assert subscriber_command

extra_pub_arg = os.environ.get("EXTRA_PUB_ARG")
if extra_pub_arg:
    extra_pub_args = extra_pub_arg.split()
else:
    extra_pub_args = []
extra_pub_arg = os.environ.get("EXTRA_PUB_ARG")

extra_sub_arg = os.environ.get("EXTRA_SUB_ARG")
if extra_sub_arg:
    extra_sub_args = extra_sub_arg.split()
else:
    extra_sub_args = []
extra_sub_arg = os.environ.get("EXTRA_SUB_ARG")

real_xml_file_pub = None
real_xml_file_sub = None
xml_file = os.environ.get("XML_FILE")
if xml_file:
    real_xml_file_pub = os.path.join(script_dir, xml_file)
    real_xml_file_sub = os.path.join(script_dir, xml_file)
else:
    xml_file_pub = os.environ.get("XML_FILE_PUB")
    if xml_file_pub:
        real_xml_file_pub = os.path.join(script_dir, xml_file_pub)
    xml_file_sub = os.environ.get("XML_FILE_SUB")
    if xml_file_sub:
        real_xml_file_sub = os.path.join(script_dir, xml_file_sub)


subscriber1_proc = subprocess.Popen([subscriber_command, "--seed", str(os.getpid())]
        + (["--xmlfile", real_xml_file_sub] if real_xml_file_sub else [])
        + extra_sub_args)
publisher_proc = subprocess.Popen([publisher_command, "--seed", str(os.getpid())]
        + (["--xmlfile", real_xml_file_pub] if real_xml_file_pub else [])
        + extra_pub_args)
subscriber2_proc = subprocess.Popen([subscriber_command, "--seed", str(os.getpid())]
        + (["--xmlfile", real_xml_file_sub] if real_xml_file_sub else [])
        + extra_sub_args)

subscriber1_proc.communicate()
retvalue = subscriber1_proc.returncode
subscriber2_proc.communicate()
if retvalue == 0:
    retvalue = subscriber2_proc.returncode

publisher_proc.kill()

sys.exit(retvalue)
