#!/usr/bin/python3

from signal import SIGINT
from signal import SIGKILL
import importlib
import os.path
import sys
import time
import subprocess

root_dir = '/home/carlos/fastdds_ws/build/fastrtps/examples/cpp/dds/BigDataTransportExample/'
xml_dir = '/home/carlos/fastdds_ws/src/fastrtps/examples/cpp/dds/BigDataTransportExample/'
interval_arg = '-i 100 '
samples_arg = '-s 10 '
history_arg = '--history '

def udp_default():

    print('\nUDP default')
    xml_sub = xml_dir + 'sub_udp_reduce_maxMessageSize.xml'
    command = "FASTRTPS_DEFAULT_PROFILES_FILE=" + xml_sub + " " + root_dir + \
        "BigDataTransportExample subscriber --xml-profile bigdata_sub_profile " + samples_arg + history_arg + "1 "
    result_sub = subprocess.Popen(command,
                                  shell=True,
                                  preexec_fn=os.setsid,
                                  text=True,
                                  universal_newlines=True)

    xml_pub = xml_dir + 'pub_udp_reduce_maxMessageSize.xml'
    command = "FASTRTPS_DEFAULT_PROFILES_FILE=" + xml_pub + " " + root_dir + \
        "BigDataTransportExample publisher --xml-profile bigdata_pub_profile -w 1 " + interval_arg + samples_arg + history_arg + "1 " + "--transient --reliable"
    result_pub = subprocess.run(command,
                                shell=True,
                                stdout=subprocess.PIPE,
                                text=True,
                                universal_newlines=True)
    time.sleep(2)
    # Pub is ended after sending 'samples_arg' samples, need to end subscriber manually in case some samples were not received
    os.killpg(os.getpgid(result_sub.pid), SIGKILL)

def udp_transient_local():
    print('\nUDP Reliable & Transient with Keep_all history')
    xml_sub = xml_dir + 'sub_udp_reduce_maxMessageSize.xml'
    command = "FASTRTPS_DEFAULT_PROFILES_FILE=" + xml_sub + " " + root_dir + \
        "BigDataTransportExample subscriber --xml-profile bigdata_sub_profile " + samples_arg + "--transient --reliable"
    result_sub = subprocess.Popen(command,
                                  shell=True,
                                  preexec_fn=os.setsid,
                                  text=True,
                                  universal_newlines=True)

    xml_pub = xml_dir + 'pub_udp_reduce_maxMessageSize.xml'
    command = "FASTRTPS_DEFAULT_PROFILES_FILE=" + xml_pub + " " + root_dir + \
        "BigDataTransportExample publisher --xml-profile bigdata_pub_profile -w 1 " + interval_arg + samples_arg + "--transient --reliable"
    result_pub = subprocess.run(command,
                                shell=True,
                                stdout=subprocess.PIPE,
                                text=True,
                                universal_newlines=True)
    time.sleep(2)
    # Pub is ended after sending 'samples_arg' samples, need to end subscriber manually in case some samples were not received
    os.killpg(os.getpgid(result_sub.pid), SIGKILL)

def udp_transient_local_history(history):
    print('\nUDP Reliable & Transient with History', history)
    xml_sub = xml_dir + 'sub_udp_reduce_maxMessageSize.xml'
    command = "FASTRTPS_DEFAULT_PROFILES_FILE=" + xml_sub + " " + root_dir + \
        "BigDataTransportExample subscriber --xml-profile bigdata_sub_profile " + samples_arg + "--transient --reliable " + history_arg + history
    result_sub = subprocess.Popen(command,
                                  shell=True,
                                  preexec_fn=os.setsid,
                                  text=True,
                                  universal_newlines=True)

    xml_pub = xml_dir + 'pub_udp_reduce_maxMessageSize.xml'
    command = "FASTRTPS_DEFAULT_PROFILES_FILE=" + xml_pub + " " + root_dir + \
        "BigDataTransportExample publisher --xml-profile bigdata_pub_profile -w 1 " + interval_arg + samples_arg + "--transient --reliable " + history_arg + history
    result_pub = subprocess.run(command,
                                shell=True,
                                stdout=subprocess.PIPE,
                                text=True,
                                universal_newlines=True)
    time.sleep(2)
    # Pub is ended after sending 'samples_arg' samples, need to end subscriber manually in case some samples were not received
    os.killpg(os.getpgid(result_sub.pid), SIGKILL)


def tcp_with_udp():
    print('\nTCP with UDP discovery')
    xml_sub = xml_dir + 'participant_config_sub.xml'
    command = "FASTRTPS_DEFAULT_PROFILES_FILE=" + xml_sub + " " + root_dir + \
        "BigDataTransportExample subscriber --xml-profile bigdata_sub_profile " + samples_arg + history_arg + "1 "
    result_sub = subprocess.Popen(command,
                                  shell=True,
                                  text=True,
                                  universal_newlines=True)

    xml_pub = xml_dir + 'participant_config_pub.xml'
    command = "FASTRTPS_DEFAULT_PROFILES_FILE=" + xml_pub + " " + root_dir + \
        "BigDataTransportExample publisher --xml-profile bigdata_pub_profile -w 1 " + interval_arg + samples_arg + history_arg + "1 " + "--transient --reliable"
    result_pub = subprocess.run(command,
                                shell=True,
                                stdout=subprocess.PIPE,
                                text=True,
                                universal_newlines=True)
    time.sleep(2)
    # Pub is ended after sending 'samples_arg' samples, need to end subscriber manually in case some samples were not received
    os.killpg(os.getpgid(result_sub.pid), SIGKILL)

def main():

    if len(sys.argv) < 2:
        print('No transport provided, using default: udp, udp_rel_tran, udp_rel_tran_hist, tcp_udp')
        transports = ['udp', 'udp_rel_tran', 'udp_rel_tran_hist', 'tcp_udp']
    else: 
        transports = sys.argv[1:]

    for t in transports:
        if t == 'udp' or t == 'udp_rel_tran' or t == 'tcp_udp' or t == 'udp_rel_tran_hist':
            print('Argument: ' + t)
        else: 
            print("ERROR: " + t + ' is not a valid argument:', 'udp, udp_rel_tran, udp_rel_tran_hist, tcp_udp')
            return 1

    for t in transports:
        if t == 'udp':
            udp_default()
        if t == 'udp_rel_tran':
            udp_transient_local()
        if t == 'udp_rel_tran_hist':
            udp_transient_local_history(' 1')
        if t == 'tcp_udp':
            tcp_with_udp()

    return 0


if __name__ == '__main__':
    sys.exit(main())
