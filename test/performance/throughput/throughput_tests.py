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
        help='The duration of test iteration [s]',
        required=False,
        default='1'
    )
    parser.add_argument(
        '-r',
        '--recoveries_file',
        help='A CSV file with maxima sleep time between bursts [ms]',
        required=False,
        default=None
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
    recoveries_file = args.recoveries_file

    if security and not interprocess:
        print('Intra-process delivery NOT supported with security')
        exit(1)  # Exit with error

    # Check that test_duration is positive
    if str.isdigit(args.test_duration) and int(args.test_duration) > 0:
        test_duration = str(args.test_duration)
    else:
        print(
            '"test_duration" must be a positive integer, NOT {}'.format(
                args.test_duration
            )
        )
        exit(1)  # Exit with error

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

    # Recoveries files options
    recoveries_options = []
    if args.recoveries_file:
        if not os.path.isfile(args.recoveries_file):
            print(
                'Recoveries file "{}" is NOT a file'.format(
                    args.recoveries_file
                )
            )
            exit(1)  # Exit with error
        else:
            recoveries_options = [
                '--recoveries_file',
                args.recoveries_file,
            ]

    # Environment variables
    executable = os.environ.get('THROUGHPUT_TEST_BIN')
    certs_path = os.environ.get('CERTS_PATH')

    # Check that executable exists
    if executable:
        if not os.path.isfile(executable):
            print('THROUGHPUT_TEST_BIN does NOT specify a file')
            exit(1)  # Exit with error
    else:
        print('THROUGHPUT_TEST_BIN is NOT set')
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
            '--time',
            test_duration,
            '--export_csv',
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

        pub_command += demands_options
        pub_command += recoveries_options
        pub_command += domain_options
        pub_command += xml_options
        sub_command += domain_options
        sub_command += xml_options

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
            '--time',
            test_duration,
            '--export_csv',
        ]

        # Manage security
        if security is True:
            command.append(
                './measurements_intraprocess_{}_security.csv'.format(
                    reliability,
                )
            )
            command += security_options
        else:
            command.append(
                './measurements_intraprocess_{}.csv'.format(
                    reliability,
                )
            )

        command += demands_options
        command += recoveries_options
        command += domain_options
        command += xml_options

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