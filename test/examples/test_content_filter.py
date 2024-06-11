"""."""
import subprocess
import pytest
import re

custom_filter_test_cases = [
    ('', True),
    ('', False),
    ('--reader-filters 0', True),
    ('--reader-filters 0', False)]
@pytest.mark.parametrize("pub_reader_filters, sub_custom_filter", custom_filter_test_cases)
def test_content_filter(pub_reader_filters, sub_custom_filter):
    """."""
    ret = False
    out = ''
    if (sub_custom_filter):
        command_prerequisites = 'PUB_ARGS="' + ' ' + pub_reader_filters + '" SUB_ARGS="' + ' --filter-kind custom" '
    else:
        command_prerequisites = 'PUB_ARGS="' + ' ' + pub_reader_filters + '" SUB_ARGS="' + ' --filter-kind default" '

    try:
        out = subprocess.check_output(command_prerequisites +
            '@DOCKER_EXECUTABLE@ compose -f content_filter.compose.yml up',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=30
        ).decode().split('\n')

        sent = 0
        received = 0
        data = []
        for line in out:
            if 'SENT' in line:
                sent += 1
                continue

            if 'RECEIVED' in line:
                received += 1
                match = re.search(r'index:\s+(\d+)', line)
                if match:
                    number = int(match.group(1))
                    data.append(number)
                continue

        ret = True
        if sent != 0 and received != 0:
            for elem in data:
                if (sub_custom_filter):
                    if not (elem < 3 and elem > 8):
                        ret = False
                else:
                    if not (elem >= 4 or elem <= 8):
                        ret = False
        else:
            print('ERROR: sent: ' + str(sent) + ', received: ' + str(received) +
                  ' but data are out of the filtered range')
            raise subprocess.CalledProcessError(1, '')

    except subprocess.CalledProcessError:
        for l in out:
            print(l)
    except subprocess.TimeoutExpired:
        print('TIMEOUT')
        print(out)

    assert(ret)
