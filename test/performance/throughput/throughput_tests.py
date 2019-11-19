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
import os
import subprocess

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
        '-t',
        '--test_duration',
        help='The duration of the test [s]',
        required=False,
        default=5
    )
    parser.add_argument(
        '-r',
        '--recovery_time',
        help='Maximum sleep time between bursts [ms]',
        required=False,
        default=10
    )
    parser.add_argument(
        '-f',
        '--demands_file',
        help='Filename of the demands configuration file',
        required=False,
        default=None
    )
    parser.add_argument(
        '-s',
        '--security',
        action='store_true',
        help='Enables security (Defaults: disable)',
        required=False
    )
    # Parse arguments
    args = parser.parse_args()
    xml_file = args.xml_file
    test_duration = str(args.test_duration)
    recovery_time = str(args.recovery_time)

    # Demands files arguments
    demands_options = []
    if args.demands_file is not None:
        demands_options = [
            '--file',
            args.demands_file,
        ]

    # Security flag
    security = False
    if args.security:
        security = True

    # Environment variables
    executable = os.environ.get('THROUGHPUT_TEST_BIN')
    certs_path = os.environ.get('CERTS_PATH')

    security_options = []
    if security is True:
        if certs_path:
            security_options = ['--security=true', '--certs=' + certs_path]

    # Base of test command for publisher agent
    pub_command = [
        executable,
        'publisher',
        '--time',
        test_duration,
        '--recovery_time',
        recovery_time,
        '--xml',
        xml_file,
        '--export_csv',
        '--export_prefix',
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
            'measurements_{}_security'.format(
                xml_file.split('/')[-1].split('\\')[-1].split('.')[-2]
            )
        )
        pub_command += security_options
        sub_command += security_options
    else:
        pub_command.append(
            'measurements_{}'.format(
                xml_file.split('/')[-1].split('\\')[-1].split('.')[-2]
            )
        )

    pub_command += demands_options

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
