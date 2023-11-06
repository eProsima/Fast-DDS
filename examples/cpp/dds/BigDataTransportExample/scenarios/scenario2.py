#!/usr/bin/python3

from mininet.topo import Topo
from signal import SIGINT


source_name = 'w1'
sink_name = 'h1'


class ScenarioTopo(Topo):
    """
    Topology for scenario 2:
    - One switch
    - One access point, connected to the switch (emulated with a switch)
    - Three stations on the wifi (the robot, a tablet watching YouTube, a TV
      watching Netflix)
    - One host (the workstation) connected directly to the switch
    """
    def build(self):
        # Performance limits for hosts
        # Robot has a Raspberry Pi 4 at 1.5 GHz and 4 cores
        # Workstation is a 2 GHz, 4-core laptop
        # Tablet is a 1 GHz, 4-core CPU.
        # Television is a 1 GHz, single-core CPU.

        # Devices
        switch = self.addSwitch('s1')
        access_point = self.addSwitch('a1')
        workstation = self.addHost('h1')
        robot = self.addHost('w1', ip='10.0.0.2/24')
        internet = self.addHost('h2')
        tablet = self.addHost('w2',)
        tv = self.addHost('w3')

        # Performance parameters for links
        # Assume 1 Gbps connection between workstation and switch
        # Wi-Fi is Wi-Fi 5 (802.11ac)
        # Robot has a 802.11n connection using the 5 GHz band, with 60 Mbps of
        #   bandwidth available, and a 0.14% packet loss rate (empirically
        #   measured)
        # Tablet and TV have 802.11ac connections in the 5 GHz band with
        #   available bandwidth of 866.7 Mbps
        # All Wi-Fi devices have a 2ms latency

        # Links
        self.addLink(switch, workstation, bw=1000)
        self.addLink(switch, internet, bw=100)
        # Emulate the maximum throughput of the wireless by limiting the link
        # between the switch and the access point
        self.addLink(switch, access_point, bw=866.7)
        self.addLink(access_point, robot, bw=60, delay='2ms', loss=0.14)
        self.addLink(access_point, tablet, bw=866.7, delay='2ms')
        self.addLink(access_point, tv, bw=866.7, delay='2ms')


def configure_network(net):
    pass


def start_network_load(net):
    tablet, tv, internet = net.get('w2', 'w3', 'h2')

    internet_traffic_sink_1 = internet.popen('iperf -s -u -p 5001')
    internet_traffic_sink_2 = internet.popen('iperf -s -u -p 5002')

    tablet_traffic_source = tablet.popen('iperf -c {} -p 5001 -u -b 1m'.format(internet.IP()))
    tv_traffic_source = tv.popen('iperf -c {} -p 5002 -u -b 25m'.format(internet.IP()))

    return tablet_traffic_source, tv_traffic_source, internet_traffic_sink_1, internet_traffic_sink_2


def stop_network_load(processes):
    for p in processes:
        p.send_signal(SIGINT)


def get_capture_interface(net, host_name):
    return 'any'
