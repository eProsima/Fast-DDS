
import subprocess
import pytest

config_test_cases = [
    ('', ''),                                                                               # Default configuration
    ('--profile-participant configuration_participant_profile', '--profile-participant configuration_participant_profile'),
    ('--profile-writer configuration_writer_profile', '--profile-reader configuration_reader_profile' ),
    ('--profile-writer best_effort_writer_profile', '--profile-reader best_effort_reader_profile'),
    ('--transport DEFAULT', '--transport DEFAULT'),
    ('--transport DEFAULT', '--transport UDPv4'),
    ('--transport UDPv4', '--transport UDPv4'),
    ('--transport LARGE_DATA', '--transport LARGE_DATA'),
    # TODO test deadline QoS
    # TODO test lifespan QoS
    # TODO test liveliness QoS
    # TODO test ownership QoS
    # TODO test disable positive acks QoS
    # TODO test partitions QoS
]

@pytest.mark.parametrize("pub_args, sub_args", config_test_cases)
def test_configuration(pub_args, sub_args):
    """."""
    ret = False
    out = ''
    pub_requirements = '--reliable --transient-local --keep-last 10 --samples 10 --wait 2'
    sub_requirements = '--reliable --transient-local --keep-last 10 --samples 10'

    command_prerequisites = 'PUB_ARGS="' + pub_requirements + ' ' + pub_args + '" SUB_ARGS="' + sub_requirements + ' ' + sub_args + '" '
    try:
        out = subprocess.check_output(command_prerequisites + '@DOCKER_EXECUTABLE@ compose -f configuration.compose.yml up',
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

        if sent * 2 == received:
            ret = True
        else:
            print ('ERROR: sent: ' + str(sent) + ', but received: ' + str(received) + '(expected: ' + str(sent * 2) + ')')
            raise subprocess.CalledProcessError(1, '')

    except subprocess.CalledProcessError:
        for l in out:
            print(l)
    except subprocess.TimeoutExpired:
        print('TIMEOUT')
        print(out)

    assert(ret)
