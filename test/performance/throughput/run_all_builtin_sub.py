#!/usr/bin/python3

from signal import SIGINT
from signal import SIGKILL
import importlib
import os.path
import sys
import time
import subprocess

root_dir = '/home/carlos/fastdds_ws/build/fastrtps/examples/cpp/dds/BigDataTransportExample/'

def run_test(builtin_transports):

    for t in builtin_transports:
        print('\n========================================')
        print('START: Builtin Tranports = ' + t)

        env_var = "FASTDDS_BUILTIN_TRANSPORTS=" + t
        root_dir = '/home/carlos/fastdds_ws/build/fastrtps/test/performance/throughput/ThroughputTest'
        reliability = ["--reliability=reliable"]

        timeout_ = 60

        command = [
            env_var,
            root_dir,
            'subscriber'
        ]

        command += reliability

        command = ' '.join(element for element in command)
        print('Used command:', command)

        result_sub = subprocess.run(command,
                                    shell=True,
                                    preexec_fn=os.setsid,
                                    stdout=subprocess.PIPE,
                                    text=True,
                                    universal_newlines=True,
                                    timeout=timeout_)

        print(result_sub.stdout)
        # Wait 1 second between tests
        time.sleep(1)

        root_dir_l = '/home/carlos/fastdds_ws/build/fastrtps/test/performance/latency/LatencyTest'

        command_l = [
            env_var,
            root_dir_l,
            'subscriber'
        ]

        command_l = ' '.join(element for element in command_l)
        print('Latency command:', command_l)

        result_sub_l = subprocess.run(command_l,
                                      shell=True,
                                      preexec_fn=os.setsid,
                                      stdout=subprocess.PIPE,
                                      text=True,
                                      universal_newlines=True,
                                      timeout=timeout_)

        print(result_sub_l.stdout)
        # Wait 3 seconds between scenarios
        time.sleep(3)

        print('\nEND: Builtin Tranports = ' + t)

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
