# Static EDP discovery example

The *eProsima Fast DDS static EDP discovery* example is an application intended to be a DDS deployment configured with a static EDP discovery phase.

This example is part of the suite of examples designed by eProsima that aims to illustrate the features and possible configurations of DDS deployments through *eProsima Fast DDS*.

In this case, the *static EDP discovery* example allows configuring the EDP phase to use a XML file with information about the remote endpoints aimed to match with, that is use in the PDP phase.
Please refer to the [Static Discovery](https://fast-dds.docs.eprosima.com/en/latest/fastdds/discovery/static.html#static-discovery-settings) section in the *eProsima Fast DDS* documentation for more information.


* [Description of the example](#description-of-the-example)
* [Run the example](#run-the-example)

## Description of the example

Each example application (publisher and subscriber) creates the required DDS entities per case.
Each application inherits from the corresponding listener(s) class(es), overriding the listener's method associated to each event.
When an event occurs, the callback method is triggered.

Moreover, this example uses loans API in both reading and writing calls (refer to ``on_data_available()`` and ``publish()`` methods, respectively)

The discovery phase can be performed through different mechanisms as simple (default), manual, static and discovery server.
This example aims to display the deployment of a static discovery mechanism, which is setup in the participant configuration to avoid all EDP meta-traffic.
It can only be configured when all the datareaders and datawriters (the endpoints part of the EDP discovery phase) are known beforehand.
The ``HelloWorld_static_disc.xml`` file contains the endpoints information that has been configured in the publisher and the subscriber applications, such as Durability and Reliability QoS, user ID, and endpoint ID.

All the example available arguments can be queried running the executable with the **``-h``** or **``--help``** argument.

## Run the example

To launch this example, two different terminals are required. One of them will run the publisher example application, and the other will run the subscriber application.

### Static EDP discovery publisher

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./static_edp_discovery publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

* Windows

    ```powershell
    example_path> static_edp_discovery.exe publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

### Static EDP discovery subscriber

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./static_edp_discovery subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

* Windows

    ```powershell
    example_path> static_edp_discovery.exe subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

## Expected output

Regardless of which application is run first, since the publisher will not start sending data until a subscriber is discovered, the expected output both for publishers and subscribers is a first displayed message acknowledging the match, followed by the amount of samples sent or received until Ctrl+C is pressed.

### Static EDP discovery publisher

```shell
Publisher running. Please press Ctrl+C to stop the Publisher at any time.
Publisher matched.
Message: 'Hello world' with index: '1' SENT
Message: 'Hello world' with index: '2' SENT
Message: 'Hello world' with index: '3' SENT
...
```

### Static EDP discovery subscriber

```shell
Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
Subscriber matched.
Message: 'Hello world' with index: '1' RECEIVED
Message: 'Hello world' with index: '2' RECEIVED
Message: 'Hello world' with index: '3' RECEIVED
...
```

When Ctrl+C is pressed to stop one of the applications, the other one will show the unmatched status, displaying an informative message, and it will stop sending / receiving samples. The following is a possible output of the publisher application when stopping the subscriber app.

```shell
Message: 'Hello world' with index: '8' SENT
Message: 'Hello world' with index: '9' SENT
Message: 'Hello world' with index: '10' SENT
Message: 'Hello world' with index: '11' SENT
Publisher unmatched.
```
