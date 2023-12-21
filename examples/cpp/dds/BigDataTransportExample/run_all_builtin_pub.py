#!/usr/bin/python3

from signal import SIGINT
from signal import SIGKILL
import importlib
import os.path
import sys
import time
import subprocess

root_dir = '/home/carlos/fastdds_ws/build/fastrtps/examples/cpp/dds/BigDataTransportExample/'
interval_arg = '-i 100 '
samples_arg = '-s 100 '
wait_arg = '-w 1 '

timeout = 75

def run_test(builtin_transports, trans_rel_arg):

    for t in builtin_transports:
        print('\nSTART: Builtin Tranports = ' + t)
        command = "FASTDDS_BUILTIN_TRANSPORTS=" + t + " " + root_dir + \
            "/BigDataTransportExample publisher " + interval_arg + samples_arg + wait_arg + trans_rel_arg
        result_pub = subprocess.run(command,
                                    shell=True,
                                    preexec_fn=os.setsid,
                                    stdout=subprocess.PIPE,
                                    text=True,
                                    universal_newlines=True,
                                    timeout=timeout)
        # Wait 5 seconds between scenarios
        time.sleep(5)
        # Make sure pub is finished after timeout
        # os.killpg(os.getpgid(result_pub.pid), SIGKILL)
        print(result_pub.stdout)
        # open('output_pub_' + t + '.txt', "w").write(result_pub.stdout)
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

    if True:
        trans_rel_arg = '--transient --reliable'
    else:
        trans_rel_arg = ""

    run_test(builtin_transports, trans_rel_arg)

    return 0


if __name__ == '__main__':
    sys.exit(main())