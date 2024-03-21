# Hello world example

The *eProsima Fast DDS hello world* example is a simple application intended to demonstrate a basic DDS deployment.

This example is part of the suite of examples designed by eProsima that aims to illustrate the features and possible configurations of DDS deployments through *eProsima Fast DDS*.

In this case, the *hello world* example describes the simplest deployment of a Fast DDS publisher and subscriber.

* [Description of the example](#description-of-the-example)
* [Run the example](#run-the-example)
* [Wait-set subscriber](#wait-set-subscriber)
* [XML profile playground](#xml-profile-playground)

## Description of the example

Each example application (publisher and subscriber) creates different nested DDS entities: domain participant, publisher, and data writer; and domain participant, subscriber, and data reader, respectively.
In both cases, the three DDS entities (domain participant, publisher/subscriber and data writer/data reader) load their default configuration from the environment.
If the environment does not specify the expected configuration, they take the default configuration per entity.
For further information regarding the configuration environment, please refer to the *[XML profile playground](#xml-profile-playground)* section.

This particular example includes two different subscription paradigms; i.e. listening callbacks and wait-sets:

* The listening callback mechanism consists on declaring a listener class and attaching it to the data reader.
  When the data reader is triggered by an event, it runs the listener's method associated to that event, as a callback.
  For simplicity, in this example, the subscriber class inherits from the listener class, overriding the corresponding callback.

* The wait-set is a mechanism where a dedicated thread waits until a status condition occurs.
  In that moment, that status condition triggering event would be evaluated to determine witch actions should be taken against it.

For this example, both listening callback and wait-set implementation would run similar code and generate equivalent output for both triggering events: subscription matching and new data available.

## Run the example

To launch this example, two different terminals are required.
One of them will run the publisher example application, and the other will run the subscriber application.

### Hello world publisher

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./hello_world publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

* Windows

    ```powershell
    example_path> hello_world.exe publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

### Hello world subscriber

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./hello_world subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

* Windows

    ```powershell
    example_path> hello_world.exe subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

All the example available flags can be queried running the executable with the ``-h`` or ``--help`` flag.

### Expected output

Regardless of which application is run first, since the publisher will not start sending data until a subscriber is discovered, the expected output both for publishers and subscribers is a first displayed message acknowledging the match, followed by the amount of samples sent or received until Ctrl+C is pressed.

### Hello world publisher

```shell
Publisher running. Please press Ctrl+C to stop the Publisher at any time.
Publisher matched.
Message: 'Hello world' with index: '1' SENT
Message: 'Hello world' with index: '2' SENT
Message: 'Hello world' with index: '3' SENT
...
```

### Hello world subscriber

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

## Wait-set subscriber

As described in the *[Description of the example](#description-of-the-example)* section, the *hello world* example has two listening implementations. Launching the subscriber example with the flag ``-w`` or ``--waitset`` will use the wait-set approach instead of the listening callback.

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./hello_world subscriber --waitset
    ```

* Windows

    ```powershell
    example_path> hello_world.exe subscriber --waitset
    ```

The expected output is exactly the same as the described in the *[previous](#expected-output)* section.

## XML profile playground

The *eProsima Fast DDS* entities can be configured through an XML profile from the environment.
This is accomplished by setting the environment variable ``FASTDDS_DEFAULT_PROFILES_FILE`` to path to the XML profiles file:

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ export FASTDDS_DEFAULT_PROFILES_FILE=hello_world_profile.xml
    ```

* Windows

    ```powershell
    example_path> set FASTDDS_DEFAULT_PROFILES_FILE=hello_world_profile.xml
    ```

The example provides with an XML profiles files with certain QoS:

- Reliable reliability: avoid sample loss.
- Transient local durability: enable late-join subscriber applications to receive previous samples.
- Keep-last history with high depth: ensure certain amount of previous samples for late-joiners.

Applying different configurations to the entities will change to a greater or lesser extent how the application behaves in relation to sample management.
Even when these settings affect the behavior of the sample management, the applications' output will be the similar.
