#!/usr/bin/python3

from signal import SIGINT
from signal import SIGKILL
import importlib
import os.path
import sys
import time
import subprocess

root_dir = '/home/carlos/fastdds_ws/build/fastrtps/examples/cpp/dds/BigDataTransportExample/'

timeout = 70

def run_test(builtin_transports, trans_rel_arg):

    for t in builtin_transports:
        print('\nSTART: Builtin Tranports = ' + t)
        command = "FASTDDS_BUILTIN_TRANSPORTS=" + t + " " + root_dir + \
            "/BigDataTransportExample subscriber " + trans_rel_arg
        result_sub = subprocess.Popen(command,
                                      shell=True,
                                      preexec_fn=os.setsid,
                                      stdout=subprocess.PIPE,
                                      text=True,
                                      universal_newlines=True)
        text = ""
        while True:
            output = result_sub.stdout.readline()
            if output == '' and result_sub.poll() is not None:
                break
            if output:
                print(output.strip())
                text = text + "\n" + output.strip()
                if output.strip().startswith("Subscriber unmatched"):
                    os.killpg(os.getpgid(result_sub.pid), SIGKILL)
                    open('output_sub_' + t + '.txt', "w").write(text)
        # Wait 5 seconds between scenarios
        time.sleep(5)
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

    if False:
        trans_rel_arg = '--transient --reliable'
    else:
        trans_rel_arg = ""

    run_test(builtin_transports, trans_rel_arg)

    return 0


if __name__ == '__main__':
    sys.exit(main())
