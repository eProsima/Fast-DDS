from signal import SIGINT
import os
import re
import subprocess


def get_source_and_sink(net, module):
    return net.get(module.source_name, module.sink_name)

def get_source_and_sink_scenario11(net, module):
    return net.get(module.source_name, module.sink_names[0], module.sink_names[1], module.sink_names[2])


def start_tshark_native(capture_file_path, interface):
    return subprocess.Popen(
        ['tshark',
         '-i',
         interface,
         '-w',
         capture_file_path],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True
        )


def start_tshark_on(host, interface, capture_file_path):
    tshark_process = host.popen(
        'tshark -i {} -w {}'.format(interface, capture_file_path))
    return tshark_process


def start_tshark_on_source(net, scenario_module, capture_file_path):
    source, sink = get_source_and_sink(net, scenario_module)
    interface = scenario_module.get_capture_interface(
        net,
        scenario_module.source_name)
    return start_tshark_on(source, interface, capture_file_path)

def start_tshark_on_source_scenario11(net, scenario_module, capture_file_path):
    source, sink1, sink2, sink3 = get_source_and_sink_scenario11(net, scenario_module)
    interface = scenario_module.get_capture_interface(
        net,
        scenario_module.source_name)
    return start_tshark_on(source, interface, capture_file_path)


def start_tshark_on_sink(net, scenario_module, capture_file_path):
    source, sink = get_source_and_sink(net, scenario_module)
    interface = scenario_module.get_capture_interface(
        net,
        scenario_module.sink_name)
    return start_tshark_on(sink, interface, capture_file_path)


def stop_tshark(tshark):
    tshark.send_signal(SIGINT)


def count_zenoh_messages(
        title,
        pcap_file,
        tcp_ports=[],
        udp_ports=[],
        include=[],
        exclude=[]):
    tcp_port_dissectors = []
    for p in tcp_ports:
        tcp_port_dissectors.append('-d')
        tcp_port_dissectors.append('tcp.port=={},zenoh-tcp'.format(p))
    # print('TCP PORTS:')
    # print(tcp_port_dissectors)
    udp_port_dissectors = []
    for p in udp_ports:
        udp_port_dissectors.append('-d')
        udp_port_dissectors.append('udp.port=={},zenoh-udp'.format(p))
    # print('UDP PORTS')
    # print(udp_port_dissectors)
    if type(include) == str:
        include = [include]
    if type(exclude) == str:
        exclude = [exclude]
    print('INCLUDE')
    print(include)
    # print('EXCLUDE')
    # print(exclude)
    filter_pattern = ''
    for msgid in include:
        filter_pattern = filter_pattern + 'zenoh.msgid == {} && '.format(msgid)
    for msgid in exclude:
        filter_pattern = filter_pattern + 'zenoh.msgid != {} && '.format(msgid)
    filter_pattern = filter_pattern[:-4]
    if len(filter_pattern) > 0:
        filters = ['-2', '-R', filter_pattern]
    else:
        filters = []
    print('FILTER')
    print(filters)
    proc = subprocess.run(
        ['tshark',
         '-d', 'tcp.port==7500,zenoh-tcp',
         '-d', 'udp.port==7447,zenoh-udp'] +
        tcp_port_dissectors +
        udp_port_dissectors +
        filters +
        ['-r', pcap_file,
         '-w', '/tmp/zenoh_filtered.pcap',
         '-q',
         '-z', 'io,stat,0'],
        stdout=subprocess.PIPE,
        #stderr=subprocess.STDOUT,
        text=True
        )
    print('Count of {} messages:'.format(title))
    print(proc.stdout)
    os.remove('/tmp/zenoh_filtered.pcap')

def process_zenoh_packet_capture(capture_file_path, robot_count):
    # Calculate the TCP port numbers that will be used
    tcp_ports = []
    # for robot_number in range(0, robot_count):
    #     for port_number in range(
    #             7501 + robot_number * 50,
    #             7522 + robot_number * 50):
    #         tcp_ports.append(port_number)
    tcp_ports.append(7502)
    tcp_port_matchers = ''
    for p in tcp_ports:
        tcp_port_matchers += ' || tcp.port == {}'.format(p)
    # Find the randomly-chosen UDP ports
    proc = subprocess.run(
        ['tshark',
         '-2',
         '-R', 'udp.dstport == 7447',
         '-r', capture_file_path],
        stdout=subprocess.PIPE,
        text=True
        )
    udp_ports = []
    for l in proc.stdout.split('\n'):
        if not l:
            continue
        m = re.match(r'\d+\s\d+(.\d+)?\s+(\d+\.\d+\.\d+\.\d+)\s.+\s(\d+\.\d+\.\d+\.\d+)\s+ZENOH\s\d+\s(?P<port>\d{4,5})\s.+\s7447', l.strip())
        if m:
            port_number = int(m.group('port'))
            if port_number not in udp_ports:
                udp_ports.append(port_number)
    print()
    print('Found {} randomly-assigned UDP ports:\n\t{}'.format(
        len(udp_ports),
        udp_ports))
    udp_port_matchers = ''
    for p in udp_ports:
        udp_port_matchers += ' || udp.port == {}'.format(p)
    # Filter out known non-zenoh packets
    proc = subprocess.run(
        ['tshark',
         '-2',
         '-R', 'udp.port == 7447 || tcp.port == 7500'
               + tcp_port_matchers
               + udp_port_matchers,
         '-r', capture_file_path,
         '-w', '/tmp/filtered.pcap',
         '-q',
         '-z', 'io,stat,0'],
        stdout=subprocess.PIPE,
        #stderr=subprocess.STDOUT,
        text=True
        )
    print()
    print('Results of initial filter')
    print(proc.stdout)
    print()
    # Count SCOUT (0x01) messages
    count_zenoh_messages(
        'SCOUT',
        '/tmp/filtered.pcap',
        tcp_ports=tcp_ports,
        udp_ports=udp_ports,
        include='0x01')
    # Count HELLO (0x02) messages
    count_zenoh_messages(
        'HELLO',
        '/tmp/filtered.pcap',
        tcp_ports=tcp_ports,
        udp_ports=udp_ports,
        include='0x02')
    # Count INIT (0x03) messages
    count_zenoh_messages(
        'INIT',
        '/tmp/filtered.pcap',
        tcp_ports=tcp_ports,
        udp_ports=udp_ports,
        include='0x03')
    # Count OPEN (0x04) messages
    count_zenoh_messages(
        'OPEN',
        '/tmp/filtered.pcap',
        tcp_ports=tcp_ports,
        udp_ports=udp_ports,
        include='0x04')
    # Count KEEPALIVE (0x08) messages
    count_zenoh_messages(
        'KEEPALIVE',
        '/tmp/filtered.pcap',
        tcp_ports=tcp_ports,
        udp_ports=udp_ports,
        include='0x08')
    # Count LINKSTATELIST (0x10) messages
    count_zenoh_messages(
        'LINKSTATELIST',
        '/tmp/filtered.pcap',
        tcp_ports=tcp_ports,
        udp_ports=udp_ports,
        include='0x10')
    # Count DECLARE (0x0B) messages
    count_zenoh_messages(
        'DECLARE',
        '/tmp/filtered.pcap',
        tcp_ports=tcp_ports,
        udp_ports=udp_ports,
        include='0x0b')
    # Count DATA (0x0C) messages
    count_zenoh_messages(
        'DATA',
        '/tmp/filtered.pcap',
        tcp_ports=tcp_ports,
        udp_ports=udp_ports,
        include='0x0c')
    # Count other messages
    count_zenoh_messages(
        'other (including TCP ACKs)',
        '/tmp/filtered.pcap',
        tcp_ports=tcp_ports,
        udp_ports=udp_ports,
        exclude=['0x01', '0x03', '0x04', '0x08', '0x10', '0x0b', '0x0c'])

    os.remove('/tmp/filtered.pcap')


def count_dds_messages(title, message_type, pcap_file):
    filters = ['-2', '-R', message_type]
    proc = subprocess.run(
        ['tshark'] +
         filters +
        ['-r', pcap_file,
         '-w', '/tmp/dds_filtered.pcap',
         '-q',
         '-z', 'io,stat,0'],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True
        )
    print('Count of {} messages:'.format(title))
    print(proc.stdout)
    os.remove('/tmp/dds_filtered.pcap')


def process_dds_packet_capture(capture_file_path, robot_count):
    # SPDP (Participant discovery)
    # SEDP (Entites discovery)

    proc = subprocess.run(
        ['tshark',
         '-2',
         '-R', 'rtps',
         '-r', capture_file_path,
         '-w', '/tmp/filtered.pcap',
         '-q',
         '-z', 'io,stat,0'],
        stdout=subprocess.PIPE,
        text=True
        )
    print()
    print('Results of initial filter')
    print(proc.stdout)
    print()

    # Count Pad messages
    count_dds_messages(
        'Pad',
        'rtps.sm.id == PAD',
        '/tmp/filtered.pcap')
    # Count AckNack messages
    count_dds_messages(
        'AckNack',
        'rtps.sm.id == ACKNACK || rtps.sm.id == NACK_FRAG',
        '/tmp/filtered.pcap')
    # Count Heartbeat messages
    count_dds_messages(
        'Heatbeat',
        'rtps.sm.id == HEARTBEAT || rtps.sm.id == HEARTBEAT_FRAG',
        '/tmp/filtered.pcap')
    # Count Gap messages
    count_dds_messages(
        'Gap',
        'rtps.sm.id == GAP',
        '/tmp/filtered.pcap')
    # Count InfoTimestamp messages
    count_dds_messages(
        'InfoTimestamp',
        'rtps.sm.id == INFO_TS',
        '/tmp/filtered.pcap')
    # Count InfoSource messages
    count_dds_messages(
        'InfoSource',
        'rtps.sm.id == INFO_SRC',
        '/tmp/filtered.pcap')
    # Count InfoReplyIp4 messages
    count_dds_messages(
        'InfoReplyIp4',
        'rtps.sm.id == INFO_REPLY_IP4',
        '/tmp/filtered.pcap')
    # Count InfoDestination messages
    count_dds_messages(
        'InfoDestination',
        'rtps.sm.id == INFO_DST',
        '/tmp/filtered.pcap')
    # Count InfoReply messages
    count_dds_messages(
        'InfoReply',
        'rtps.sm.id == INFO_REPLY',
        '/tmp/filtered.pcap')
    # Count DATA messages
    count_dds_messages(
        'Data',
        'rtps.sm.id == DATA || rtps.sm.id == DATA_FRAG',
        '/tmp/filtered.pcap')

    is_data_filter = '(rtps.sm.id == DATA or rtps.sm.id == DATA_FRAG)'
    is_not_data_filter = '(rtps.sm.id != DATA and rtps.sm.id != DATA_FRAG)'
    is_heartbeat_filter = '(rtps.sm.id == HEARTBEAT or rtps.sm.id == HEARTBEAT_FRAG)'
    is_acknack_filter = '(rtps.sm.id == ACKNACK or rtps.sm.id == NACK_FRAG)'
    entity_id_participant_filter = '(rtps.sm.rdEntityId == ENTITYID_BUILTIN_PARTICIPANT_READER or rtps.sm.wrEntityId == ENTITYID_BUILTIN_PARTICIPANT_WRITER)'
    not_entity_id_participant_filter = '(rtps.sm.rdEntityId != ENTITYID_BUILTIN_PARTICIPANT_READER and rtps.sm.wrEntityId != ENTITYID_BUILTIN_PARTICIPANT_WRITER)'
    entity_id_participant_msg_filter = '(rtps.sm.rdEntityId == ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER or rtps.sm.wrEntityId == ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER)'
    not_entity_id_participant_msg_filter = '(rtps.sm.rdEntityId != ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER and rtps.sm.wrEntityId != ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER)'
    entity_id_publication_filter = '(rtps.sm.rdEntityId == ENTITYID_BUILTIN_PUBLICATIONS_READER or rtps.sm.wrEntityId == ENTITYID_BUILTIN_PUBLICATIONS_WRITER)'
    not_entity_id_publication_filter = '(rtps.sm.rdEntityId != ENTITYID_BUILTIN_PUBLICATIONS_READER and rtps.sm.wrEntityId != ENTITYID_BUILTIN_PUBLICATIONS_WRITER)'
    entity_id_subscription_filter = '(rtps.sm.rdEntityId == ENTITYID_BUILTIN_SUBSCRIPTIONS_READER or rtps.sm.wrEntityId == ENTITYID_BUILTIN_SUBSCRIPTIONS_WRITER)'
    not_entity_id_subscription_filter = '(rtps.sm.rdEntityId != ENTITYID_BUILTIN_SUBSCRIPTIONS_READER and rtps.sm.wrEntityId != ENTITYID_BUILTIN_SUBSCRIPTIONS_WRITER)'
    # Count participant announcement messages
    count_dds_messages(
        'Participant announcements',
        is_data_filter + ' and ' + entity_id_participant_filter,
        '/tmp/filtered.pcap')
    count_dds_messages(
        'Participant announcements overhead',
        is_not_data_filter + ' and ' + entity_id_participant_filter,
        '/tmp/filtered.pcap')
    # Count participant message announcement messages
    count_dds_messages(
        'Participant message announcements',
        is_data_filter + ' and ' + entity_id_participant_msg_filter,
        '/tmp/filtered.pcap')
    count_dds_messages(
        'Participant message announcements overhead',
        is_not_data_filter + ' and ' + entity_id_participant_msg_filter,
        '/tmp/filtered.pcap')
    # Count publication announcement messages
    count_dds_messages(
        'Publication announcements',
        is_data_filter + ' and ' + entity_id_publication_filter,
        '/tmp/filtered.pcap')
    count_dds_messages(
        'Publication announcements overhead',
        is_not_data_filter + ' and ' + entity_id_publication_filter,
        '/tmp/filtered.pcap')
    # Count subscription announcement messages
    count_dds_messages(
        'Subscription announcements',
        is_data_filter + ' and ' + entity_id_subscription_filter,
        '/tmp/filtered.pcap')
    count_dds_messages(
        'Subscription announcements overhead',
        is_not_data_filter + ' and ' + entity_id_subscription_filter,
        '/tmp/filtered.pcap')
    # Count application data messages
    count_dds_messages(
        'Application data',
        is_data_filter + ' and ' +
            not_entity_id_participant_filter + ' and ' +
            not_entity_id_participant_msg_filter + ' and ' +
            not_entity_id_publication_filter + ' and ' +
            not_entity_id_subscription_filter,
        '/tmp/filtered.pcap')
    # Count application non-data messages
    count_dds_messages(
        'Application overhead',
        is_not_data_filter + ' and ' +
            not_entity_id_participant_filter + ' and ' +
            not_entity_id_participant_msg_filter + ' and ' +
            not_entity_id_publication_filter + ' and ' +
            not_entity_id_subscription_filter,
        '/tmp/filtered.pcap')

    # Count multicast messages
    count_dds_messages(
        'Multicast',
        'ip.dst >= 224.0.0.0',
        '/tmp/filtered.pcap')
    # Count unicast messages
    count_dds_messages(
        'Unicast',
        'ip.dst < 224.0.0.0',
        '/tmp/filtered.pcap')

    os.remove('/tmp/filtered.pcap')


def process_packet_capture(capture_file_path, robot_count, middleware_type):
    if middleware_type == 'zenoh':
        process_zenoh_packet_capture(capture_file_path, robot_count)
    elif middleware_type == 'dds':
        process_dds_packet_capture(capture_file_path, robot_count)
    else:
        raise('Unknown middleware type: {}'.format(middleware_type))


def print_interfaces(net, scenario_module):
    source, sink = get_source_and_sink(net, scenario_module)

    source_interface_info = source.cmd('ip a')
    print('Source interfaces')
    print(source_interface_info)

    sink_interface_info = sink.cmd('ip a')
    print('Sink interfaces')
    print(sink_interface_info)


def print_iptables_rules(net, scenario_module):
    source, sink = get_source_and_sink(net, scenario_module)

    source_rules = source.cmd('iptables -L -nv')
    print('Source iptables rules')
    print(source_rules)

    sink_rules = sink.cmd('iptables -L -nv')
    print('Sink iptables rules')
    print(sink_rules)
