import subprocess
import pytest

delivery_test_cases = [
    ('--mechanism shm', '--mechanism shm', 3),
    ('--mechanism shm --ignore-local-endpoints', '--mechanism shm', 2),
    ('--mechanism udp', '--mechanism udp', 3),
    ('--mechanism udp --ignore-local-endpoints', '--mechanism udp', 2),
    ('--mechanism tcp', '--mechanism tcp', 3),
    ('--mechanism tcp --ignore-local-endpoints', '--mechanism tcp', 2),
    ('--mechanism data-sharing', '--mechanism data-sharing', 3),
    ('--mechanism data-sharing --ignore-local-endpoints', '--mechanism data-sharing', 2),
    ('--mechanism intra-process', '--unknown-argument', 1)      # force subscribers to fail
]

@pytest.mark.parametrize("pub_args, sub_args, repetitions", delivery_test_cases)
def test_delivery_mechanisms(pub_args, sub_args, repetitions):
    """."""
    ret = False
    out = ''

    command_prerequisites = 'PUB_ARGS="' + pub_args + '" SUB_ARGS="' + sub_args + '" '
    try:
        out = subprocess.check_output(command_prerequisites + '@DOCKER_EXECUTABLE@ compose -f delivery_mechanisms.compose.yml up',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=60 # TCP requires higher timeout
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
        if sent != 0 and received != 0 and sent * repetitions == received:
            ret = True
        else:
            print('ERROR: sent: ' + str(sent) + ', but received: ' + str(received) +
                  ' (expected: ' + str(sent * repetitions) + ')')
            raise subprocess.CalledProcessError(1, '')

    except subprocess.CalledProcessError:
        for l in out:
            print(l)
    except subprocess.TimeoutExpired:
        print('TIMEOUT')
        print(out)

delivery_timeout_test_cases = [
    ('--mechanism shm --ignore-local-endpoints', '--mechanism udp'),
    ('--mechanism shm --ignore-local-endpoints', '--mechanism tcp'),
    ('--mechanism udp --ignore-local-endpoints', '--mechanism tcp'),
    ('--mechanism udp --ignore-local-endpoints', '--mechanism data-sharing'),
    ('--mechanism tcp --ignore-local-endpoints', '--mechanism data-sharing'),
    ('--mechanism intra-process --ignore-local-endpoints', '--unknown-argument')    # force subscribers to fail
]

@pytest.mark.parametrize("pub_args, sub_args", delivery_timeout_test_cases)
def test_delivery_mechanisms_timeout(pub_args, sub_args):
    """."""
    ret = False
    out = ''

    command_prerequisites = 'PUB_ARGS="' + pub_args + '" SUB_ARGS="' + sub_args + '" '
    try:
        out = subprocess.check_output(command_prerequisites + '@DOCKER_EXECUTABLE@ compose -f delivery_mechanisms.compose.yml up',
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
            timeout=30
        )

    assert(ret)
