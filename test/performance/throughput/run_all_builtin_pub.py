#!/usr/bin/python3

from signal import SIGINT
from signal import SIGKILL
import importlib
import os.path
import sys
import time
import subprocess

def run_test(builtin_transports):

    for t in builtin_transports:
        print('\n========================================')
        print('START: Builtin Tranports = ' + t)

        env_var = "FASTDDS_BUILTIN_TRANSPORTS=" + t
        root_dir_tp = '/home/carlos/fastdds_ws/build/fastrtps/test/performance/throughput/ThroughputTest'
        reliability = ["--reliability=reliable"]
        demand = ["--demand=1"]
        period = ["--recovery_time=100"]
        duration = ["--time=10"]
        bytes = ["--msg_size=1000000"]
        data_sharing = ["--data_sharing=off"]

        timeout_ = 60

        command = [
            env_var,
            root_dir_tp,
            'publisher'
        ]

        command += reliability
        command += demand
        command += period
        command += duration
        command += bytes
        command += data_sharing

        command = ' '.join(element for element in command)
        print('Throughput command:', command)

        result_pub = subprocess.run(command,
                                    shell=True,
                                    preexec_fn=os.setsid,
                                    stdout=subprocess.PIPE,
                                    text=True,
                                    universal_newlines=True,
                                    timeout=timeout_)

        print(result_pub.stdout)
        open('output_' + t + '.txt', "w").write(result_pub.stdout)
        # Wait 1 second between tests
        time.sleep(1)

        root_dir_l = '/home/carlos/fastdds_ws/build/fastrtps/test/performance/latency/LatencyTest'
        samples = ["--samples=100"]

        command_l = [
            env_var,
            root_dir_l,
            'publisher'
        ]

        command_l += samples

        command_l = ' '.join(element for element in command_l)
        print('Latency command:', command_l)

        result_pub_l = subprocess.run(command_l,
                                      shell=True,
                                      preexec_fn=os.setsid,
                                      stdout=subprocess.PIPE,
                                      text=True,
                                      universal_newlines=True,
                                      timeout=timeout_)

        print(result_pub_l.stdout)
        open('output_' + t + '.txt', "a").write('\n\n===============================\n')
        open('output_' + t + '.txt', "a").write(result_pub_l.stdout)
        # Wait 3 seconds between scenarios
        time.sleep(3)

        print('END: Builtin Tranports = ' + t)

def main():

    if len(sys.argv) < 2:
        print('No transport provided, using default: DEFAULT, DEFAULTv6, SHM, UDPv4, UDPv6, LARGE_DATA')
        builtin_transports = ['DEFAULT', 'DEFAULTv6', 'UDPv4', 'UDPv6', 'LARGE_DATA']
    else:
        builtin_transports = sys.argv[1:]

    for t in builtin_transports:
        if t == 'DEFAULT' or t == 'DEFAULTv6' or t == 'UDPv4' or t == 'UDPv6' or t == 'LARGE_DATA':
            print('Argument: ' + t)
        else:
            print("ERROR: " + t + ' is not a valid argument:', 'DEFAULT, DEFAULTv6, SHM, UDPv4, UDPv6, LARGE_DATA')
            return 1

    run_test(builtin_transports)

    return 0


if __name__ == '__main__':
    sys.exit(main())