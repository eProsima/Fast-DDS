import subprocess
import pytest

"""
    Expected output test cases for Discovery Server example.
    Each element is a ternary (pub_args, sub_args, server_args)
"""
discovery_server_test_cases = [
    ('--samples 10', '--samples 10', '--timeout 5', '--id 2 --listening-port 11400 --timeout 1'),
    ('--connection-port 11500 --samples 10', '--connection-port 11500 --samples 10', '--listening-port 11500 --timeout 5', '--id 2 --listening-port 11400 --timeout 1'),
    ('--transport tcpv4 --samples 10', '--transport tcpv4 --samples 10', '--transport tcpv4 --timeout 5', '--id 2 --listening-port 11400 --timeout 1'),
    ('--connection-ds-id 1 --samples 10', '--connection-ds-id 1 --samples 10', '--id 1 --timeout 5', '--id 2 --listening-port 11400 --timeout 1'),
    ('--connection-ds-id 1 --samples 10', '--connection-ds-id 2 --connection-port 11400 --samples 10', '--id 1 --timeout 5 --connection-ds-id 2 --connection-port 11400', '--id 2 --listening-port 11400 --timeout 5')
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
