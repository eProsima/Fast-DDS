# Throughput testing

This directory provides all you need for [measuring the network throughput](#throughput-measure) of Fast-DDS in your environment.

Throughput measure implies having at least two nodes: one with the publication role and the other one with the subscription role.
The publication node, besides publishing fixed-size data in a custom rate, is in charge of controlling the synchronization
with the subscription nodes and gathering the results. Both roles are provided by the same [utility](#usage) which can be
[compiled](#compilation) with Fast-DDS.

Also this directory provides a [python script](#python-launcher) which helps launching both nodes in the same system.

## Throughput measure

The test consists on sending during a period of time as much as possible bursts of samples separated by a fix *recovery*
time. While the test doesn't exceed the test time, the publication node will try in a loop to send a burst of nth samples and
sleep for the *recovery* time. After the test exceeds the test time, the publication node will gather all information
from the subscription nodes to show you at the end.

Our utility is able to execute at the same time several tests with different setups.
These are the elements you can configure:

- Size of the samples.
- Number of samples for each burst.
- Recovery time -- time of each break between sending a burst of samples and the next one.
- Test time -- maximum time the throughput test must be running.

At the end of the testing the utility will show a table with the test results.

```
Payload [Bytes],   Demand [sample/burst],   Recovery time [ms],    Sent [samples],   Publication time [us],   Publication sample rate [Sample/s],   Publication throughput [Mb/s],   Received [samples],   Lost [samples],   Subscription time [us],   Subscription sample rate [Sample/s],   Subscription throughput [Mb/s]
16,                30000,                   0,                     1830000,          1002619.284,             1825219.232,                          233.628,                         1830000,              0,                1002608.786,              1825238.344,                           233.631
32,                30000,                   0,                     1860000,          1015109.505,             1832314.634,                          469.073,                         1860000,              0,                1015109.223,              1832315.143,                           469.073
```

Each line of the table is an execution with a specific setup.
Columns shows next information:

* Payload [Bytes] -- Size the samples used in the test.
* Demand [sample/burst] -- Number of samples in each burst.
* Recovery time [ms] -- Break time between sending a burst of samples and the next one.
* Sent [samples] -- Total samples the publication node is able to send.
* Publication time [us] -- Total time the publication node was running the test.
* Publication sample rate [Sample/s] -- Samples rate in the publication side.
* Publication throughput [Mb/s] -- Throughput measure in the publication side.
* Received [samples] -- Number of samples received by the subscription node.
* Lost [samples] -- Number of samples lost during the testing.
* Subscription time [us] -- Total time the subscription was running the test.
* Subscription sample rate [Sample/s] -- Samples rate in the subscription side.
* Subscription throughput [Mb/s] -- Throughput measure in the subscription side.


## Compilation

You can enable the compilation of the utility using the CMake option `PERFORMANCE_TESTS`.
Following our [*Installation from sources* guide](https://fast-dds.docs.eprosima.com/en/latest/installation/sources/sources_linux.html)
 you may compile Fast-DDS and this utility using next command.

```
colcon build --cmake-args -DPERFORMANCE_TESTS=ON
```

You can find this utility executable in the building directory.

```
build/fastrtps/test/performance/throughput
├── CMakeFiles
├── cmake_install.cmake
├── CTestTestfile.cmake
├── Makefile
├── ThroughputTest  <=== Throughput test utility
└── xml
```

## Usage

The utility is able to have the publication role or the subscription role.
Also it is able to have both roles, useful for testing special Fast-DDS mechanisms such as *intraprocess communication*

```bash
# Run a publication node
$ ThroughputTest publisher

# Run a subscription node
$ ThroughputTest subscriber

# Run a node with both roles
$ ThroughputTest both
```

The utility offers several options:

**General options**

| Option                              | Description                                                                                                                                |
| -                                   | -                                                                                                                                          |
| --reliability=[reliable/besteffort] | Set the Reliability QoS of the DDS entity                                                                                                  |
| --domain \<domain_id>               | Set the DDS domain to be used. Default domain is a random one. If testing in separate processes, always set the domain using this argument |
| --data_sharing=[on/off]             | Explicitly enable/disable Data Sharing feature. Fast-DDS default is *auto*                                                                 |
| --data_load                         | Enables the use of Data Loans feature                                                                                                      |
| --shared_memory                     | Explicitly enable/disable Shared Memory transport. Fast-DDS default is *on*                                                                |
| --security=[true/false]             | Enable/disable DDS security                                                                                                                |
| --certs=\<directory>                | Directory with the certificates. Used when security is enable                                                                              |


**Publication options**

| Option                          | Description                                                                      |
| -                               | -                                                                                |
| --subscribers=\<number>         | Number of subscriber in the testing. Default is *1*                              |
| --time=\<seconds>               | Time the test must be running . Default is *5 seconds*                           |
| --recovery_time=\<milliseconds> | Break time between sending a burst and the next one. Default is *5 milliseconds* |
| --demand=\<number>              | Number of samples send in each burst. Default is *10000*                         |
| --msg_size=\<bytes>             | Size of each sample in bytes. Default is *1024 bytes*                            |

**Batch testing options**

These options are used to execute a batch of tests with different setups.

| Option                         | Description                                                                        |
| -                              | -                                                                                  |
| --recoveries_file=\<file>      | A CSV file with the recovery times.                                                |
| --file=\<file>                 | File containing the different demands                                              |

The CSV for recovery times has the format of all recovery times separated with `;` character.

```
5;10
```

This example will execute two tests, one with a recovery time of 5 milliseconds and the other one with a recovery time of 10 milliseconds.

The file with the demands has the format of each line the data size separated of all demands with a `;` character.
```
16;100;1000
32;100;1000
```

This examples will execute four tests: one sending bursts of 100 samples of 16 bytes, other test sending bursts of 1000 samples of 16 bytes,
, other seding bursts of 100 samples of 32 bytes and the last one sending bursts of 1000 samples of 32 bytes.


### Examples

**Testing throughput for best effort communications using UDP transport**

The setup will be:

- Size of each sample: 2 Mb
- Size of each burst: 100 samples
- Recovery time: 90 milliseconds
- Test time: 10 seconds

```bash
# Publication node
$ ThroughtputTest publisher --reliability=beteffort --domain 0 --shared_memory=off --time=10 --recovery_time=90 --demand=100 --msg_size=2097152

# Subscription node
$ ThroughtputTest subscriber --reliability=beteffort --domain 0 --shared_memory=off
```

## Python launcher

The directory also comes with a python script which automates the execution of the test nodes.

```batch
# Indicate where is the utility executable
export THROUGHPUT_TEST_BIN=build/fastrtps/test/performance/throughtput/ThroughputTest

# Call python script to run tests.
python3 src/fastrtps/test/performance/throughput/throughput_tests.py
```

The python scripts offers several options:

| Option                              | Description                                                                                                                                |
| -                                   | -                                                                                                                                          |
| --reliability                       | Set the Reliability QoS of the DDS entities to reliable. Default Reliability is best-effort                                                |
| --data_loasn                        | Enable the use of the loan sample API. Default is disable                                                                                  |
| --shared_memory [on/off]            | Explicitly enable/disable shared memory transport. Fast-DDS default is *on*                                                                |
| --interprocess                      | Publisher and subscriber in separate processes. Default is both in the sample process and using intraprocess communications                |
| --security                          | Enable security. Default disable                                                                                                           |
| -t \<seconds>                       | Test time in seconds. Default is *1 second*                                                                                                |
| -r <file>                           | A CSV file with recovery time                                                                                                              |
| -f <file>                           | A file containing the demands                                                                                                              |

