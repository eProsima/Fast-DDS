# Topic instances example

The *eProsima Fast DDS topic instances* example is an application intended to demonstrate how to deploy keyed topics and instances per topic, using Shape types.

This example is part of the suite of examples designed by eProsima that aims to illustrate the features and possible configurations of DDS deployments through *eProsima Fast DDS*.

In this case, the *topic instances* example describes how to manage different keyed samples with instances.

* [Description of the example](#description-of-the-example)
* [Run the example](#run-the-example)
* [Instances configuration](#instances-configuration)
* [Shape Configuration](#shape-configuration)
* [Shapes Demo compatible](#shapes-demo-compatible)

## Description of the example

Each example application (publisher and subscriber) creates the required DDS entities per case.
Each application inherits from the corresponding listener(s) class(es), overriding the listener's method associated to each event.
When an event occurs, the callback method is triggered.

By default, the publisher sends send four samples with the same shape size and coordinates, but with different _color_ values (key type).
Meanwhile, the subscriber is configured to handle an equivalent number of instances, allowing each keyed _color_ to be managed independently with its own History QoS depth.
Refer to the [Topic, keys and instances](https://fast-dds.docs.eprosima.com/en/stable/fastdds/dds_layer/topic/instances.html#topics-keys-and-instances) section in the _eProsima Fast DDS_ documentation section for more information.

The publisher registers an instance for each key value, and sends a dispose message of each instance when all the instance's related samples have been sent.
An instance disposal informs other subscribers that no additional samples will be sent from the writer related to that specific topic key, but allows late-joining subscribers to receive samples that were previously sent.

Both publisher and subscriber are configured as `KEEP_LAST_HISTORY_QOS`, with a _depth_ depending of the amount of samples given my CLI command (if no _samples_ limit provided, the _max_samples_per_instance_ value is considered, which is `400` by default) **plus one** (so it has enough space to allocate the disposal message too).

To simulate a late-join scenario when a specific number of samples is provided, the publisher application does not stop after sending all the expected samples but after a waiting period, which by default is 10 seconds.
That allows to perform a late-join check scenario: During this time, if a new subscriber matches with the publisher after it has finished sending its samples, this subscriber will receive all the samples for each instance. This occurs even if the total number of samples received exceeds the set History QoS depth because the History QoS depth is configured independently for each instance.

## Run the example

To launch this example, two different terminals are required.
One of them will run the publisher example application, and the other will run the subscriber application.

### Topic instances publisher

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./topic_instances publisher
    Registering instance for RED shape
    Registering instance for BLUE shape
    Registering instance for GREEN shape
    Registering instance for YELLOW shape
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

* Windows

    ```powershell
    example_path> topic_instances.exe publisher
    Registering instance for RED shape
    Registering instance for BLUE shape
    Registering instance for GREEN shape
    Registering instance for YELLOW shape
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

### Topic instances subscriber

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./topic_instances subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

* Windows

    ```powershell
    example_path> topic_instances.exe subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

All the example available flags can be queried running the executable with the ``-h`` or ``--help`` flag.

### Expected output

Regardless of which application is run first, since the publisher does not start sending data until a subscriber is discovered, the expected output for both publishers and subscribers is a first displayed message acknowledging the match, followed by the amount of samples sent or received until Ctrl+C is pressed.

In this example, four different messages are sent per sample, each one associated to different keys (colors).

### Topic instances publisher

```shell
Publisher running. Please press Ctrl+C to stop the Publisher at any time.
Publisher matched.
RED Shape with size 30 at X:113, Y:113 SENT
BLUE Shape with size 30 at X:113, Y:113 SENT
GREEN Shape with size 30 at X:113, Y:113 SENT
YELLOW Shape with size 30 at X:113, Y:113 SENT
RED Shape with size 30 at X:116, Y:113 SENT
BLUE Shape with size 30 at X:116, Y:113 SENT
GREEN Shape with size 30 at X:116, Y:113 SENT
YELLOW Shape with size 30 at X:116, Y:113 SENT
...
```

The subscriber receives by default also those different message and manage them through different instances, one per key (color).

### Topic instances subscriber

```shell
Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
Subscriber matched.
RED Shape with size 30 at X:113, Y:113 RECEIVED
BLUE Shape with size 30 at X:113, Y:113 RECEIVED
GREEN Shape with size 30 at X:113, Y:113 RECEIVED
YELLOW Shape with size 30 at X:113, Y:113 RECEIVED
RED Shape with size 30 at X:116, Y:113 RECEIVED
BLUE Shape with size 30 at X:116, Y:113 RECEIVED
GREEN Shape with size 30 at X:116, Y:113 RECEIVED
YELLOW Shape with size 30 at X:116, Y:113 RECEIVED
...
```

When Ctrl+C is pressed to stop one of the applications, the other one will show the unmatched status, displaying an informative message, and it will stop sending / receiving messages.
The following is a possible output of the publisher application when stopping the subscriber app.

```shell
...
RED Shape with size 30 at X:122, Y:113 SENT
BLUE Shape with size 30 at X:122, Y:113 SENT
GREEN Shape with size 30 at X:122, Y:113 SENT
YELLOW Shape with size 30 at X:122, Y:113 SENT
Publisher unmatched.
```

## Instances configuration

**``-i``** ``<number>`` or **``--instances``** ``<number>`` will configure the amount of instances to be deployed.
This argument is available in both publisher and subscriber.

The argument **``-c``** ``<color>`` or **``--color``** ``<color>`` allows to force a specific keys (color) transmission on the publisher side,  in combination with **``-i``** or **``--instances``**.

### Topic instances publisher

```shell
user@machine:example_path$ ./topic_instances publisher -c MAGENTA,PURPLE -i 2
Registering instance for MAGENTA shape
Registering instance for PURPLE shape
Publisher running. Please press Ctrl+C to stop the Publisher at any time.
Publisher matched.
MAGENTA Shape with size 30 at X:113, Y:113 SENT
PURPLE Shape with size 30 at X:113, Y:113 SENT
MAGENTA Shape with size 30 at X:116, Y:113 SENT
PURPLE Shape with size 30 at X:110, Y:113 SENT
MAGENTA Shape with size 30 at X:119, Y:113 SENT
PURPLE Shape with size 30 at X:107, Y:113 SENT
MAGENTA Shape with size 30 at X:122, Y:113 SENT
PURPLE Shape with size 30 at X:104, Y:113 SENT
...
```

### Topic instances subscriber

```shell
user@machine:example_path$ ./topic_instances subscriber -i 2
Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
Subscriber matched.
MAGENTA Shape with size 30 at X:113, Y:113 RECEIVED
PURPLE Shape with size 30 at X:113, Y:113 RECEIVED
MAGENTA Shape with size 30 at X:116, Y:113 RECEIVED
PURPLE Shape with size 30 at X:110, Y:113 RECEIVED
MAGENTA Shape with size 30 at X:119, Y:113 RECEIVED
PURPLE Shape with size 30 at X:107, Y:113 RECEIVED
MAGENTA Shape with size 30 at X:122, Y:113 RECEIVED
PURPLE Shape with size 30 at X:104, Y:113 RECEIVED
...
```

## Shape Configuration

The arguments **``-n``** ``<topic_name>`` or **``--name``** ``<topic_name>`` configure both publisher and subscriber with the topic name for the shapes communication.
This example restricts those names to be ``Square``, ``Triangle`` or ``Circle``.

The arguments **``--size``**, **``--steps``**, **``--width``** and **``--height``** configure the publisher to set the shape size, the movement steps of the shape per send, and the space width and height bounds where the shape moves, respectively.

### Topic instances publisher

```shell
user@machine:example_path$ ./topic_instances publisher -c GREY -i 1 --size 10 --steps 10 --width 100 --height 100
Publisher running. Please press Ctrl+C to stop the Publisher at any time.
Publisher matched.
GREY Shape with size 10 at X:50, Y:50 SENT
GREY Shape with size 10 at X:50, Y:60 SENT
GREY Shape with size 10 at X:50, Y:70 SENT
GREY Shape with size 10 at X:50, Y:80 SENT
...
```

## Shapes Demo compatible

This example is compatible with the _eProsima Shapes Demo_ application.

The communication between _topic instances_ example publisher and _ShapesDemo_ app subscriber is directly compatible, while for a _ShapesDemo_ publisher and a _topic instances_ subscriber, it is mandatory to configure the _ShapesDemo_ publisher with **``TRANSIENT_LOCAL``** Durability QoS, because the default configuration is _``VOLATILE``_, and it is incompatible with the **``TRANSIENT_LOCAL``** _topic instances_ subscriber from the example.
Refer to the [compatibility rule](https://fast-dds.docs.eprosima.com/en/stable/fastdds/dds_layer/core/policy/standardQosPolicies.html#durability-compatibilityrule) section for more information.
