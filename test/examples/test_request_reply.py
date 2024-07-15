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

import re
import subprocess

def parse_response(client_responses: dict, response: str):
    """."""
    # Define the regex pattern with two capturing groups
    pattern = r"ID '(\d+)' with result: '(-?\d+)'"

    # Use re.search to find the first match
    match = re.search(pattern, response)

    if match:
        id_str = match.group(1)           # First capturing group
        result_int = int(match.group(2))  # Second capturing group, convert to int

        client_responses[id_str] = result_int

    return client_responses


def test_request_reply():
    """."""
    expected_responses = {
        'server-client-1': {
            '1': 7,
            '2': -3,
            '3': 10,
            '4': 0
        },
        'alone-client-1': {
            '1': 91,
            '2': 43,
            '3': 1608,
            '4': 2
        },
    }

    responses = {
        'server-client-1': {},
        'alone-client-1': {},
    }

    ret = True
    out = ''

    try:
        out = subprocess.check_output(
            '/usr/bin/docker compose -f request_reply.compose.yml up',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=30
        ).decode().split('\n')

        for line in out:
            for client in expected_responses:
                if client in line and 'Reply received from server' in line:
                    responses[client] = parse_response(responses[client], line)

        for client in responses:
            if responses[client] != expected_responses[client]:
                ret = False
                print(f'ERROR: {client} expected "{expected_responses[client]}" but received "{responses[client]}"')
                raise subprocess.CalledProcessError(1, '')

    except subprocess.CalledProcessError:
        for l in out:
            ret = False
            print(l)
    except subprocess.TimeoutExpired:
        ret = False
        print('TIMEOUT')
        print(out)

    assert(ret)
