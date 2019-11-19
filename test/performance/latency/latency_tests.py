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

import argparse
import subprocess
import os
from os import listdir

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        '-x',
        '--xml_file',
        help='A Fast-RTPS XML configuration file',
        required=False
    )
    parser.add_argument(
        '-n',
        '--number_of_samples',
        help='The number of measurements to take for each payload',
        required=False,
        default=10000
    )
    parser.add_argument(
        '-s',
        '--security',
        action='store_true',
        help='Enables security (Defaults: disable)',
        required=False
    )
    parser.add_argument(
        '-i',
        '--interprocess',
        action='store_true',
        help='Publisher and subscribers in separate processes. Defaults:False',
        required=False,
    )
    # Parse arguments
    args = parser.parse_args()
    xml_file = args.xml_file
    samples = str(args.number_of_samples)

    # Security flag
    security = False
    if args.security:
        security = True

    # Inter-process flag
    interprocess = False
    if args.interprocess:
        interprocess = True

    # Environment variables
    executable = os.environ.get('LATENCY_TEST_BIN')
    certs_path = os.environ.get('CERTS_PATH')

    security_options = []
    if security is True:
        if certs_path:
            security_options = ['--security=true', '--certs=' + certs_path]

    if interprocess is True:
        # Base of test command for publisher agent
        pub_command = [
            executable,
            'publisher',
            '--samples',
            samples,
            '--xml',
            xml_file,
            '--export_raw_data',
        ]
        # Base of test command for subscriber agent
        sub_command = [
            executable,
            'subscriber',
            '--xml',
            xml_file,
        ]

        # Manage security
        if security is True:
            pub_command.append(
                './measurements_{}_security.xml'.format(
                    xml_file.split('/')[-1].split('\\')[-1].split('.')[-2]
                )
            )
            pub_command += security_options
            sub_command += security_options
        else:
            pub_command.append(
                './measurements_{}.xml'.format(
                    xml_file.split('/')[-1].split('\\')[-1].split('.')[-2]
                )
            )

        print('Publisher command: {}'.format(
            ' '.join(element for element in pub_command)),
            flush=True
        )
        print('Subscriber command: {}'.format(
            ' '.join(element for element in sub_command)),
            flush=True
        )

        # Spawn processes
        subscriber = subprocess.Popen(pub_command)
        publisher = subprocess.Popen(sub_command)
        # Wait until finish
        subscriber.communicate()
        publisher.communicate()

        if subscriber.returncode != 0:
            exit(subscriber.returncode)
        elif publisher.returncode != 0:
            exit(publisher.returncode)
    else:
        # Base of test command to execute
        command = [
            executable,
            'both',
            '--samples',
            samples,
            '--xml',
            xml_file,
            '--export_raw_data',
        ]

        # Manage security
        if security is True:
            command.append(
                './measurements_{}_security.xml'.format(
                    xml_file.split('/')[-1].split('\\')[-1].split('.')[-2]
                )
            )
            command += security_options
        else:
            command.append(
                './measurements_{}.xml'.format(
                    xml_file.split('/')[-1].split('\\')[-1].split('.')[-2]
                )
            )

        # command_string = ' '.join(element for element in command)
        print('Executable command: {}'.format(
            ' '.join(element for element in command)),
            flush=True
        )

        # Spawn process
        both = subprocess.Popen(command)
        # Wait until finish
        both.communicate()
        exit(both.returncode)
