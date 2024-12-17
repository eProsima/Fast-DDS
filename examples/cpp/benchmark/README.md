# Benchmark example

The *eProsima Fast DDS benchmark* example is a simple application intended to demonstrate a basic DDS deployment.

This example is part of the suite of examples designed by eProsima that aims to illustrate the features and possible configurations of DDS deployments through *eProsima Fast DDS*.

In this case, the *benchmark* example allows to select between different messages sizes and configurations to show how the DDS transport is carried and how many package are sent in each case.

* [Description of the example](#description-of-the-example)
* [Run the example](#run-the-example)
* [Configuration](#configuration)
* [XML profile playground](#xml-profile-playground)

## Description of the example

Each example application (publisher and subscriber) creates the required DDS entities per case.
Both publisher and subscriber inherit from the corresponding listener class, overriding the listener's method associated to each event.
When an event occurs, the callback method is triggered.

If the environment does not specify the expected configuration, they take the default configuration per entity.

Moreover, this example extends the configuration options and message type size to showcase the results on the amount of messages transported during each sample time of the whole example duration for these conditions.

## Run the example

To launch this example, two different terminals are required.
One of them will run the publisher example application, and the other will run the subscriber application.

### Benchmark publisher

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./benchmark publisher
    Publisher running for 10000 milliseconds. Please press Ctrl+C to stop the Publisher at any time.
    ```

* Windows

    ```powershell
    example_path> benchmark.exe publisher
    Publisher running for 10000 milliseconds. Please press Ctrl+C to stop the Publisher at any time.
    ```

### Benchmark subscriber

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./benchmark subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

* Windows

    ```powershell
    example_path> benchmark.exe subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

All the example available flags can be queried running the executable with the ``-h`` or ``--help`` flag.

### Expected output

Regardless of which application is run first, since the publisher will not start sending data until a subscriber is discovered, the expected output both for publishers and subscribers is a first displayed message acknowledging the match, followed by the sent and received data in each application, and finishing showing in the publisher the amount of data shared in each sample time during the running time with additional throughput data.

### Benchmark publisher

```shell
Publisher running for 500 samples. Please press Ctrl+C to stop the Publisher at any time.
Subscriber matched.
Publisher matched.
First Sample with index: '0' (Array  8388608 Bytes) SENT
Sample with index: '1' (Array  8388608 Bytes) RECEIVED
Sample with index: '2' (Array  8388608 Bytes) SENT
Sample with index: '3' (Array  8388608 Bytes) RECEIVED
Sample with index: '4' (Array  8388608 Bytes) SENT
Sample with index: '5' (Array  8388608 Bytes) RECEIVED
...
Sample with index: '499' (Array  8388608 Bytes) RECEIVED
Sample with index: '500' (Array  8388608 Bytes) SENT
Publisher unmatched.
Subscriber unmatched.
RESULTS after 1206 milliseconds:
COUNT: 500
SAMPLES: 41,46,44,42,44,42,52,46,48,44,48,4,
THROUGHPUT BPS(Bytes per Second): 3.48482 Gbps

...
```

### Benchmark subscriber

```shell
Publisher matched.
Subscriber matched.
Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
Sample with index: '0' (Array  8388608 Bytes) RECEIVED
Sample with index: '1' (Array  8388608 Bytes) SENT
Sample with index: '2' (Array  8388608 Bytes) RECEIVED
Sample with index: '3' (Array  8388608 Bytes) SENT
Sample with index: '4' (Array  8388608 Bytes) RECEIVED
Sample with index: '5' (Array  8388608 Bytes) SENT
...
Sample with index: '499' (Array  8388608 Bytes) SENT
Sample with index: '500' (Array  8388608 Bytes) RECEIVED
...
```

When Ctrl+C is pressed to stop one of the applications, the other one will show the unmatched status, displaying an informative message, and it will stop sending / receiving messages.
The following is a possible output of the publisher application when stopping the subscriber app.

```shell
...
Publisher running for 500 samples. Please press Ctrl+C to stop the Publisher at any time.
Subscriber matched.
Publisher matched.
First Sample with index: '0' (Array  8388608 Bytes) SENT
Sample with index: '1' (Array  8388608 Bytes) RECEIVED
Sample with index: '2' (Array  8388608 Bytes) SENT
Sample with index: '3' (Array  8388608 Bytes) RECEIVED
Sample with index: '4' (Array  8388608 Bytes) SENT
Publisher unmatched.
Subscriber unmatched.
```

## Configuration
Ahead are described the different configuration that can be selected by argument in the terminal.

### Domain ID

Using argument **`-d`** `<num>` or **`--domain`** `<num>` configures the Domain ID for the application.
The Domain ID ensures communication isolation between different groups of participants.

- **Range**: `[0 <= <num> <= 232]`
- **Default**: `0`

### Topic Name

Using argument **`-n`** `<str>` or **`--name`** `<str>` configures the custom name of the topic used by the publisher and subscriber.
This parameter allows flexibility in naming the topic, making it easier to identify or organize multiple test cases running simultaneously.

- **Default**: `benchmark_topic`


### Reliability QoS

Using argument **``-r``** or **``--reliable``** will configure the corresponding endpoint with **``RELIABLE``** reliability QoS.
If the argument is not provided, by default is configured as **``BEST_EFFORT``**.

Whereas **``BEST_EFFORT``** do not retransmit missing samples, **``RELIABLE``** expects an arrival confirmation sample (acknowledge sample or ACK) per sample sent.

**Note**: **``RELIABLE``** option may block the write operation if certain scenarios are met ([resource limits](#resource-limits-qos) reached for instance).

Moreover, there is a compatibility rule between data readers and data writers, where the reliability QoS kind is checked to ensure the expected behavior.
The following table represents the compatibility matrix (compatible ✔️ vs incompatible ✖️):

<table>
  <tr style="text-align:center">
    <td colspan="2" rowspan="2"></td>
    <th colspan="2" style="text-align:center">Data writer reliability QoS kind</th>
  </tr>
  <tr style="text-align:center">
    <td>Best effort</td>
    <td>Reliable</td>
  </tr>
  <tr style="text-align:center">
    <th rowspan="2" style="text-align:center">Data reader<br>reliability QoS kind</th>
    <td>Best effort</td>
    <td>✔️</td>
    <td>✔️</td>
  </tr>
  <tr style="text-align:center">
    <td>Reliable</td>
    <td>✖️</td>
    <td>✔️</td>
  </tr>
</table>

### Durability QoS

Using argument **`--transient-local`** will configure the corresponding endpoint with **`TRANSIENT_LOCAL`** durability QoS.
If the argument is not provided, by default it is configured as **`VOLATILE`**.

Whereas **`VOLATILE`** does not store samples for late-joining subscribers, **`TRANSIENT_LOCAL`** ensures that samples are stored and delivered to any late-joining subscribers.

**Note**: **`TRANSIENT_LOCAL`** option may require additional resources to store the samples until they are acknowledged by all subscribers.

Moreover, there is a compatibility rule between data readers and data writers, where the durability QoS kind is checked to ensure the expected behavior.
The following table represents the compatibility matrix (compatible ✔️ vs incompatible ✖️):

<table>
    <tr style="text-align:center">
        <td colspan="2" rowspan="2"></td>
        <th colspan="2" style="text-align:center">Data writer durability QoS kind</th>
    </tr>
    <tr style="text-align:center">
        <td>Volatile</td>
        <td>Transient Local</td>
    </tr>
    <tr style="text-align:center">
        <th rowspan="2" style="text-align:center">Data reader<br>durability QoS kind</th>
        <td>Volatile</td>
        <td>✔️</td>
        <td>✔️</td>
    </tr>
    <tr style="text-align:center">
        <td>Transient Local</td>
        <td>✖️</td>
        <td>✔️</td>
    </tr>
</table>

### Message Size

Using argument **`-m`** `<num>` or **`--msg-size`** `<num>` configures the size of the message payload.

- **Options**:
  - `NONE`: Only an integer value is sent (smallest payload).
  - `SMALL`: An integer value + an array of 16 KB.
  - `MEDIUM`: An integer value + an array of 512 KB.
  - `BIG`: An integer value + an array of 8 MB.
- **Default**: `NONE`

### Number of Samples

Using argument **`-s`** `<num>` or **`--samples`** `<num>` configures the number of samples to send or receive during the test.

- **Range**: `[0 <= <num> <= 65535]`
- **Default**: `0` (until timeout)

If a value is given, the timeout parameter is ignored, and the application will run until the specified number of samples is sent or received.

### Builtin transports

Using argument **``-t``** ``<transport>`` or **``--transport``** ``<transport>`` will configure the internal DomainParticipant using the selected builtin transport:

* **``SHM``** option instantiates a shared memory transport.
* **``UDPv4``** option instantiates a UDP for IPv4 transport.
* **``DEFAULT``** option instantiates both SHM and UDPv4 transports.
  SHM transport has priority over the UDPv4 transport, meaning that SHM will always be used when possible.
* **``LARGE_DATA``** option instantiates UDPv4, TCPv4, and SHM transports.
  However, UDPv4 will only be used for multicast announcements during the participant discovery phase (PDP), while the participant liveliness and the application data delivery occurs over TCPv4 or SHM.

Argument **``--ttl``** ``<num>`` configures the number of multicast discovery Time To Live (TTL) hops.
It can be necessary to update this argument if the connection is deployed in different subnets.

### Wait Time

Using argument **`-w`** `<num>` or **`--wait`** `<num>` configures the time (in milliseconds) before the publisher starts sending samples.
This parameter allows controlled delays in the start of sample transmission, useful for synchronizing with other entities.

- **Range**: `[0 <= <num> <= 4294967]`
- **Default**: `1000` (1 second)

### Interval Between Samples

Using argument **`-i`** `<num>` or **`--interval`** `<num>` configures the time interval (in milliseconds) between each sample sent by the publisher.
This parameter determines the message frequency, influencing the throughput and network usage.

- **Range**: `[1 <= <num> <= 4294967]`
- **Default**: `100` (0.1 seconds)

### Test Duration

Using argument **`-e`** `<num>` or **`--end`** `<num>` configures the total runtime of the test in milliseconds.
This parameter controls how long the publisher or subscriber remains active.

- **Range**: `[1 <= <num> <= 4294967]`
- **Default**: `10000` (10 seconds)

## XML profile playground

The *eProsima Fast DDS* entities can be configured through an XML profile from the environment.
This is accomplished by setting the environment variable ``FASTDDS_DEFAULT_PROFILES_FILE`` to path to the XML profiles file:

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ export FASTDDS_DEFAULT_PROFILES_FILE=benchmark_profile.xml
    ```

* Windows

    ```powershell
    example_path> set FASTDDS_DEFAULT_PROFILES_FILE=benchmark_profile.xml
    ```

The example provides with an XML profiles files with certain QoS:

- Reliable reliability: avoid sample loss.
- Transient local durability: enable late-join subscriber applications to receive previous samples.
- Keep-last history with high depth: ensure certain amount of previous samples for late-joiners.

Applying different configurations to the entities will change to a greater or lesser extent how the application behaves in relation to sample management.
Even when these settings affect the behavior of the sample management, the applications' output will be the similar.
