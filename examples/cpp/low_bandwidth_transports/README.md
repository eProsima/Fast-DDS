# Low bandwidth transports example

The *eProsima Fast DDS low bandwidth transports* example is a simple application intended to demonstrate how to
configure and use the network transports provided by *eProsima Fast DDS* to optimize the network bandwidth usage.

This example is part of the suite of examples designed by eProsima that aims to illustrate the features and possible
configurations of DDS deployments through *eProsima Fast DDS*.

In this case, the *low bandwidth* example describes the simplest deployment of a Fast DDS publisher and subscriber.

* [Description of the example](#description-of-the-example)
* [Run the example](#run-the-example)

## Description of the example

Each example application (publisher and subscriber) creates different nested DDS entities: domain participant, publisher,
and data writer; and domain participant, subscriber, and data reader, respectively.
In both cases, the domain participant is configured to use the low bandwidth transports.

## Run the example

To launch this example, two different terminals are required.
One of them will run the publisher example application, and the other will run the subscriber application.

### Low bandwidth transports publisher

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./low_bandwidth_transports publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

* Windows

    ```powershell
    example_path> low_bandwidth_transports.exe publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

### Low bandwidth transports subscriber

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./low_bandwidth_transports subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

* Windows

    ```powershell
    example_path> low_bandwidth_transports.exe subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

All the example available flags can be queried running the executable with the ``-h`` or ``--help`` flag.

### Expected output

Regardless of which application is run first, since the publisher will not start sending data until a subscriber is
discovered, the expected output both for publishers and subscribers is a first displayed message acknowledging the match,
followed by the amount of samples sent or received until Ctrl+C is pressed.

### Low bandwidth transports publisher

```shell
Publisher running. Please press Ctrl+C to stop the Publisher at any time.
Publisher matched.
Message: 'Hello world' with index: '1' SENT
Message: 'Hello world' with index: '2' SENT
Message: 'Hello world' with index: '3' SENT
...
```

### Low bandwidth transports subscriber

```shell
Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
Subscriber matched.
Message: 'Hello world' with index: '1' RECEIVED
Message: 'Hello world' with index: '2' RECEIVED
Message: 'Hello world' with index: '3' RECEIVED
...
```

When Ctrl+C is pressed to stop one of the applications, the other one will show the unmatched status, displaying an informative message, and it will stop sending / receiving messages.
The following is a possible output of the publisher application when stopping the subscriber app.

```shell
...
Message: 'Hello world' with index: '8' SENT
Message: 'Hello world' with index: '9' SENT
Message: 'Hello world' with index: '10' SENT
Message: 'Hello world' with index: '11' SENT
Publisher unmatched.
```
