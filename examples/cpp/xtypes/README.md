# X-Types example

The *eProsima Fast DDS xtypes* example is an application intended to demonstrate the use of both the Dynamic Types binding and the remote type discovery features.

This example is part of the suite of examples designed by eProsima that aims to illustrate the features and possible configurations of DDS deployments through *eProsima Fast DDS*.

In this case, the *xtypes* example describes a simple deployment of a Fast DDS publisher and subscriber, withe the particularity of it leveraging X-Types.

* [Description of the example](#description-of-the-example)
* [Run the example](#run-the-example)
* [XML profile playground](#xml-profile-playground)

## Description of the example

Each example application (publisher and subscriber) creates different nested DDS entities: domain participant, publisher, and data writer; and domain participant, subscriber, and data reader, respectively.
In both cases, the three DDS entities (domain participant, publisher/subscriber and data writer/data reader) load their default configuration from the environment.
If the environment does not specify the expected configuration, they take the default configuration per entity.
For further information regarding the configuration environment, please refer to the *[XML profile playground](#xml-profile-playground)* section.

The particularity of this example resides in the use of X-Types, which allows the definition of types at runtime.
In this case:

1. The publisher application creates a type at runtime using the Fast DDS Dynamic Types API.
The types can be configured through C++ API or [XML file](https://fast-dds.docs.eprosima.com/en/latest/fastdds/xml_configuration/dynamic_types.html).
The flag `--xml-types` allows to use the types defined in the XML file. The profile must be uploaded setting the environment variable ``FASTDDS_DEFAULT_PROFILES_FILE`` to the XML file path, see [XML profile playground](#xml-profile-playground).
2. The subscriber application discovers the type defined by the publisher and uses it to create a data reader, introspect the type, and print the received data.

It is important to note that this example is fully type compatible with the [Hello world](../hello_world/README.md) example, meaning that the publisher and subscriber applications can be run interchangeably with the *hello world* example applications.

## Run the example

To launch this example, two different terminals are required.
One of them will run the publisher example application, and the other will run the subscriber application.

### Hello world publisher

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./xtypes publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

* Windows

    ```powershell
    example_path> xtypes.exe publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

### Hello world subscriber

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./xtypes subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

* Windows

    ```powershell
    example_path> xtypes.exe subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

All the example available flags can be queried running the executable with the ``-h`` or ``--help`` flag.

### Expected output

Regardless of which application is run first, since the publisher will not start sending data until a subscriber is discovered, the expected output both for publishers and subscribers is a first displayed message acknowledging the match, followed by the amount of samples sent or received until Ctrl+C is pressed.

### Hello world publisher

```shell
Publisher running for 10 samples. Please press Ctrl+C to stop the Publisher at any time.
Publisher matched.
Message sent:
  - index: 1
  - message: 'Hello xtypes world'
Message sent:
  - index: 2
  - message: 'Hello xtypes world'
Message sent:
  - index: 3
  - message: 'Hello xtypes world'
...
```

### Hello world subscriber

```shell
Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
Subscriber matched.
Message received:
  - index: 1
  - message: 'Hello xtypes world'
Message received:
  - index: 2
  - message: 'Hello xtypes world'
Message received:
  - index: 3
  - message: 'Hello xtypes world'
...
```

When Ctrl+C is pressed to stop one of the applications, the other one will show the unmatched status, displaying an informative message, and it will stop sending / receiving messages.
The following is a possible output of the publisher application when stopping the subscriber app.

```shell
...
Message sent:
  - index: 5
  - message: 'Hello xtypes world'
Message sent:
  - index: 6
  - message: 'Hello xtypes world'
Publisher unmatched.
```

## XML profile playground

The *eProsima Fast DDS* entities can be configured through an XML profile from the environment.
This is accomplished by setting the environment variable ``FASTDDS_DEFAULT_PROFILES_FILE`` to path to the XML profiles file:

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ export FASTDDS_DEFAULT_PROFILES_FILE=xtypes_complete_profile.xml
    ```

* Windows

    ```powershell
    example_path> set FASTDDS_DEFAULT_PROFILES_FILE=xtypes_complete_profile.xml
    ```

The example provides with two XML profiles files with certain QoS:

- Reliable reliability: avoid sample loss.
- Transient local durability: enable late-join subscriber applications to receive previous samples.
- Keep-last history with high depth: ensure certain amount of previous samples for late-joiners.
- Type propagation: Set to either complete of minimal depending on the used XML profiles file.

Applying different configurations to the entities will change to a greater or lesser extent how the application behaves in relation to sample management.
Even when these settings affect the behavior of the sample management, the applications' output will be the similar.
