# Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

import subprocess
import os
import pytest
import random

isolated_delivery_test_cases = []

# IPv6 nor SHM/Data-Saring are supported on windows docker hosts
# https://docs.docker.com/engine/daemon/ipv6/
if os.name == 'nt':
    isolated_delivery_test_cases = [
        # Default builtin transports (UDP forced)
        ('--mechanism default', '-s 20 --mechanism default', '-s 20 --mechanism default', '--mechanism default', 50),
        # following test flaky in windows
        #('--mechanism default', '-s 20 --mechanism default', '-s 20 --mechanism default', '--ignore-local-endpoints --mechanism default', 50),
        # Large-data with only one publisher (testing TCP)
        ('--mechanism large-data', '--unknown-argument', '--mechanism large-data', '--unknown-argument', 10),
        # TCP takes longer to match, so explicitly expect much more samples in this case.
        # TCP is configured through initial peers (a single locator), so we test only one publisher at a time
        #  Note: pubsub with TCP and ignore-local-endpoints NOT set is not supported, tested in  expected output test cases
        ('-s 100 --mechanism tcpv4 -a 113.1.1.3', '-s 100 --mechanism tcpv4 -a 113.1.1.3', '-s 100 --mechanism tcpv4 -a 113.1.1.3', '--unknown-command', 200),
        # UDP
        ('--mechanism udpv4', '-s 20 --mechanism udpv4', '-s 20 --mechanism udpv4', '--mechanism udpv4', 50),
        # following test flaky in windows
        #('--mechanism udpv4', '-s 20 --mechanism udpv4', '-s 20 --mechanism udpv4', '--mechanism udpv4 --ignore-local-endpoints', 50),
    ]
else:
    isolated_delivery_test_cases = [
        # Default builtin transports (UDP forced)
        ('--mechanism default', '-s 20 --mechanism default', '-s 20 --mechanism default', '--mechanism default', 50),
        ('--mechanism default', '-s 20 --mechanism default', '-s 20 --mechanism default', '--ignore-local-endpoints --mechanism default', 50),
        # Large-data with only one publisher (testing TCP)
        ('--mechanism large-data', '--unknown-argument', '--mechanism large-data', '--unknown-argument', 10),
        # TCP takes longer to match, so explicitly expect much more samples in this case.
        # TCP is configured through initial peers (a single locator), so we test only one publisher at a time
        #  Note: pubsub with TCP and ignore-local-endpoints NOT set is not supported, tested in  expected output test cases
        ('-s 100 --mechanism tcpv4 -a 113.1.1.3', '-s 100 --mechanism tcpv4 -a 113.1.1.3', '-s 100 --mechanism tcpv4 -a 113.1.1.3', '--unknown-command', 200),
        # UDP
        ('--mechanism udpv4', '-s 20 --mechanism udpv4', '-s 20 --mechanism udpv4', '--mechanism udpv4', 50),
        ('--mechanism udpv4', '-s 20 --mechanism udpv4', '-s 20 --mechanism udpv4', '--mechanism udpv4 --ignore-local-endpoints', 50),
    ]

@pytest.mark.parametrize("pub_args, sub_args, isub_args, pubsub_args, repetitions", isolated_delivery_test_cases)
def test_delivery_mechanisms_isolated(pub_args, sub_args, isub_args, pubsub_args, repetitions):
    """."""
    ret = False
    out = ''

    menv = dict(os.environ)

    menv["PUB_ARGS"] =  pub_args
    menv["SUB_ARGS"] =  sub_args
    menv["ISUB_ARGS"] =  isub_args
    menv["PUBSUB_ARGS"] =  pubsub_args

    # Set a random suffix to the cotainer name to avoid conflicts with other containers
    menv["CONTAINER_SUFFIX_COMPOSE"] = str(random.randint(0,100))

    try:
        out = subprocess.check_output('"@DOCKER_EXECUTABLE@" container prune -f && "@DOCKER_EXECUTABLE@" compose -f delivery_mechanisms_isolated.compose.yml up',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=40,
            env=menv
        ).decode().split('\n')

        sent = 0
        received = 0
        for line in out:
            if 'SENT' in line:
                sent += 1

            if 'RECEIVED' in line:
                received += 1

        if sent != 0 and received != 0 and repetitions == received:
            ret = True
        else:
            print('ERROR: sent: ' + str(sent) + ', but received: ' + str(received) +
                  ' (expected: ' + str(repetitions) + ')')
            raise subprocess.CalledProcessError(1, '')

    except subprocess.CalledProcessError:
        for l in out:
            print(l)
    except subprocess.TimeoutExpired:
        print('TIMEOUT')
        print(out)

    assert(ret)

# Python's check_output() in windows does not kill the shell
# process on timeout, skip these tests on windowds
if os.name != 'nt':
    isolated_timeout_test_cases = [
        # Shared memory and data-sharing isolated subscriber timeout test cases
        ('--mechanism shm', '--unknown-command', '--mechanism shm', '--mechanism shm -i'),
        ('--mechanism data-sharing', '--unknown-command', '--mechanism data-sharing', '--mechanism data-sharing -i'),
        # Incompatible mechanisms timeout test cases
        ('--mechanism tcpv4', '--mechanism udpv4', '--mechanism udpv6', '--mechanism tcpv6 -i'),
        ('--mechanism shm', '--mechanism data-sharing', '--mechanism large-data', '--mechanism intra-process -i')
    ]

    @pytest.mark.parametrize("pub_args, sub_args, isub_args, pubsub_args", isolated_timeout_test_cases)
    def test_delivery_mechanisms_isolated_timeout(pub_args, sub_args, isub_args, pubsub_args):
        """."""
        ret = False
        out = ''

        menv = dict(os.environ)

        menv["PUB_ARGS"] =  pub_args
        menv["SUB_ARGS"] =  sub_args
        menv["ISUB_ARGS"] =  isub_args
        menv["PUBSUB_ARGS"] =  pubsub_args

        try:
            out = subprocess.check_output('"@DOCKER_EXECUTABLE@" compose -f delivery_mechanisms_isolated.compose.yml up',
                stderr=subprocess.STDOUT,
                shell=True,
                timeout=10,
                env=menv
            )
        except subprocess.CalledProcessError as e:
            print (e.output)
        except subprocess.TimeoutExpired:
            ret = True
            subprocess.check_output('"@DOCKER_EXECUTABLE@" compose -f delivery_mechanisms_isolated.compose.yml down',
                stderr=subprocess.STDOUT,
                shell=True,
                timeout=15,
                env=menv
            )

        assert(ret)

# Unsupported delivery mechanisms corner case test
isolated_expected_output_test_cases = [
    ('--unknown-argument', '--unknown-argument', '--mechanism tcp', 'Unsupported', 1)
]

@pytest.mark.parametrize("pub_args, sub_args, pubsub_args, expected_message, n_messages", isolated_expected_output_test_cases)
def test_delivery_mechanisms_isolated_expected_output(pub_args, sub_args, pubsub_args, expected_message, n_messages):
    """."""
    ret = False
    out = ''
    render_out = ''

    menv = dict(os.environ)

    menv["PUB_ARGS"] =  pub_args
    menv["SUB_ARGS"] =  sub_args
    menv["ISUB_ARGS"] =  sub_args
    menv["PUBSUB_ARGS"] =  pubsub_args

    try:
        out = subprocess.check_output('"@DOCKER_EXECUTABLE@" compose -f delivery_mechanisms_isolated.compose.yml up',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=20,
            env=menv
        )
        render_out = out.decode().split('\n')

        count = 0
        for line in render_out:
            if expected_message in line:
                count += 1

        if count >= int(n_messages):
            ret = True
        else:
            print ('ERROR: expected at least: ' + n_messages +' "' + expected_message + '" messages, but received ' + str(count))
            raise subprocess.CalledProcessError(1, render_out)

    except subprocess.CalledProcessError as e:
        print (render_out)
    except subprocess.TimeoutExpired:
        print('TIMEOUT')
        print(out)

    assert(ret)
