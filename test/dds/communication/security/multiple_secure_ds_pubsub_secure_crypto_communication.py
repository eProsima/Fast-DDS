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
"""Script to test the secure communication with encrypted RTPS messages
over multiple secure discovery servers."""

import argparse
import os
import subprocess
import sys
import time

class ParseOptions():
    """Parse arguments."""

    def __init__(self):
        """Object constructor."""
        self.args = self.__parse_args()

    def __parse_args(self):
        """
        Parse the input arguments.

        :return: A dictionary containing the arguments parsed.
        """
        parser = argparse.ArgumentParser(
            formatter_class=argparse.ArgumentDefaultsHelpFormatter,
            add_help=True,
            description=(
                'Script to test the secure communication with encrypted RTPS'
                'messages.'),
        )
        parser.add_argument(
            '-p',
            '--pub',
            type=str,
            required=True,
            help='Path to the Publisher executable.'
        )
        parser.add_argument(
            '-s',
            '--sub',
            type=str,
            required=True,
            help='Path to the Subscriber executable.'
        )
        parser.add_argument(
            '-P',
            '--xml-pub',
            type=str,
            help='Path to the publisher xml configuration file.'
        )
        parser.add_argument(
            '-S',
            '--xml-sub',
            type=str,
            help='Path to the subscriber xml configuration file.'
        )
        parser.add_argument(
            '-w',
            '--wait',
            type=int,
            help='Number of participants being discovered by the publisher to start publishing'
        )
        parser.add_argument(
            '-a',
            '--samples',
            type=int,
            help='Number of samples sent by the publisher.'
        )
        parser.add_argument(
            '-ds',
            '--servers',
            required=True,
            nargs='*',
            type=str,
            help='Path to the discovery server executable.'
        )
        parser.add_argument(
            '-DS',
            '--xml-servers',
            required=True,
            nargs='*',
            type=str,
            help='Path to the xml configuration file containing discovery server.'
        )
        parser.add_argument(
            '-nc',
            '--n-clients',
            type=int,
            help='Number of pubsub clients to launch (1 of each).'
        )
        parser.add_argument(
            '-rc',
            '--relaunch-clients',
            action='store_true',
            help='Whether to kill clients and relaunch them.'
        )
        parser.add_argument(
            '-vm',
            '--validation-method',
            type=str,
            help='Validation method to use [server, subscriber].'
        )

        return parser.parse_args()

def launch_discovery_server_processes(servers, xml_servers):

    ds_procs = []

    for i in range(0, len(servers)):
        server_cmd = []

        if not os.path.isfile(servers[i]):
            print(f'Discovery server executable file does not exists: {servers[i]}')
            sys.exit(1)

        if not os.access(servers[i], os.X_OK):
            print(
                'Discovery server executable does not have execution permissions:'
                f'{servers[i]}')
            sys.exit(1)

        # Call tool with the SERVER option
        server_cmd.extend([servers[i], '42'])
        server_cmd.extend(['--xml-file', xml_servers[i]])
        server_cmd.extend(['--server-id', str(i)])

        ds_proc = subprocess.Popen(server_cmd)
        print(
            'Running Discovery Server - commmand:  ',
            ' '.join(map(str, server_cmd)))

        ds_procs.append(ds_proc)

    return ds_procs

def launch_client_processes(n_clients, pub_command, sub_command):

    pub_procs = []
    sub_procs = []

    for i in range(0, n_clients):
        sub_proc = subprocess.Popen(sub_command)
        print(
                f'Running Subscriber - commmand:  ',
                ' '.join(map(str, sub_command)))

        pub_proc = subprocess.Popen(pub_command)
        print(
            'Running Publisher - commmand:  ',
            ' '.join(map(str, pub_command)))

        sub_procs.append(sub_proc)
        pub_procs.append(pub_proc)

    return sub_procs, pub_procs

def cleanup(pub_procs, sub_procs, ds_procs):

    [sub_proc.kill() for sub_proc in sub_procs]
    [pub_proc.kill() for pub_proc in pub_procs]
    [ds_proc.kill() for ds_proc in ds_procs]

def cleanup_clients(pub_procs, sub_procs):

    [sub_proc.kill() for sub_proc in sub_procs]
    [pub_proc.kill() for pub_proc in pub_procs]

def terminate(ok = True):
    if ok:
        try:
            sys.exit(os.EX_OK)
        except AttributeError:
            sys.exit(0)
    else:
        try:
            sys.exit(os.EX_SOFTWARE)
        except AttributeError:
            sys.exit(1)

def run(args):
    """
    Run the publisher, subscriber and secure discovery_servers.

    :param args: The input parameters.

    :return: The return code resulting from the publisher, subscriber
        and discovery servers execution. It is the number of failed processes.
    """
    pub_command = []
    sub_command = []
    n_clients = 1
    relaunch_clients = False

    script_dir = os.path.dirname(os.path.realpath(__file__))

    if not os.path.isfile(args.pub):
        print(f'Publisher executable file does not exists: {args.pub}')
        sys.exit(1)

    if not os.access(args.pub, os.X_OK):
        print(
            'Publisher executable does not have execution permissions:'
            f'{args.pub}')

    pub_command.append(args.pub)

    if not os.path.isfile(args.sub):
        print(f'Subscriber executable file does not exists: {args.sub}')
        sys.exit(1)

    if not os.access(args.sub, os.X_OK):
        print(
            'Subscriber executable does not have execution permissions:'
            f'{args.sub}')
        sys.exit(1)

    sub_command.append(args.sub)

    if args.xml_pub and args.xml_sub:
        if args.xml_pub:
            xml_file_pub = os.path.join(script_dir, args.xml_pub)
        if args.xml_sub:
            xml_file_sub = os.path.join(script_dir, args.xml_sub)
    else:
        print('Not provided xml configuration files.')
        sys.exit(1)

    pub_command.extend(['--xmlfile', xml_file_pub])
    sub_command.extend(['--xmlfile', xml_file_sub])

    pub_command.extend(['--seed', str(os.getpid())])
    sub_command.extend(['--seed', str(os.getpid())])

    if args.wait:
        pub_command.extend(['--wait', str(args.wait)])

    if args.samples:
        pub_command.extend(['--samples', str(args.samples)])
        sub_command.extend(['--samples', str(args.samples)])

    if args.n_clients:
        n_clients = int(args.n_clients)

    if args.relaunch_clients:
        relaunch_clients = True

    if len(args.servers) != len(args.xml_servers):
        print(
            'Number of servers arguments should be equal to the number of xmls provided.')
        sys.exit(1)

    ds_procs = launch_discovery_server_processes(args.servers, args.xml_servers)

    sub_procs, pub_procs = launch_client_processes(n_clients, pub_command, sub_command)

    terminate_ok = True

    if relaunch_clients:
        time.sleep(3)

        cleanup_clients(pub_procs, sub_procs)
        sub_procs, pub_procs = launch_client_processes(n_clients, pub_command, sub_command)

        time.sleep(3)

    if args.validation_method == 'server':
        # Check If discovery servers are still running
        for ds_proc in ds_procs:
            retcode = ds_proc.poll()
            if retcode is not None and retcode is not 0:
                print('Discovery Server process dead, terminating...')
                terminate_ok = False
    else:
        try:
            for sub_proc in sub_procs:
                outs, errs = sub_proc.communicate(timeout=15)
        except subprocess.TimeoutExpired:
            print('Subscriber process timed out, terminating...')
            terminate_ok = False

    cleanup(pub_procs, sub_procs, ds_procs)
    terminate(terminate_ok)

if __name__ == '__main__':

    # Parse arguments
    args = ParseOptions()

    run(args.args)