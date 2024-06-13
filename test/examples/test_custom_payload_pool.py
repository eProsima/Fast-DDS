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
