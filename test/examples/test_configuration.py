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
    ('--keep-last 10 --async', '--keep-last 10 '),                                          # Publish mode QoS
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

    command_prerequisites = 'PUB_ARGS="' + pub_requirements + ' ' + pub_args + '" SUB_ARGS="' + sub_requirements + ' ' + sub_args + '" '
    try:
        out = subprocess.check_output(command_prerequisites + '@DOCKER_EXECUTABLE@ compose -f configuration.compose.yml up',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=20
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

    assert(ret)

timeout_test_cases = [
    ('--transport SHM', '--transport UDPv4'),                                       # Builtin transports
    ('--deadline 300', '--deadline 100'),                                           # Deadline QoS
    ('', '--disable-positive-ack'),                                                 # Disable positive ACKs QoS
    ('', '--transient-local'),                                                      # Durability QoS
    #('--keep-last 10 --max-samples-per-instance 1', ''), # this one only displays warning from now on
    ('--liveliness-kind AUTOMATIC', '--liveliness-kind MANUAL_BY_PARTICIPANT'),     # Liveliness QoS
    ('--liveliness-kind AUTOMATIC', '--liveliness-kind MANUAL_BY_TOPIC'),
    ('--liveliness-kind MANUAL_BY_PARTICIPANT', '--liveliness-kind MANUAL_BY_TOPIC'),
    ('--ownership', ''),                                                            # Ownership QoS
    ('', '--ownership'),
    ('--partition configuration_example_partition', '--partition other_partition'), # Partition QoS
    ('', '--reliable'),                                                             # Reliability QoS
]

@pytest.mark.parametrize("pub_args, sub_args", timeout_test_cases)
def test_configuration_timeout(pub_args, sub_args):
    """."""
    ret = False
    out = ''

    command_prerequisites = 'PUB_ARGS="' + pub_args + '" SUB_ARGS="' + sub_args + '" '
    try:
        out = subprocess.check_output(command_prerequisites + '@DOCKER_EXECUTABLE@ compose -f configuration.compose.yml up',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=10
        )
    except subprocess.CalledProcessError as e:
        print (e.output)
    except subprocess.TimeoutExpired:
        ret = True
        subprocess.check_output('@DOCKER_EXECUTABLE@ compose -f configuration.compose.yml down',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=15
        )

    assert(ret)

expected_output_test_cases = [
    ('--deadline 80', '--deadline 80', 'Deadline missed!', '10'),               # 10 = n samples sent
    ('--deadline 80', '--deadline 80', 'Requested deadline missed!', '15'),     # 20 = n samples received (10 x 2) (flaky)
    ('--liveliness 80 --liveliness-assert 70', '--liveliness 80 --liveliness-assert 70', 'Liveliness changed!', '1'),
    ('--liveliness-kind MANUAL_BY_TOPIC --liveliness 50 --liveliness-assert 40', '--liveliness-kind MANUAL_BY_TOPIC --liveliness 50 --liveliness-assert 40', 'Liveliness lost!', '10'),
]

@pytest.mark.parametrize("pub_args, sub_args, expected_message, n_messages", expected_output_test_cases)
def test_configuration_expected_output(pub_args, sub_args, expected_message, n_messages):
    """."""
    ret = False
    out = ''
    render_out = ''
    pub_requirements = '--reliable --transient-local --keep-last 10'
    sub_requirements = '--reliable --transient-local --keep-last 10'

    command_prerequisites = 'PUB_ARGS="' + pub_requirements + ' ' + pub_args + '" SUB_ARGS="' + sub_requirements + ' ' + sub_args + '" '
    try:
        out = subprocess.check_output(command_prerequisites + '@DOCKER_EXECUTABLE@ compose -f configuration.compose.yml up',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=20
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
