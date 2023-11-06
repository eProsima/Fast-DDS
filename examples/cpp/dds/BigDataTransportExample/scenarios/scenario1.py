#!/usr/bin/python3

from mininet.topo import Topo
from signal import SIGINT


source_name = 'h1'
sink_name = 'h1'


def get_cpu_fraction(target_frequency):
    host_frequency = 3400.0
    return target_frequency / host_frequency


class ScenarioTopo(Topo):
    """
    Topology for scenario 1:
    - One switch
    - One host (the workstation) connected directly to the switch
    """
    def build(self):
        # Performance limits for hosts
        # Workstation is a common PC

        # Devices
        switch = self.addSwitch('s1')
        workstation = self.addHost('h1', ip='10.0.0.2/24')

        # Performance parameters for links
        # Assume 1 Gbps connection between workstation and switch

        # Links
        self.addLink(switch, workstation, bw=1000)


def configure_network(net):
    pass


def start_network_load(net):
    return []


def stop_network_load(processes):
    for p in processes:
        p.send_signal(SIGINT)


def get_capture_interface(net, host):
    return 'any'
