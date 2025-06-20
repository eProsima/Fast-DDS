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
    ('--keep-last 10 --liveliness 500 --liveliness-assert 400', '--keep-last 10 --liveliness 500 --liveliness-assert 400'), # Liveliness QoS
    ('--keep-last 10 --liveliness-kind AUTOMATIC', '--keep-last 10 --liveliness-kind AUTOMATIC'),
    ('--keep-last 10 --liveliness-kind MANUAL_BY_PARTICIPANT', '--keep-last 10 --liveliness-kind MANUAL_BY_PARTICIPANT'),
    ('--keep-last 10 --liveliness-kind MANUAL_BY_TOPIC', '--keep-last 10 --liveliness-kind MANUAL_BY_TOPIC'),
    ('--keep-last 10 --liveliness-assert 500', '--keep-last 10 --liveliness-assert 500'),
    ('--keep-last 10 --ownership', '--keep-last 10 --ownership'),                           # Ownership QoS
    ('--keep-last 10 --ownership --ownership-strength 10', '--keep-last 10 --ownership'),
    ('--keep-last 10 --partition configuration_example_partition', '--keep-last 10 --partition configuration_example_partition'), # Partition QoS
    ('--keep-last 10 --partition \'configuration_example_partition\'', '--keep-last 10 --partition \'configuration_example_partition\''),
    ('--keep-last 10 --partition "configuration_example_partition"', '--keep-last 10 --partition "configuration_example_partition"'),
    ('--keep-last 10 --profile-participant configuration_participant_profile', '--keep-last 10 --profile-participant configuration_participant_profile'),
    ('--keep-last 10 --profile-writer configuration_datawriter_profile', '--keep-last 10 --profile-reader configuration_datareader_profile' ),
    ('--keep-last 10 --async', '--keep-last 10'),                                          # Publish mode QoS
    ('--keep-last 10 --max-instances 1', '--keep-last 10 --max-instances 1'),               # Resource limits QoS
    ('--keep-last 10 --max-samples-per-instance 10', '--keep-last 10 --max-samples-per-instance 10'),
    ('--keep-last 10 --max-instances 1 --max-samples-per-instance 10 --max-samples 10', '--keep-last 10 --max-instances 1 --max-samples-per-instance 10 --max-samples 10'),
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
