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
        '-H',
        '--height',
        help='Height of the video',
        required=False,
        default=720
    )
    parser.add_argument(
        '-w',
        '--width',
        help='Width of the video',
        required=False,
        default=1024
    )
    parser.add_argument(
        '-r',
        '--frame_rate',
        help='Frame rate of the video [Hz]',
        required=False,
        default=30
    )
    parser.add_argument(
        '-t',
        '--test_duration',
        help='The duration of the test [s]',
        required=False,
        default=2
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
    samples = str(args.number_of_samples)
    height = str(args.height)
    width = str(args.width)
    frame_rate = str(args.frame_rate)
    test_duration = str(args.test_duration)

    # Security flag
    security = False
    if args.security:
        security = True

    # Environment variables
    executable = os.environ.get('VIDEO_TEST_BIN')
    certs_path = os.environ.get('CERTS_PATH')

    # Security
    security_options = []
    if security is True:
        if certs_path:
            security_options = ['--security=true', '--certs=' + certs_path]

    # Domain
    domain = str(os.getpid() % 230)
    domain_options = ['--domain', domain]

    # Base of test command for publisher agent
    pub_command = [
        executable,
        'publisher',
        '--samples',
        samples,
        '--testtime',
        test_duration,
        '--xml',
        xml_file,
        '--width',
        width,
        '--height',
        height,
        '--rate',
        frame_rate
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
        pub_command += security_options
        sub_command += security_options

    pub_command += domain_options
    sub_command += domain_options


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
