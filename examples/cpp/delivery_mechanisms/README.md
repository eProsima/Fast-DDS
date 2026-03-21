# Delivery Mechanisms example

The *eProsima Fast DDS delivery mechanisms* example is an application intended to be a DDS deployment with all the available delivery mechanisms.

This example is part of the suite of examples designed by eProsima that aims to illustrate the features and possible configurations of DDS deployments through *eProsima Fast DDS*.

In this case, the *delivery mechanisms* example allows configuring all the possible delivery mechanisms for communicating the example application.


* [Description of the example](#description-of-the-example)
* [Run the example](#run-the-example)
* [Ignore local endpoints](#ignore-local-endpoints)

## Description of the example

Each example application (publisher, subscriber, and pubsub) creates the required DDS entities per case.
Each application inherits from the corresponding listener(s) class(es), overriding the listener's method associated to each event.
When an event occurs, the callback method is triggered.

Moreover, this example uses loans API in both reading and writing calls (refer to ``on_data_available()`` and ``publish()`` methods, respectively)

The delivery mechanisms configuration considers all the possible delivery mechanisms, that can be configured by using **``-m``** or **``--mechanism``** flag, followed by any of these options:

* **``data-sharing``** option instantiates a shared memory transport, but uses data-sharing delivery to write in the memory segment.
* **``intra-process``** option shares the payload pool between the participant's datareader and datawriter.
  This option can only be available when the endpoints belong to the same participant and same process, so it can only be used by the PubSub application.
* **``large-data``** option instantiates a UDPv4 multicast transport to perform PDP discovery phase, and a TCPv4 and SHM transports to perform user data communication.
  Fast DDS priors SHM over TCP if possible.
* **``SHM``** option instantiates a shared memory transport.
* **``TCPv4``** option instantiates a TCP for IPv4 transport.
* **``TCPv6``** option instantiates a TCP for IPv6 transport.
* **``UDPv4``** option instantiates a UDP for IPv4 transport.
* **``UDPv6``** option instantiates a UDP for IPv6 transport.

If it is not configured, the default delivery mechanisms are the default builtin transports: UDPv4 and SHM transports are instantiated, and Fast DDS priors SHM over UDP is possible.

All the example available arguments can be queried running the executable with the **``-h``** or **``--help``** argument.

**Note**: This example type have been purposely configured as plain, bounded, and ``FINAL`` extensibility, in order to meet all delivery mechanisms constraints.

## Run the example

To launch this example, two different terminals are required. One of them will run the publisher example application, and the other will run the subscriber application.

### Delivery mechanisms publisher

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./delivery_mechanisms publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

* Windows

    ```powershell
    example_path> delivery_mechanisms.exe publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

### Delivery mechanisms subscriber

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./delivery_mechanisms subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

* Windows

    ```powershell
    example_path> delivery_mechanisms.exe subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

### Delivery mechanisms pubsub

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./delivery_mechanisms pubsub
    PubSub running. Please press Ctrl+C to stop the PubSub at any time.
    ```

* Windows

    ```powershell
    example_path> delivery_mechanisms.exe pubsub
    PubSub running. Please press Ctrl+C to stop the PubSub at any time.

## Expected output

Regardless of which application is run first, since the publisher will not start sending data until a subscriber is discovered, the expected output both for publishers and subscribers is a first displayed message acknowledging the match, followed by the amount of samples sent or received until Ctrl+C is pressed.

### Delivery mechanisms publisher

```shell
Publisher running. Please press Ctrl+C to stop the Publisher at any time.
Publisher matched.
Sample: 'Delivery mechanisms' with index: '1' SENT
Sample: 'Delivery mechanisms' with index: '2' SENT
Sample: 'Delivery mechanisms' with index: '3' SENT
...
```

### Delivery mechanisms subscriber

```shell
Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
Subscriber matched.
Sample: 'Delivery mechanisms' with index: '1' RECEIVED
Sample: 'Delivery mechanisms' with index: '2' RECEIVED
Sample: 'Delivery mechanisms' with index: '3' RECEIVED
...
```

### Delivery mechanisms pubsub

```shell
PubSub running. Please press Ctrl+C to stop the PubSub at any time.
Sub matched.
Pub matched.
Sample: 'Delivery mechanisms' with index: '1' SENT
Sample: 'Delivery mechanisms' with index: '1' RECEIVED
Sample: 'Delivery mechanisms' with index: '2' SENT
Sample: 'Delivery mechanisms' with index: '2' RECEIVED

...
```

When Ctrl+C is pressed to stop one of the applications, the other one will show the unmatched status, displaying an informative message, and it will stop sending / receiving samples. The following is a possible output of the publisher application when stopping the subscriber app.

```shell
Sample: 'Delivery mechanisms' with index: '8' SENT
Sample: 'Delivery mechanisms' with index: '9' SENT
Sample: 'Delivery mechanisms' with index: '10' SENT
Sample: 'Delivery mechanisms' with index: '11' SENT
Publisher unmatched.
```

## Ignore local endpoints

Using argument **``-i``** or **``--ignore-local-endpoints``** will avoid matching compatible datareaders and datawriters that belong to the same domain participant.

**Note**: It can only be used with the **``PubSub``** entity, otherwise it has no effect.

