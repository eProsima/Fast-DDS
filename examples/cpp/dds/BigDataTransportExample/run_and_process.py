
from signal import SIGINT
import os.path
import sys
import subprocess
import pandas as pd
import numpy as np
import re
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

def run_scenario(scenario):
    """
    Run bandwidth test.

    :param scenario: Scenario to be tested

    :return: Process stdout
    """
    print('\nRESULT OF: ' + scenario)
    result = subprocess.run(
        ['sudo',
         'python3',
         'mininet_tests.py',
         scenario],
         stdout=subprocess.PIPE,
         text=True,
         universal_newlines=True
         )
    print(result.stdout)
    open('/tmp/' + scenario + '_bigdata_tests.txt', "w").write(result.stdout)

    return result.stdout

def run_packet_count(scenario):
    """
    Run packet count test using tshark.

    :param scenario: Scenario to be tested

    :return: Process stdout
    """
    print('\nPacket count of', scenario)
    result = subprocess.run(
        ['python3',
         'process_count_packets.py',
         scenario],
         stdout=subprocess.PIPE,
         text=True,
         universal_newlines=True
    )
    print(result.stdout)
    open('/tmp/' + scenario + '_packet_count.txt', "w").write(result.stdout)

    return result.stdout

def check_units_seconds(string):
    """
    Set time units to milliseconds.

    :param string: unit to be standarized

    :return: time in milliseconds
    """
    milli = re.search(r'ms', string)
    micro = re.search(r'µs', string)
    nano = re.search(r'ns', string)

    v = re.sub(r'[a-zA-Zµ]+$', '', string)

    # Use milliseconds as standard unit
    if milli:
        return float(v)
    elif micro:
        return float(v) / 1000
    elif nano:
        return float(v) / 10e6
    else:
        return float(v) * 1000

def check_units_bits(match):
    """
    Set Bits units to Megabits.

    :param string: unit to be standarized

    :return: Bits in Megabits
    """
    Gunit = re.search(r'Gbits', match.group(0))
    Munit = re.search(r'Mbits', match.group(0))

    # Use Megabits as standard unit
    if Gunit:
        return float(match.group(1)) * 1000
    elif Munit:
        return float(match.group(1))
    else:
        return float(match.group(1)) / 1000
    
def obtain_stats(df):
    """
    Obtain stats of Bandwidth (Mbps).

    :param df: DataFrame containing the Bandwidth (Mbps) to be analyzed

    :return: Numpy array with the correspoding stats
    """
    return np.array([df['Bandwidth (Mbps)'].mean(),
                     df['Bandwidth (Mbps)'].median(),
                     df['Bandwidth (Mbps)'].std(),
                     df['Bandwidth (Mbps)'].min(),
                     df['Bandwidth (Mbps)'].quantile(0.25),
                     df['Bandwidth (Mbps)'].quantile(0.75),
                     df['Bandwidth (Mbps)'].max(),
                     ])

def order_and_totals_dds(a):
    """
    Order and group DDS' messages according their type.

    :param a: DataFrame containing count of messages sent

    :return: Ordered Numpy array with totals
    """
    r = np.empty(0)

    disc_total = a[11] + a[12] + np.sum(a[15:19])
    app_total = a[19] + a[20]
    other_total = a[2] + a[3]
    disc_and_app = disc_total + app_total

    r = np.append(r, [a[0], a[2], a[3]])
    r = np.append(r, a[10:21])
    r = np.append(r, [disc_total, app_total, other_total])
    r = np.append(r, [disc_total * 100 / disc_and_app, app_total * 100 / disc_and_app])
    r = np.append(r, [a[22], a[21]])
                      
    return r

def totals_zenoh(a):
    """
    Order and group Zenoh messages according their type.

    :param a: DataFrame containing count of messages sent

    :return: Ordered Numpy array with totals
    """
    r = np.empty(0)

    disc_total = np.sum(a[1:5]) + a[7]
    other_total = np.sum(a[5:7]) + a[8] + a[9]
    total = disc_total + other_total

    r = np.append(r, a)
    r = np.append(r, [disc_total, other_total])
    r = np.append(r, [disc_total * 100 / total, other_total * 100 / total])

    return r

def process_data(scenario):
    """
    Run tests and process data returned.

    :param scenario: Scenario to be used

    :return: Dataframes containing iPerf, Zenoh and DDS Throughput, Zenoh packet count and DDS packet count 
    """
    
    output = run_scenario(scenario)
    # with open('/home/carlos/copy_data_tests/data_tests/scenario1.txt') as file:
    #     output = file.read()
    return
    
    lines = output.split('\n')

    # Iperf Throughput
    bw_regex = r'Bytes\s+(\d+(\.\d+)?)\s+([\w/]+)'
    iperf_array = np.empty(0)

    for line in lines:
        if line.startswith('[  1] 0.0000'):
            match = re.search(bw_regex, line)
            if  match:
                bw_data = check_units_bits(match)
                # Store data in array
                iperf_array = np.append(iperf_array, bw_data)
        if line.startswith('[  1] Server Report:'):
            # Delete last element
            iperf_array = iperf_array[:-1]

    iperf_df = pd.DataFrame(iperf_array, columns = [scenario], index = ['TCP', 'UDP'])

    # Zenoh and DDS Throughput
    zenoh_flag = 0
    dds_flag = 0

    zenoh_regex = r'(\d+\.\d+)\,(\d+\.\d+\w+)\,(\d+)\,(\d+\.\d+)\,(\d+\.\d+)\,(\d+\.\d+)'
    zenoh_array = np.empty((0, 6))

    dds_regex = r'(\d+\.\d+)\,(\d+)\,(\d+)\,(\d+\.\d+)\,(\d+\.\d+)\,(\d+\.\d+)'
    dds_array = np.empty((0, 6))

    for line in lines: 
        if line.startswith('Zenoh bandwidth test'):
            zenoh_flag = 1
            dds_flag = 0
            continue
        if line.startswith('FastDDS bandwidth test'):
            zenoh_flag = 0
            dds_flag = 1
            continue
        if zenoh_flag:
            row = np.empty(0)
            match = re.search(zenoh_regex, line)
            if match:
                for i in range(1, 7):
                    if (i == 2):
                        v = check_units_seconds(match.group(i))
                    else:
                        v = float(match.group(i))
                    row = np.append(row, v)
                zenoh_array = np.vstack((zenoh_array, row))
        if dds_flag:
            row = np.empty(0)
            match = re.search(dds_regex, line)
            if match:
                for i in range(1, 7):
                    v = float(match.group(i))
                    row = np.append(row, v)
                dds_array = np.vstack((dds_array, row))

    headers = ["Received at", "Transmission time (ms)", "Transmitted (Bytes)", "Rate (B/s)", "Bandwidth (bps)", "Bandwidth (Mbps)"]
    zenoh_df = pd.DataFrame(zenoh_array, columns = headers)
    dds_df = pd.DataFrame(dds_array, columns = headers)

    # Packet count
    output_packets = run_packet_count(scenario)
    # with open('/home/carlos/copy_data_tests/data_tests/count_packets1.txt') as file:
    #     output_packets = file.read()

    lines = output_packets.split('\n')

    packet_regex = r'\|\s+(\d+\.\d+)\s+\<\>\s+(\d+\.\d+)\s+[\|\s+\d+\s+\|]+'
    zenoh_packet_array = np.empty(0)
    dds_packet_array = np.empty(0)

    zenoh_flag = 1
    dds_flag = 0

    for line in lines:
        if line.startswith('Process DDS packets'):
            zenoh_flag = 0
            dds_flag = 1
        match = re.search(packet_regex, line)
        if match:
            if zenoh_flag:
                numbers = match.group(0).split('|')
                zenoh_packet_array = np.append(zenoh_packet_array, int(numbers[2]))
            if dds_flag:
                numbers = match.group(0).split('|')
                dds_packet_array = np.append(dds_packet_array, int(numbers[2]))

    # Reorder count array and compute totals
    packet_zenoh_df = pd.DataFrame(totals_zenoh(zenoh_packet_array), columns = [scenario])
    packet_dds_df = pd.DataFrame(order_and_totals_dds(dds_packet_array), columns = [scenario])

    return (iperf_df, zenoh_df, dds_df, packet_zenoh_df, packet_dds_df)

def main():

    if len(sys.argv) < 2:
        print('No scenarios provided, using default: 1, 2, 3, 5, 7')
        scenarios = ['1', '2', '3', '5', '7']
    else: 
        scenarios = sys.argv[1:]

    print('Running bandwidth test for scenarios:', scenarios)

    for s in scenarios: 
        process_data('scenario' + s)

    return 0

    rows_headers_data = ['Mean',
                         'Median',
                         'Standard deviation',
                         'Minimum',
                         '1st quartile',
                         '3rd quartile',
                         'Maximum']

    rows_headers_zenoh = ['Total',
                          'Scout',
                          'Hello',
                          'Init',
                          'Open',
                          'KeepAlive',
                          'LinkStateList',
                          'Declare',
                          'Data',
                          'Other(inc. ACKs)',
                          'Discovery total',
                          'Other total',
                          'Discovery (%)',
                          'Other (%)']
    
    rows_headers_dds = ['Total',
                        'AckNack',
                        'Heartbeat',
                        'Data (any kind)',
                        'Participant announcement data',
                        'Participant announcement overhead',
                        'Participant message announcement data',
                        'Participant message announcement overhead',
                        'Publication announcement data',
                        'Publication announcement overhead',
                        'Subscription announcement data',
                        'Subscription announcement overhead',
                        'Application data',
                        'Application overhead',
                        'Discovery total',
                        'Application total',
                        'Other total',
                        'Discovery (%)',
                        'Application (%)',
                        'Unicast',
                        'Multicast']

    iperf_df_global = pd.DataFrame()
    zenoh_stats_df = pd.DataFrame(rows_headers_data, columns = ["Stats"])
    zenoh_data_df = pd.DataFrame()
    dds_stats_df = pd.DataFrame(rows_headers_data, columns = ["Stats"])
    dds_data_df = pd.DataFrame()
    zenoh_packet_global = pd.DataFrame(rows_headers_zenoh, columns = ["Message"])
    dds_packet_global = pd.DataFrame(rows_headers_dds, columns = ["Message"])

    for s in scenarios:
        [iperf_df, zenoh_df, dds_df, packet_zenoh_df, packet_dds_df] = process_data('scenario' + s)
        iperf_df_global = pd.concat([iperf_df_global, iperf_df], axis = 1)
        zenoh_data_df = pd.concat([zenoh_data_df, zenoh_df['Bandwidth (Mbps)']], axis = 1)
        zenoh_data_df.rename(columns={"Bandwidth (Mbps)": "Scenario" + s}, inplace=True)
        zenoh_stats_df = zenoh_stats_df.join(pd.DataFrame(obtain_stats(zenoh_df), columns = ['Scenario' + s]))
        dds_data_df = pd.concat([dds_data_df, dds_df['Bandwidth (Mbps)']], axis = 1)
        dds_data_df.rename(columns={"Bandwidth (Mbps)": "Scenario" + s}, inplace=True)
        dds_stats_df = dds_stats_df.join(pd.DataFrame(obtain_stats(dds_df), columns = ['Scenario' + s]))
        zenoh_packet_global = pd.concat([zenoh_packet_global, packet_zenoh_df], axis = 1)
        dds_packet_global = pd.concat([dds_packet_global, packet_dds_df], axis = 1)

    zenoh_stats_df.set_index('Stats', inplace=True)
    dds_stats_df.set_index('Stats', inplace=True)
    zenoh_packet_global.set_index('Message', inplace=True)
    dds_packet_global.set_index('Message', inplace=True)

    print(iperf_df_global.transpose())
    print(zenoh_data_df)
    print(zenoh_stats_df)
    print(dds_data_df)
    print(dds_stats_df)
    print(zenoh_packet_global)
    print(dds_packet_global)

    # Plot and save to pdf
    with PdfPages('zenoh_vs_dds_graphs.pdf') as pdf:
        iperf_df_global.transpose().plot.bar(title = 'iPerf Throughput', rot = 0)
        plt.xlabel('Scenarios')
        plt.ylabel('Mbps')
        plt.legend(loc = 'best')
        pdf.savefig()
        plt.close()

        zenoh_data_df.plot.box(title = 'Zenoh Throughput')
        plt.xlabel('Scenarios')
        plt.ylabel('Mbps')
        pdf.savefig()
        plt.close()

        dds_data_df.plot.box(title = 'DDS Throughput')
        plt.xlabel('Scenarios')
        plt.ylabel('Mbps')
        pdf.savefig()
        plt.close()

        zenoh_packet_global.transpose()[['Discovery total', 'Other total']].plot.bar(title = 'Zenoh Packet counts (absolute)', stacked = True, rot = 0)
        plt.xlabel('Scenarios')
        plt.ylabel('Messages')
        pdf.savefig()
        plt.close()

        zenoh_packet_global.transpose()[['Discovery (%)', 'Other (%)']].plot.bar(title = 'Zenoh Packet counts (relative)', stacked = True, rot=0)
        plt.xlabel('Scenarios')
        plt.ylabel('Messages (%)')
        pdf.savefig()
        plt.close()

        dds_packet_global.transpose()[['Discovery total', 'Application total']].plot.bar(title = 'DDS Packet counts (absolute)', stacked = True, rot = 0)
        plt.xlabel('Scenarios')
        plt.ylabel('Messages')
        pdf.savefig()
        plt.close()

        dds_packet_global.transpose()[['Discovery (%)', 'Application (%)']].plot.bar(title = 'DDS Packet counts (relative)', stacked = True, rot = 0)
        plt.xlabel('Scenarios')
        plt.ylabel('Messages (%)')
        pdf.savefig()
        plt.close()

    return 0

if __name__ == '__main__':
    sys.exit(main())
