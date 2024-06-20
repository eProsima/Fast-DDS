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

"""
    Expected output test cases for Discovery Server example.
    Each element is a ternary (pub_args, sub_args, server_args)
"""
discovery_server_test_cases = [
    ('--samples 10', '--samples 10', '--timeout 5', '--listening-port 11400 --timeout 1'),
    ('--connection-port 11500 --samples 10', '--connection-port 11500 --samples 10', '--listening-port 11500 --timeout 5', '--listening-port 11400 --timeout 1'),
    ('--transport tcpv4 --samples 10', '--transport tcpv4 --samples 10', '--transport tcpv4 --timeout 5', '--listening-port 11400 --timeout 1'),
    ('--samples 10', '--samples 10', '--timeout 5', '--listening-port 11400 --timeout 1'),
    ('--samples 10', '--connection-port 11400 --samples 10', '--timeout 5 --connection-port 11400', '--listening-port 11400 --timeout 5'),
    ('--transport udpv6 --samples 10', '--transport udpv6 --connection-port 18000 --samples 10', '--transport udpv6 --timeout 5 --connection-port 18000', '--transport udpv6 --listening-port 18000 --timeout 5'),
    ('--transport tcpv4 --samples 10', '--transport tcpv4 --connection-port 18000 --samples 10', '--transport tcpv4 --timeout 5 --connection-port 18000', '--transport tcpv4 --listening-port 18000 --timeout 5'),
    ('--transport tcpv6 --samples 10', '--transport tcpv6  --connection-port 18000 --samples 10', '--transport tcpv6 --timeout 5 --connection-port 18000', '--transport tcpv6 --listening-port 18000 --timeout 5')
]

@pytest.mark.parametrize("pub_args, sub_args, server1_args, server2_args", discovery_server_test_cases)
def test_discovery_server(pub_args, sub_args, server1_args, server2_args):
    """."""
    ret = False
    out = ''
    command_args = 'PUB_ARGS="' + pub_args + '" SUB_ARGS="' + sub_args + '" SERVER1_ARGS="' + server1_args + '" SERVER2_ARGS="' + server2_args + '" '
    try:
        out = subprocess.check_output(
            command_args + '@DOCKER_EXECUTABLE@ compose -f discovery_server.compose.yml up',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=30
        ).decode().split('\n')

        sent = 0
        received = 0
        for line in out:
            if 'SENT' in line:
                sent += 1
                continue

            if 'RECEIVED' in line:
                received += 1
                continue

        if sent != 0 and received != 0 and sent == received:
            ret = True
        else:
            print('ERROR: sent: ' + str(sent) + ', but received: ' + str(received))
            raise subprocess.CalledProcessError(1, '')

    except subprocess.CalledProcessError:
        for l in out:
            print(l)
    except subprocess.TimeoutExpired:
        print('TIMEOUT')
        print(out)

    assert(ret)
