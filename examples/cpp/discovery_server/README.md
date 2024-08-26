# Discovery Server example

The *eProsima Fast DDS discovery server* example is an application intended to demonstrate the use of *Discovery Server*.

This example is part of the suite of examples designed by eProsima that aims to illustrate the features and possible configurations of DDS deployments through *eProsima Fast DDS*.

In this case, the *discovery server* example describes the use of *Discovery Server* as a discovery hub among different publishers and subscribers.

* [Description of the example](#description-of-the-example)
* [Run the example](#run-the-example)

## Description of the example

Within the example, user can create three different entities: a publisher, a subscriber, or a discovery server.
The publisher and subscriber entities are discovery server clients designed to send and receive messages, respectively, while the server is intended to act as the centralized discovery entity among them.

A server can also act as a client of another server, allowing the creation of a network of servers that can be used to distribute the discovery information among them.
For further information about discovery server, please refer to the [Discovery Server documentation](https://fast-dds.docs.eprosima.com/en/latest/fastdds/discovery/discovery_server.html).

## Run the example

To launch this example, three different terminals are required.
One of them will run the publisher example application, another one will run the subscriber application and the third one will run the discovery server application.

### Discovery Server publisher client

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./discovery_server publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

* Windows

    ```powershell
    example_path> discovery_server.exe publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

### Discovery Server subscriber client

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./discovery_server subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

* Windows

    ```powershell
    example_path> discovery_server.exe subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

### Discovery Server server

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./discovery_server server
    Subscriber running. Please press Ctrl+C to stop the Server at any time.
    ```

* Windows

    ```powershell
    example_path> discovery_server.exe server
    Subscriber running. Please press Ctrl+C to stop the Server at any time.
    ```

All the example available flags can be queried running the executable with the ``-h`` or ``--help`` flag.

### Expected output

Regardless of which application is run first, the publisher will not start sending data until the server is up and the subscriber is discovered.
The expected output both for publisher and subscriber is a first displayed message acknowledging the match, followed by the amount of samples sent or received until Ctrl+C is pressed.

### Discovery Server publisher client

```shell
Publisher running. Please press Ctrl+C to stop the Publisher at any time.
Publisher matched.
Message: 'Hello world' with index: '1' SENT
Message: 'Hello world' with index: '2' SENT
Message: 'Hello world' with index: '3' SENT
...
```

### Discovery Server subscriber client

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

