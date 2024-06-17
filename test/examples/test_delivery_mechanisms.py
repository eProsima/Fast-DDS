import subprocess
import pytest

delivery_test_cases = [
    ('', '', '--unknown-command', 2),
    ('--unknown-command', '', '', 3),
    ('--unknown-command', '', ' --ignore-local-endpoints', 2),
    ('--mechanism shm', '--mechanism shm', '--unknown-command', 2),
    ('--unknown-command', '--mechanism shm', '--mechanism shm', 3),
    ('--unknown-command', '--mechanism shm', '--mechanism shm --ignore-local-endpoints', 2),
    ('--mechanism udp', '--mechanism udp', '--unknown-command', 2),
    ('--unknown-command', '--mechanism udp', '--mechanism udp', 3),
    ('--unknown-command', '--mechanism udp', '--mechanism udp --ignore-local-endpoints', 2),
    # tcp takes longer to match, so explicitly expect much more samples in this case
    #  Note: pubsub with TCP and ignore-local-endpoints NOT set is not supported, tested in  below tests
    ('--mechanism tcp -s 220', '--mechanism tcp -s 220', '--unknown-command', 2),
    ('--unknown-command', '--mechanism tcp -s 220', '--mechanism tcp -s 220 --ignore-local-endpoints', 2),
    ('--mechanism data-sharing', '--mechanism data-sharing', '--unknown-command', 2),
    ('--unknown-command', '--mechanism data-sharing', '--mechanism data-sharing', 3),
    ('--unknown-command', '--mechanism data-sharing', '--mechanism data-sharing --ignore-local-endpoints', 2),
#    # Intra-process only makes sense for pubsub entities. This test forces entities != pubsub  to fail, so
#    #  only 1 message is expected per sample (the local one)
    ('--unknown-argument', '--unknown-argument', '--mechanism intra-process', 1)
]

@pytest.mark.parametrize("pub_args, sub_args, pubsub_args, repetitions", delivery_test_cases)
def test_delivery_mechanisms(pub_args, sub_args, pubsub_args, repetitions):
    """."""
    ret = False
    out = ''

    command_prerequisites = 'PUB_ARGS="' + pub_args + '" SUB_ARGS="' + sub_args + '" PUBSUB_ARGS="' + pubsub_args + '" '
    try:
        out = subprocess.check_output(command_prerequisites + '@DOCKER_EXECUTABLE@ compose -f delivery_mechanisms.compose.yml up',
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

    assert(ret)

timeout_test_cases = [
    ('--mechanism tcp', '--mechanism udp', '--mechanism shm -i'),
    ('--mechanism shm', '--mechanism data-sharing', '--mechanism intra-process -i')
]

@pytest.mark.parametrize("pub_args, sub_args, pubsub_args", timeout_test_cases)
def test_delivery_mechanisms_timeout(pub_args, sub_args, pubsub_args):
    """."""
    ret = False
    out = ''

    command_prerequisites = 'PUB_ARGS="' + pub_args + '" SUB_ARGS="' + sub_args + '" PUBSUB_ARGS="' + pubsub_args + '" '
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
        subprocess.check_output('@DOCKER_EXECUTABLE@ compose -f delivery_mechanisms.compose.yml down',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=15
        )

    assert(ret)

expected_output_test_cases = [
    ('--unknown-argument', '--unknown-argument', '--mechanism tcp', 'Unsupported', 1)
]

@pytest.mark.parametrize("pub_args, sub_args, pubsub_args, expected_message, n_messages", expected_output_test_cases)
def test_delivery_mechanisms_expected_output(pub_args, sub_args, pubsub_args, expected_message, n_messages):
    """."""
    ret = False
    out = ''
    render_out = ''

    command_prerequisites = 'PUB_ARGS="' + pub_args + '" SUB_ARGS="' + sub_args + '" PUBSUB_ARGS="' + pubsub_args + '" '
    try:
        out = subprocess.check_output(command_prerequisites + '@DOCKER_EXECUTABLE@ compose -f delivery_mechanisms.compose.yml up',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=20
        )
        render_out = out.decode().split('\n')

        count = 0
        for line in render_out:
            if expected_message in line:
                count += 1
                continue

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
