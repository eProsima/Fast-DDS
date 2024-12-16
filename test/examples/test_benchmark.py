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
import pytest
import re

config_test_cases = [
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

@pytest.mark.parametrize("pub_args, sub_args", config_test_cases)
def test_benchmark(pub_args, sub_args):
    """."""
    ret = False
    out = ''

    command_prerequisites = 'PUB_ARGS="' + pub_args + '" SUB_ARGS="' + sub_args + '" '

    try:
        out = subprocess.check_output(command_prerequisites + '@DOCKER_EXECUTABLE@ compose -f benchmark.compose.yml up',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=5
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
            raise subprocess.CalledProcessError(1, out.decode())

    except subprocess.CalledProcessError as e:
        print (e.output)
    except subprocess.TimeoutExpired:
        print('TIMEOUT')
        print(out)

    assert(ret)
