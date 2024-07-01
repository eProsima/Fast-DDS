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

def test_request_reply_isolated():
    """."""
    expected_responses = {
        'request_reply_isolated-client-1-1': 5,
        'request_reply_isolated-client-2-1': 1,
        'request_reply_isolated-client-3-1': 6,
        'request_reply_isolated-client-4-1': 4
    }

    responses = {
        'request_reply_isolated-client-1-1': None,
        'request_reply_isolated-client-2-1': None,
        'request_reply_isolated-client-3-1': None,
        'request_reply_isolated-client-4-1': None
    }

    ret = True
    out = ''

    try:
        out = subprocess.check_output(
            '/usr/bin/docker compose -f request_reply_isolated.compose.yml up',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=30
        ).decode().split('\n')

        for line in out:
            for client in expected_responses:
                if client in line and 'Reply received from server' in line:
                    responses[client] = int(line.rstrip().split(' ')[-1])

        for client in responses:
            if responses[client] != expected_responses[client]:
                ret = False
                print('ERROR: ' + client + ' expected "' + expected_responses[client] + '" but received "' + responses[client] + '"')
                raise subprocess.CalledProcessError(1, '')

    except subprocess.CalledProcessError:
        for l in out:
            print(l)
    except subprocess.TimeoutExpired:
        print('TIMEOUT')
        print(out)

    assert(ret)
