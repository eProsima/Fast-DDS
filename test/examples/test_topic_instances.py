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

topic_instances_test_cases = [
    ('--samples 20', '--samples 20'),
    ('--instances 1', '--instances 1'),
    ('--instances 3', '--instances 5'),  # only the first 3 instances should receive the messages
]

@pytest.mark.parametrize("pub_args, sub_args", topic_instances_test_cases)
def test_topic_instances(pub_args, sub_args):
    """."""
    ret = False
    out = ''

    menv = dict(os.environ)

    menv["PUB_ARGS"] =  pub_args
    menv["SUB_ARGS"] =  sub_args

    try:
        out = subprocess.check_output('"@DOCKER_EXECUTABLE@" compose -f topic_instances.compose.yml up',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=60,
            env=menv
        ).decode().split('\n')

        sent = 0
        received = 0
        for line in out:
            if 'SENT' in line:
                sent += 1

            if 'RECEIVED' in line:
                received += 1

        if sent != 0 and received != 0 and sent * 2 == received:
            ret = True
        else:
            print('ERROR: sent: ' + str(sent) + ', but received: ' + str(received) +
                  ' (expected: ' + str(sent * 2) + ')')
            raise subprocess.CalledProcessError(1, '')

    except subprocess.CalledProcessError:
        for l in out:
            print(l)
    except subprocess.TimeoutExpired:
        print('TIMEOUT')
        print(out)

    assert(ret)

timeout_test_cases = [
    ('--timeout 0', '--samples 0'),
]

# Python's check_output() in windows does not kill the shell
# process on timeout, skip these tests on windowds
if os.name != 'nt':
    @pytest.mark.parametrize("pub_args, sub_args", timeout_test_cases)
    def test_topic_instances_timeout(pub_args, sub_args):
        """."""
        ret = False
        out = ''

        menv = dict(os.environ)

        menv["PUB_ARGS"] =  pub_args
        menv["SUB_ARGS"] =  sub_args

        try:
            out = subprocess.check_output('"@DOCKER_EXECUTABLE@" compose -f topic_instances.compose.yml up',
                stderr=subprocess.STDOUT,
                shell=True,
                timeout=10,
                env=menv
            )
        except subprocess.CalledProcessError as e:
            print (e.output)
        except subprocess.TimeoutExpired:
            ret = True
            subprocess.check_output('"@DOCKER_EXECUTABLE@" compose -f topic_instances.compose.yml down',
                stderr=subprocess.STDOUT,
                shell=True,
                timeout=15,
                env=menv
            )

        assert(ret)
