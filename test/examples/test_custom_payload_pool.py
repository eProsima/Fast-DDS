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
def test_custom_payload_pool():
    """."""
    ret = False
    out = ''
    try:
        out = subprocess.check_output(
            '@DOCKER_EXECUTABLE@ compose -f custom_payload_pool.compose.yml up',
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
