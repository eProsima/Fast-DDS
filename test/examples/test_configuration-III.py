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

# Python's check_output() in windows does not kill the shell
# process on timeout, skip these tests on windowds
if os.name != 'nt':

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
        menv = dict(os.environ)

        menv["PUB_ARGS"] =  pub_args
        menv["SUB_ARGS"] =  sub_args
        menv["CONTAINER_SUFFIX_COMPOSE"] = str(random.randint(0, 100))

        try:
            out = subprocess.check_output('"@DOCKER_EXECUTABLE@" compose -f configuration.compose.yml up',
                stderr=subprocess.STDOUT,
                shell=True,
                timeout=10,
                env=menv
            )
        except subprocess.CalledProcessError as e:
            print (e.output)
        except subprocess.TimeoutExpired:
            ret = True
            subprocess.check_output('@DOCKER_EXECUTABLE@ compose -f configuration.compose.yml down',
                stderr=subprocess.STDOUT,
                shell=True,
                timeout=15,
                env=menv
            )

        # Cleanup resources
        subprocess.call('"@DOCKER_EXECUTABLE@" compose -f configuration.compose.yml down', shell=True)

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

    menv = dict(os.environ)

    menv["PUB_ARGS"] =  pub_requirements + ' ' + pub_args
    menv["SUB_ARGS"] =  sub_requirements + ' ' + sub_args
    menv["CONTAINER_SUFFIX_COMPOSE"] = str(random.randint(0, 100))

    try:
        out = subprocess.check_output('"@DOCKER_EXECUTABLE@" compose -f configuration.compose.yml up',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=60,
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

    # Cleanup resources
    subprocess.call('"@DOCKER_EXECUTABLE@" compose -f configuration.compose.yml down', shell=True)

    assert(ret)
