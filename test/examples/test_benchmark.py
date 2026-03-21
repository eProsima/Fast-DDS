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

benchmark_test_cases = []

# IPv6 nor SHM/Data-Saring are supported on windows docker hosts
# https://docs.docker.com/engine/daemon/ipv6/
if os.name == 'nt':
    benchmark_test_cases = [
        ('--transport DEFAULT --msg-size NONE', '--transport DEFAULT --msg-size NONE'),
        ('--transport DEFAULT --msg-size SMALL', '--transport DEFAULT --msg-size SMALL'),
        ('--transport DEFAULT --msg-size MEDIUM', '--transport DEFAULT --msg-size MEDIUM'),
        # Following test case flaky en windows
        #('--transport DEFAULT --msg-size BIG', '--transport DEFAULT --msg-size BIG'),
        ('--transport DEFAULT --msg-size NONE', '--transport UDPv4 --msg-size NONE'),
        ('--transport DEFAULT --msg-size SMALL', '--transport UDPv4 --msg-size SMALL'),
        ('--transport DEFAULT --msg-size MEDIUM', '--transport UDPv4 --msg-size MEDIUM'),
        # Following test case flaky en windows
        #('--transport DEFAULT --msg-size BIG', '--transport UDPv4 --msg-size BIG'),
        ('--transport UDPv4 --msg-size NONE', '--transport UDPv4 --msg-size NONE'),
        ('--transport UDPv4 --msg-size SMALL', '--transport UDPv4 --msg-size SMALL'),
        ('--transport UDPv4 --msg-size MEDIUM', '--transport UDPv4 --msg-size MEDIUM'),
        # Following test case flaky en windows
        #('--transport UDPv4 --msg-size BIG', '--transport UDPv4 --msg-size BIG'),
        ('--transport LARGE_DATA --msg-size NONE', '--transport LARGE_DATA --msg-size NONE'),
        ('--transport LARGE_DATA --msg-size SMALL', '--transport LARGE_DATA --msg-size SMALL'),
        ('--transport LARGE_DATA --msg-size MEDIUM', '--transport LARGE_DATA --msg-size MEDIUM'),
        # Following test case flaky en windows
        #('--transport LARGE_DATA --msg-size BIG', '--transport LARGE_DATA --msg-size BIG'),
    ]
else:
    benchmark_test_cases = [
        ('--transport DEFAULT --msg-size NONE', '--transport DEFAULT --msg-size NONE'),
        ('--transport DEFAULT --msg-size SMALL', '--transport DEFAULT --msg-size SMALL'),
        ('--transport DEFAULT --msg-size MEDIUM', '--transport DEFAULT --msg-size MEDIUM'),
        ('--transport DEFAULT --msg-size BIG', '--transport DEFAULT --msg-size BIG'),
        ('--transport DEFAULT --msg-size NONE', '--transport UDPv4 --msg-size NONE'),
        ('--transport DEFAULT --msg-size SMALL', '--transport UDPv4 --msg-size SMALL'),
        ('--transport DEFAULT --msg-size MEDIUM', '--transport UDPv4 --msg-size MEDIUM'),
        ('--transport DEFAULT --msg-size BIG', '--transport UDPv4 --msg-size BIG'),
        ('--transport SHM --msg-size NONE', '--transport SHM --msg-size NONE'),
        ('--transport SHM --msg-size SMALL', '--transport SHM --msg-size SMALL'),
        ('--transport SHM --msg-size MEDIUM', '--transport SHM --msg-size MEDIUM'),
        ('--transport SHM --msg-size BIG', '--transport SHM --msg-size BIG'),
        ('--transport UDPv4 --msg-size NONE', '--transport UDPv4 --msg-size NONE'),
        ('--transport UDPv4 --msg-size SMALL', '--transport UDPv4 --msg-size SMALL'),
        ('--transport UDPv4 --msg-size MEDIUM', '--transport UDPv4 --msg-size MEDIUM'),
        ('--transport UDPv4 --msg-size BIG', '--transport UDPv4 --msg-size BIG'),
        ('--transport LARGE_DATA --msg-size NONE', '--transport LARGE_DATA --msg-size NONE'),
        ('--transport LARGE_DATA --msg-size SMALL', '--transport LARGE_DATA --msg-size SMALL'),
        ('--transport LARGE_DATA --msg-size MEDIUM', '--transport LARGE_DATA --msg-size MEDIUM'),
        ('--transport LARGE_DATA --msg-size BIG', '--transport LARGE_DATA --msg-size BIG'),
    ]

@pytest.mark.parametrize("pub_args, sub_args", benchmark_test_cases)
def test_benchmark(pub_args, sub_args):
    """."""
    ret = False
    out = ''

    pub_requirements = '--reliable --transient-local'
    sub_requirements = '--reliable --transient-local'

    menv = dict(os.environ)

    menv["PUB_ARGS"] =  pub_requirements + ' ' + pub_args
    menv["SUB_ARGS"] =  sub_requirements + ' ' + sub_args

    # Set a random suffix to the cotainer name to avoid conflicts with other containers
    menv["CONTAINER_SUFFIX_COMPOSE"] = str(random.randint(0,100))

    timeout = 10
    #In windows timeout argument does not wor properly
    #It is not able to kill the subprocess when it is surpassed
    #and when the subprocess exists then it is checked.
    #This test is about samples (1000) so for big msgs in windows
    #it can take longer. Set it to a higher value
    if os.name == 'nt':
        timeout = 200

    try:
        out = subprocess.check_output('"@DOCKER_EXECUTABLE@" compose -f benchmark.compose.yml up',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=timeout,
            env=menv
        ).decode().split('\n')

        sent = 0
        received = 0
        for line in out:
            if 'SENT' in line:
                sent += 1

            if 'RECEIVED' in line:
                received += 1

        if sent != 0 and received != 0 and sent == received:
            ret = True
        else:
            print ('ERROR: sent: ' + str(sent) + ', but received: ' + str(received) + ' (expected: ' + str(sent) + ')')
            raise subprocess.CalledProcessError(1, out)

    except subprocess.CalledProcessError as e:
        print (e.output)
    except subprocess.TimeoutExpired:
        print('TIMEOUT')
        print(out)

    # Cleanup resources
    subprocess.call('"@DOCKER_EXECUTABLE@" compose -f benchmark.compose.yml down', shell=True)

    assert(ret)
