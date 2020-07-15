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
        '-f',
        '--demands_file',
        help='Filename of the demands configuration file',
        required=False,
        default=None
    )
    parser.add_argument(
        '-n',
        '--number_of_samples',
        help='The number of measurements to take for each payload',
        required=False,
        default='10000'
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
    security = args.security
    interprocess = args.interprocess

    if security and not interprocess:
        print('Intra-process delivery NOT supported with security')
        exit(1)  # Exit with error

    # Check that samples is positive
    if str.isdigit(args.number_of_samples) and int(args.number_of_samples) > 0:
        samples = str(args.number_of_samples)
    else:
        print(
            '"number_of_samples" must be a positive integer, NOT {}'.format(
                args.number_of_samples
            )
        )
        exit(1)  # Exit with error

    # Demands files options
    demands_options = []
    if args.demands_file:
        if not os.path.isfile(args.demands_file):
            print('Demands file "{}" is NOT a file'.format(args.demands_file))
            exit(1)  # Exit with error
        else:
            demands_options = [
                '--file',
                args.demands_file,
            ]

    # XML options
    reliability = 'default'
    xml_options = []
    if xml_file:
        if not os.path.isfile(xml_file):
            print('XML file "{}" is NOT a file'.format(xml_file))
            exit(1)  # Exit with error
        else:
            xml_options = ['--xml', xml_file]
            # Get reliability from XML
            reliability = xml_file.split('/')[-1].split('\\')[-1]
            reliability = reliability.split('.')[-2].split('_')[1:]
            reliability = '_'.join(reliability)

    # Environment variables
    executable = os.environ.get('LATENCY_TEST_BIN')
    certs_path = os.environ.get('CERTS_PATH')

    # Check that executable exists
    if executable:
        if not os.path.isfile(executable):
            print('LATENCY_TEST_BIN does NOT specify a file')
            exit(1)  # Exit with error
    else:
        print('LATENCY_TEST_BIN is NOT set')
        exit(1)  # Exit with error

    # Security
    security_options = []
    if security is True:
        if certs_path:
            if os.path.isdir(certs_path):
                security_options = ['--security=true', '--certs=' + certs_path]
            else:
                print('CERTS_PATH does NOT specify a directory')
                exit(1)  # Exit with error
        else:
            print('Cannot find CERTS_PATH environment variable')
            exit(1)  # Exit with error

    # Domain
    domain = str(os.getpid() % 230)
    domain_options = ['--domain', domain]

    if interprocess is True:
        # Base of test command for publisher agent
        pub_command = [
            executable,
            'publisher',
            '--samples',
            samples,
            '--export_raw_data',
        ]
        # Base of test command for subscriber agent
        sub_command = [
            executable,
            'subscriber',
        ]

        # Manage security
        if security is True:
            pub_command.append(
                './measurements_interprocess_{}_security.csv'.format(
                    reliability
                )
            )
            pub_command += security_options
            sub_command += security_options
        else:
            pub_command.append(
                './measurements_interprocess_{}.csv'.format(
                    reliability
                )
            )

        pub_command += domain_options
        pub_command += xml_options
        pub_command += demands_options

        sub_command += domain_options
        sub_command += xml_options
        sub_command += demands_options

        print('Publisher command: {}'.format(
            ' '.join(element for element in pub_command)),
            flush=True
        )
        print('Subscriber command: {}'.format(
            ' '.join(element for element in sub_command)),
            flush=True
        )

        # Spawn processes
        publisher = subprocess.Popen(pub_command)
        subscriber = subprocess.Popen(sub_command)
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
            '--export_raw_data',
        ]

        # Manage security
        if security is True:
            command.append(
                './measurements_intraprocess_{}_security.csv'.format(
                    reliability
                )
            )
            command += security_options
        else:
            command.append(
                './measurements_intraprocess_{}.csv'.format(
                    reliability
                )
            )

        command += domain_options
        command += xml_options
        command += demands_options

        print('Executable command: {}'.format(
            ' '.join(element for element in command)),
            flush=True
        )

        # Spawn process
        both = subprocess.Popen(command)
        # Wait until finish
        both.communicate()
        exit(both.returncode)
    exit(0)
