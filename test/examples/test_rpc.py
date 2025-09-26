# Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

import os
import pytest
import subprocess

rpc_calculator_test_cases = [
    ('-r', 'Representation limits received: min_value = -2147483648, max_value = 2147483647'),
    ('--addition 1 2', 'Addition result: 1 + 2 = 3'),
    ('--substraction 8 5', 'Substraction result: 8 - 5 = 3'),
]

@pytest.mark.parametrize("client_args, expected_response", rpc_calculator_test_cases)
def test_rpc(client_args, expected_response):
    """."""

    ret = False
    out = ''

    menv = dict(os.environ)

    menv["CLIENT_ARGS"] =  client_args

    try:
        out = subprocess.check_output(
            '"@DOCKER_EXECUTABLE@" compose -f rpc.compose.yml up',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=20,
            env=menv
        ).decode().split('\n')

        for line in out:
            if line.find(expected_response) != -1:
                ret = True
                break

    except subprocess.CalledProcessError:
        for l in out:
            ret = False
            print(l)
    except subprocess.TimeoutExpired:
        ret = False
        print('TIMEOUT')
        print(out)

    assert(ret)
