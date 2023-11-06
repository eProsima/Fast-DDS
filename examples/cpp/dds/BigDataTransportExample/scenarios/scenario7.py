#!/usr/bin/python3

from mininet.topo import Topo
from signal import SIGINT


source_name = 'r1'
sink_name = 'h1'

class ScenarioTopo(Topo):
    """
    Topology for scenario 7:
    - One switch
    - One access point, connected to the switch (emulated with a switch)
    - One host (the GPU cluster) connected directly to the switch
    - One robot connected to the Wifi

    """
    def build(self):
        # Performance limits for hosts
        # The robot has a 3 GHz, 4-core CPU
        # The GPU cluster has effectively unlimited performance

        # Devices
        switch = self.addSwitch('s1')
        access_point = self.addSwitch('a1')
        gpu_cluster = self.addHost('h1')
        target_robot = self.addHost('r1', ip='10.0.0.2/24')

        # Performance parameters for links
        # 10 Gbps connection between the GPU cluster and the switch
        # Wi-Fi is Wi-Fi 5 (802.11ac)
        # The robot has a 802.11n connection using the 5 GHz band, with 400
        #   Mbps of bandwidth available, and a 0.14% packet loss rate

        # Links
        self.addLink(switch, gpu_cluster, bw=10000)
        # Emulate the maximum throughput of the wireless by limiting the link
        # between the switch and the access point
        self.addLink(switch, access_point, bw=866.7)
        # Robot link
        self.addLink(access_point, target_robot, bw=400, delay='2ms', loss=0.14)


def configure_network(net):
    pass


def start_network_load(net):
    return []


def stop_network_load(processes):
    for p in processes:
        p.send_signal(SIGINT)


def get_capture_interface(net, host_name):
    return 'any'
