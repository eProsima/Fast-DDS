# Latency testing

This directory provides everything needed for [measuring the network latency](latency-measure) of Fast DDS.

Latency measure implies having at least two nodes: one with the publication role and the other one with the subscription role.
The publication node, besides publishing fixed-size data and wait for the reply, is in charge of controlling the synchronization
with the subscription nodes and gathering the results.
Both roles are provided by the same [utility](#usage) which can be [compiled](#compilation) with Fast DDS.

Also this directory provides a [Python script](#python-launcher) which helps launching both nodes in the same system.

## Latency measure

The test consists on sending samples and wait for their replies.
The publication node will try in a loop to send a sample, wait for the reply and calculate the time used in this process.
After sending the specified number of samples, the publication node will gather the information to show you at the end.

The latency test application is able to execute at the same time several tests with different setups.
The elements that can be configured are the following:

- Size of the samples.
- Number of samples to send.

At the end of the execution the utility will show a table with the test results.
Below is an example of these test results.

```
   Bytes, Samples,   stdev,    mean,     min,     50%,     90%,     99%,  99.99%,     max
--------,--------,--------,--------,--------,--------,--------,--------,--------,--------,
      16,   10000,   0.248,   1.279,   1.106,   1.263,   1.358,   2.509,   6.932,   7.261
    1024,   10000,   0.822,   1.678,   1.078,   1.145,   2.399,   2.538,  17.373,  17.862
   64512,   10000,   1.769,   5.641,   4.574,   4.744,   7.574,  12.189,  31.385,  45.567
 1048576,   10000,  20.211,  69.110,  58.913,  62.671,  82.916, 140.723, 447.954, 458.905
```

Each line of the table is an execution with a specific setup.
The columns show the next information:

* Bytes -- Size the samples used in the test.
* Demand -- Number of samples sent in the test.
* stdev -- Standard desviation
* mean -- Mean latency time in microseconds
* min -- Minimum latency time in microseconds
* 50% -- Lantency time in the 50% of all latencies
* 90% -- Lantency time in the 90% of all latencies
* 99% -- Lantency time in the 99% of all latencies
* 99.99% -- Lantency time in the 99.99% of all latencies
* max -- Maximum latency time in microseconds


## Compilation

This utility can be enabled by using the CMake option `PERFORMANCE_TESTS`.
Following the Fast DDS [*Installation from sources*
guide](https://fast-dds.docs.eprosima.com/en/latest/installation/sources/sources_linux.html),
Fast DDS and this utility can be compiled executing the next command.

```
colcon build --cmake-args -DPERFORMANCE_TESTS=ON
```

The latency test executable can be found in the `build` directory.

```
build/fastdds/test/performance/latency
├── CMakeFiles
├── cmake_install.cmake
├── CTestTestfile.cmake
├── Makefile
├── LatencyTest  <=== Latency test utility
└── xml
```

## Usage

The utility is able to have the publication role or the subscription role.
Also it is able to have both roles, useful for testing special Fast DDS mechanisms such as *intraprocess communication*

```bash
# Run a publication node
$ LatencyTest publisher

# Run a subscription node
$ LatencyTest subscriber

# Run a node with both roles
$ LatencyTest both
```

The utility offers several options:

**General options**

| Option                              | Description                                                                                                                                |
| -                                   | -                                                                                                                                          |
| --reliability=[reliable/besteffort] | Set the Reliability QoS of the DDS entity                                                                                                  |
| --samples=<number>                  | Number of samples sent in the test. Default is *10000 samples*                                                                             |
| --domain \<domain_id>               | Set the DDS domain to be used. Default domain is a random one. If testing in separate processes, always set the domain using this argument |
| --file=<file>                       | File to read the payload demands.                                                                                                          |
| --data_sharing=[on/off]             | Explicitly enable/disable Data Sharing feature. Fast DDS default is *auto*                                                                 |
| --data_load                         | Enables the use of Data Loans feature                                                                                                      |
| --shared_memory                     | Explicitly enable/disable Shared Memory transport. Fast DDS default is *on*                                                                |
| --security=[true/false]             | Enable/disable DDS security                                                                                                                |
| --certs=\<directory>                | Directory with the certificates. Used when security is enable                                                                              |


**Publication options**

| Option                          | Description                                                                      |
| -                               | -                                                                                |
| --subscribers=\<number>         | Number of subscriber in the testing. Default is *1*                              |

**Subscription options**

| Option                          | Description                                                                      |
| -                               | -                                                                                |
| --echo[true/false]              | Enable/disable echo mode. Default is *true*                                      |


The file with the demands has the format of all data sizes to be used separated by the `;` character.
```
16;32;64
```

This examples will execute three tests: one testing latency for samples of 16 bytes, other testing latency for samples
of 32 bytes the last one testing latency for samples of 64 bytes.


### Examples

**Testing latency for best effort communications using UDP transport**

The setup will be:

- Size of each sample: 2 Mb
- Size of each burst: 100 samples
- Recovery time: 90 milliseconds
- Test time: 10 seconds

The CSV file with demands will have this content: `2097152``

```bash
# Publication node
$ LatenchTest publisher --reliability=besteffort --domain 0 --shared_memory=off --file=demands.csv

# Subscription node
$ LatenchTest subscriber --reliability=besteffort --domain 0 --shared_memory=off --file=demands.csv
```

## Python launcher

The directory also comes with a Python script which automates the execution of the test nodes.

```batch
# Indicate where is the utility executable
export LATENCY_TEST_BIN=build/fastdds/test/performance/latency/LatencyTest

# Call python script to run tests.
python3 src/fastdds/test/performance/latency/latency_tests.py
```

The python scripts offers several options:

| Option                              | Description                                                                                                                                |
| -                                   | -                                                                                                                                          |
| --reliability                       | Set the Reliability QoS of the DDS entities to reliable. Default Reliability is best-effort                                                |
| --data_loans                        | Enable the use of the loan sample API. Default is disable                                                                                  |
| --shared_memory [on/off]            | Explicitly enable/disable shared memory transport. Fast DDS default is *on*                                                                |
| --interprocess                      | Publisher and subscriber in separate processes. Default is both in the sample process and using intraprocess communications                |
| --security                          | Enable security. Default disable                                                                                                           |
| -n \<number>                        | Number of samples sent in the test. Default is *10000 samples*
| -f \<file>                          | A file containing the demands                                                                                                              |
