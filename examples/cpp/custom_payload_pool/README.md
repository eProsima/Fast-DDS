# Custom Payload Pool example

The *eProsima Fast DDS Custom payload pool* example is a simple application intended to demonstrate how to set the endpoints' payload pool from the DDS layer.

This example is part of the suite of examples designed by eProsima that aims to illustrate the features and possible configurations of DDS deployments through *eProsima Fast DDS*.

In this example, a custom payload pool is implemented instead of the default one, and passed to the endpoints in the publisher and subscriber implementations.

* [Description of the example](#description-of-the-example)
* [Run the example](#run-the-example)

## Description of the example
A Payload is the data the user wants to transmit between a DataWriter and a DataReader. In order to manage the lifecycle of the Payloads, DataReaders and DataWriters use a pool object, an implementation of the `IPayloadPool` interface. The user can define different pool implementations, to obtain different kind of optimizations to manage the Payloads. A custom Payload pool can be given to `create_datawriter()` and `create_datareader()` functions, and DataWriters and DataReaders will use the provided pool when a new change in their history is requested or released. The example provides a simple payload pool that reserves and frees memory each time.

## Run the example

To launch this example, two different terminals are required.
One of them will run the publisher example application, and the other will run the subscriber application.

### Custom payload pool publisher

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./custom_payload_pool publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

* Windows

    ```powershell
    example_path> custom_payload_pool.exe publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

### Custom payload pool subscriber

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./custom_payload_pool subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

* Windows

    ```powershell
    example_path> custom_payload_pool.exe subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

All the example available flags can be queried running the executable with the ``-h`` or ``--help`` flag.

### Expected output

Regardless of which application is run first, since the publisher will not start sending data until a subscriber is discovered, the expected output both for publishers and subscribers is a first displayed message acknowledging the match, followed by the amount of samples sent or received until Ctrl+C is pressed.

### Custom payload pool publisher

```shell
Publisher running. Please press Ctrl+C to stop the Publisher at any time.
Publisher matched.
Message: 'Hello world' with index: '1' SENT
Message: 'Hello world' with index: '2' SENT
Message: 'Hello world' with index: '3' SENT
...
```

### Custom payload pool subscriber

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
Message: 'Hello world' with index: '9' SENT
Message: 'Hello world' with index: '10' SENT
Message: 'Hello world' with index: '11' SENT
Publisher unmatched.
```
