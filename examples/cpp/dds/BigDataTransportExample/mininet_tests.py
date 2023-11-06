#!/usr/bin/python3

from mininet.net import Mininet
from mininet.node import CPULimitedHost
from mininet.link import TCLink
from mininet.util import dumpNodeConnections, waitListening, decode, pmonitor
from signal import SIGINT
from signal import SIGKILL
import importlib
import os.path
import sys
import time
import pyutils
import subprocess

root_dir = '/home/carlos/fastdds_ws/build/fastrtps/examples/cpp/dds/BigDataTransportExample/'
xml_dir = '/home/carlos/fastdds_ws/src/fastrtps/examples/cpp/dds/BigDataTransportExample/'
interval_arg = '-i 100 '
samples_arg = '-s 10 '
history_arg = '--history '
configs = ['udp', 'udp_rel_tran', 'udp_rel_tran_hist', 'tcp_udp']

def clean_mn():
    print('Cleaning previous mininet config...')
    proc = subprocess.run(
        ['sudo',
         'mn',
         '-c'],
        stdout=subprocess.PIPE,
        text=True
        )

def ping_test(net, scenario_module):
    print('Doing ping test')
    source, sink = pyutils.get_source_and_sink(net, scenario_module)
    net.ping((source, sink))

def ping_test_scenario11(net, scenario_module):
    print('Doing ping test of Scenario 11')
    source, sink1, sink2, sink3 = pyutils.get_source_and_sink_scenario11(net, scenario_module)
    net.ping((source, sink1))
    net.ping((source, sink2))
    net.ping((source, sink3))

def raw_bandwidth_test(net, scenario_module):
    source, sink = pyutils.get_source_and_sink(net, scenario_module)

    print('Raw bandwidth test (TCP)')
    sink_process = sink.popen('iperf -s -p 5001 -e')
    waitListening(source, sink, 5001, timeout=5)
    sink_process.stdout.readline()
    result = source.cmd('iperf -t 10 -p 5001 -c {} -e'.format(sink.IP()))
    print(result)
    sink_process.send_signal(SIGINT)

    print('Raw bandwidth test (UDP)')
    sink_process = sink.popen('iperf -s -p 5001 -u -e')
    time.sleep(2)
    sink_process.stdout.readline()
    result = source.cmd('iperf -t 10 -b 10G -p 5001 -u -c {} -e'.format(sink.IP()))
    print(result)
    sink_process.send_signal(SIGINT)

def run_python_pub_sub(net, scenario_module, config):

    if (config == 1):
        print('\nConfiguration: UDP default')
    if (config == 2):
        print('\nConfiguration: UDP Reliable & Transient with KEEP_ALL history')
    if (config == 3):
        print('\nConfiguration: UDP Reliable & Transient with History')
    if (config == 4):
        print('\nConfiguration: TCP with UDP discovery')

    data_lines = []
    source, sink = pyutils.get_source_and_sink(net, scenario_module)

    sub_script = xml_dir + 'sub_processes.py ' + configs[config-1]
    command = "python3 " + sub_script
    sink_process = sink.popen(
        command
        )
    
    pub_script = xml_dir + 'pub_processes.py ' + configs[config-1]
    command = "python3 " + pub_script
    source_process = source.popen(
        command
        )

    for host, line in pmonitor({source: source_process, sink: sink_process}, timeoutms=200):
        if host:
            # print('\t{}-->{}:'.format(host, len(data_lines)), line)
            if host == sink and (line.startswith('[') or line.startswith('Total')):
                data_lines.append(line)
                print(len(data_lines), end=' ', flush=True)
                # print(data_lines[len(data_lines)-1], end='\n', flush=True)
        if len(data_lines) >= 200:
            print('\n200 msgs received')
            print('')
            break
    source_process.send_signal(SIGINT)
    sink_process.send_signal(SIGINT)
    print('Completed; accumulated data:')
    
    #Print lines
    if (len(data_lines) > 0):
        # print(data_lines[len(data_lines)-1], end='\n', flush=True)
        for l in data_lines:
            print(l.strip())
    else:
        print('[200]: Troughtput (MB/s):  0.0, Messages lost: 200')


def run_python_pub_sub_scenario11(net, scenario_module, config):

    if (config == 1):
        print('\nConfiguration: UDP default')
    if (config == 2):
        print('\nConfiguration: UDP Reliable & Transient with KEEP_ALL history')
    if (config == 3):
        print('\nConfiguration: UDP Reliable & Transient with History')
    if (config == 4):
        print('\nConfiguration: TCP with UDP discovery')

    data_lines = []

    print('Scenario 11 with 3 subscribers')
    source, sink1, sink2, sink3 = pyutils.get_source_and_sink_scenario11(net, scenario_module)
    sub_script = xml_dir + 'sub_processes.py ' + configs[config-1]
    command = "python3 " + sub_script
    sink1_process = sink1.popen(
        command
        )
    # sub_script = xml_dir + 'sub_processes_sub2.py ' + configs[config-1]
    command = "python3 " + sub_script
    sink2_process = sink2.popen(
        command
        )
    # sub_script = xml_dir + 'sub_processes_sub3.py ' + configs[config-1]
    command = "python3 " + sub_script
    sink3_process = sink3.popen(
        command
        )
    pub_script = xml_dir + 'pub_processes.py ' + configs[config-1]
    command = "python3 " + pub_script
    source_process = source.popen(
        command
        )

    for host, line in pmonitor({sink1: sink1_process, sink2: sink2_process, sink3: sink3_process}, timeoutms=200):
        if host:
            # print('\t{}-->{}:'.format(host, len(data_lines)), line)
            if (host == sink1) and (line.startswith('[') or line.startswith('Total')):
                data_lines.append('Sub1: ' + line)
                print(len(data_lines), end=' ', flush=True)
                # print(data_lines[len(data_lines)-1], end='\n', flush=True)
            if (host == sink2) and (line.startswith('[') or line.startswith('Total')):
                data_lines.append('Sub2: ' + line)
                print(len(data_lines), end=' ', flush=True)
                # print(data_lines[len(data_lines)-1], end='\n', flush=True)
            if (host == sink2) and (line.startswith('[') or line.startswith('Total')):
                data_lines.append('Sub3: ' + line)
                print(len(data_lines), end=' ', flush=True)
        if len(data_lines) >= 500:
            print('\n200 msgs received')
            print('')
            break
    source_process.send_signal(SIGINT)
    sink1_process.send_signal(SIGINT)
    sink2_process.send_signal(SIGINT)
    sink3_process.send_signal(SIGINT)
    print('Completed; accumulated data:')
    
    #Print lines
    if (len(data_lines) > 0):
        # print(data_lines[len(data_lines)-1], end='\n', flush=True)
        for l in data_lines:
            print(l.strip())
    else:
        print('[200]: Troughtput (MB/s):  0.0, Messages lost: 200')

def main():
    '''
    Run all configurations test for one single scenario
    '''
    if len(sys.argv) != 2:
        print('Please supply a scenario name')
        return 1
    
    clean_mn()
    scenario_name = sys.argv[1]
    scenario_module = importlib.import_module('.' + scenario_name, 'scenarios')
    topo = scenario_module.ScenarioTopo()
    net = Mininet(topo, link=TCLink)
    net.start()
    scenario_module.configure_network(net)
    load = scenario_module.start_network_load(net)

    dumpNodeConnections(net.hosts)
    path = '/tmp/tcp'

    if scenario_name == 'scenario11':
        ping_test_scenario11(net, scenario_module)
        tshark = pyutils.start_tshark_on_source_scenario11(net, scenario_module, path + '_large_data.pcap')
        for i in range(4):
            run_python_pub_sub_scenario11(net, scenario_module, i+1)
        # run_python_pub_sub_scenario11(net, scenario_module, 4)
        pyutils.stop_tshark(tshark)
    else:
        ping_test(net, scenario_module)
        print()
        # raw_bandwidth_test(net, scenario_module)
        tshark = pyutils.start_tshark_on_source(net, scenario_module, path + '_large_data.pcap')
        for i in range(4):
            run_python_pub_sub(net, scenario_module, i+1)
        # run_python_pub_sub(net, scenario_module, 4)
        pyutils.stop_tshark(tshark)

    scenario_module.stop_network_load(load)
    net.stop()

    return 0


if __name__ == '__main__':
    sys.exit(main())