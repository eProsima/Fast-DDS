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

config_test_cases = [
    ('--keep-last 10 --transport DEFAULT', '--keep-last 10 --transport DEFAULT'),           # Builtin transports
    ('--keep-last 10 --transport DEFAULT', '--keep-last 10 --transport UDPv4'),
    ('--keep-last 10 --transport UDPv4', '--keep-last 10 --transport UDPv4'),
    ('--keep-last 10 --transport LARGE_DATA', '--keep-last 10 --transport LARGE_DATA'),
    ('--keep-last 10 --deadline 300', '--keep-last 10 --deadline 300'),                     # Deadline QoS
    ('--keep-last 10 --deadline 80', '--keep-last 10 --deadline 80'),
    ('--keep-last 10 --disable-positive-ack', '--keep-last 10 --disable-positive-ack'),     # Disable positive ACKs QoS
    ('--keep-last 10 --disable-positive-ack --ack-keep-duration 10', '--keep-last 10 --disable-positive-ack'),
    ('--keep-last 100', '--keep-last 100'),                                                 # History QoS
    ('--keep-last 10', '--keep-all'),
    ('--keep-all', '--keep-all'),
    ('--keep-last 10 --lifespan 500', '--keep-last 10 --lifespan 500'),                     # Lifespan QoS
]

@pytest.mark.parametrize("pub_args, sub_args", config_test_cases)
def test_configuration(pub_args, sub_args):
    """."""
    ret = False
    out = ''
    pub_requirements = '--reliable --transient-local'
    sub_requirements = '--reliable --transient-local'
    menv = dict(os.environ)

    menv["PUB_ARGS"] =  pub_requirements + ' ' + pub_args
    menv["SUB_ARGS"] =  sub_requirements + ' ' + sub_args
    menv["CONTAINER_SUFFIX_COMPOSE"] = str(random.randint(0, 100))

    timeout = 30
    #In windows timeout argument does not wor properly
    #It is not able to kill the subprocess when it is surpassed
    #and when the subprocess exists then it is checked.
    #This test is about samples (1000) so for big msgs in windows
    #it can take longer. Set it to a higher value
    if os.name == 'nt':
        timeout = 200

    try:
        out = subprocess.check_output('"@DOCKER_EXECUTABLE@" compose -f configuration.compose.yml up',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=timeout,
            env=menv
        )
        render_out = out.decode().split('\n')

        sent = 0
        received = 0
        for line in render_out:
            if 'SENT' in line:
                sent += 1

            if 'RECEIVED' in line:
                received += 1

        if sent != 0 and received != 0 and sent * 2 == received:
            ret = True
        else:
            print ('ERROR: sent: ' + str(sent) + ', but received: ' + str(received) + ' (expected: ' + str(sent * 2) + ')')
            raise subprocess.CalledProcessError(1, out.decode())

    except subprocess.CalledProcessError as e:
        print (e.output)
    except subprocess.TimeoutExpired:
        print('TIMEOUT')
        print(out)

    # Cleanup resources
    subprocess.call('"@DOCKER_EXECUTABLE@" compose -f configuration.compose.yml down', shell=True)

    assert(ret)
