# Hello world example

The *eProsima Fast DDS hello world* example is a simple application intended to demonstrate a basic DDS deployment.

## Introduction

This example is part of the suite of examples designed by eProsima that aims to illustrate the features and possible configurations of DDS deployments through *eProsima Fast DDS*.

In this case, the *hello world* example describes the simplest deployment of a Fast DDS publisher and subscriber.

## Example in deep

Each entity of the example (publisher and subscriber) creates different nested DDS entities: domain participant, publisher & dataWriter, and subscriber & dataReader, respectively.
In both cases, the three DDS entities (domain participant, publisher/subscriber and dataWriter/dataReader) load their default configuration from the environment.
If the environment does not specify the expected configuration, they take the default configuration per entity.
For further information regarding the configuration environment, please refer to the *[XML profile playground](#xml-profile-playground)* subsection in the *[advanced configuration](#advanced-configuration)* section.

The *hello world* example, together with the remain examples, would include a listening callback on the subscriber side. The subscriber will manage the new available data in the same thread as the main subscriber application.
For simplicity, the ``HelloWorldSubscriber`` implements it own callback.

Furthermore, this particular example includes a wait-set implementation. In contrast to implementing a listening callback, the wait-set is a mechanism where a dedicated thread waits until a status condition occurs. In that moment, that status condition triggering event would be evaluated to determine witch actions should be taken against it.

For this example, both listening callback and wait-set implementation would run the same code and generate the same output for both triggering events: subscription matching and new data available.

## Run the example

To launch this test open two different consoles. One of them will run the publisher side of the example, and the other would run the subscriber side.

### Hello world publisher

#### Ubuntu ( / MacOS )

```shell
user@machine:example_path$ ./hello_world publisher
Publisher running. Please press Ctrl+C to stop the Publisher at any time.
```

#### Windows

```powershell
example_path> hello_world.exe publisher
Publisher running. Please press Ctrl+C to stop the Publisher at any time.
```

### Hello world subscriber

#### Ubuntu ( / MacOS )

```shell
user@machine:example_path$ ./hello_world subscriber
Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
```

#### Windows

```powershell
example_path> hello_world.exe subscriber
Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
```

## Expected output

It does not matter which entity is launched first, because the publisher has a condition to wait until the first match to start sending hello world samples.
The expected output on both cases is a first displayed message acknowledging they have matched, followed by the amount of samples sent or received until user press Ctrl+C.

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

When pressed Ctrl+C to stop one of the applications, the other will detect the unmatched status, and will display an informative message.
This output example represents stopping the subscriber application.

### Hello world publisher

```shell
...
Message: 'Hello world' with index: '9' SENT
Message: 'Hello world' with index: '10' SENT
Message: 'Hello world' with index: '11' SENT
Publisher unmatched.
Message: 'Hello world' with index: '12' SENT
Message: 'Hello world' with index: '13' SENT
...
```

### Hello world subscriber

```shell
...
Message: 'Hello world' with index: '9' RECEIVED
Message: 'Hello world' with index: '10' RECEIVED
Message: 'Hello world' with index: '11' RECEIVED
user@machine:example_path$ |
```

Stopping the subscriber application does not make the publisher stop sending data. But in the opposite case, stopping the publisher application will make subscriber stop receiving data.

### Hello world publisher

```shell
...
Message: 'Hello world' with index: '9' SENT
Message: 'Hello world' with index: '10' SENT
Message: 'Hello world' with index: '11' SENT
user@machine:example_path$ |
```

### Hello world subscriber

```shell
...
Message: 'Hello world' with index: '9' RECEIVED
Message: 'Hello world' with index: '10' RECEIVED
Message: 'Hello world' with index: '11' RECEIVED
Subscriber unmatched.
```

## Advanced configuration

### Wait-set subscriber

As described in the *[example in deep](#example-in-deep)* section, the *hello world* example has two listening implementations. Launching the subscriber example with the flag ``-w`` or ``--waitset`` will use the wait-set approach instead of the listening callback.

#### Ubuntu ( / MacOS )

```shell
user@machine:example_path$ ./hello_world subscriber --waitset
```

#### Windows

```powershell
example_path> hello_world.exe subscriber --waitset
```

The expected output is exactly the same as the described in the *[previous](#expected-output)* section.


All the example available flags can be queried running the executable with the ``-h`` or ``--help`` flag.

### XML profile playground

The *eProsima Fast DDS* entities can be configured through an XML profile from the environment by adding a reference the XML profiles file setting the environment variable ``FASTDDS_DEFAULT_PROFILES_FILE`` to its path.

#### Ubuntu ( / MacOS )

```shell
user@machine:example_path$ export FASTDDS_DEFAULT_PROFILES_FILE=hello_world_profile.xml
```

#### Windows

```powershell
example_path> set FASTDDS_DEFAULT_PROFILES_FILE=hello_world_profile.xml
```

The example contains a XML profiles files with certain QoS:

- Reliable reliability: avoid sample loss.
- Transient local durability: enable late-join-participants to receive previous samples if connection was lost.
- Keep-last history with high depth: ensure certain amount of previous samples for late-joiners.

Applying different configurations to the entities would change the sample management behavior, among other configurations, but in any case these configurations would affect in the behavior of the *hello_world* example.
The expected output would be exactly the same as launching it with no environment configuration.

Try your own XML profile to see how your configuration affects the *hello world* example communication.
