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

By default, the publisher will send five samples with the same shape size and coordinates, but with different _color_ values (key type).
On the other hand, the subscriber is configured to handle the same amount of instances.
In that way, each keyed _color_ is managed separately, having its own _History QoS depth_.
Refer to the [Topic, keys and instances](https://fast-dds.docs.eprosima.com/en/stable/fastdds/dds_layer/topic/instances.html#topics-keys-and-instances) section in the _eProsima Fast DDS_ documentation section for more information.

Both publisher and subscriber are configured as `KEEP_LAST_HISTORY_QOS`, with a _depth_ depending of the amount of samples given my CLI command (if no _samples_ limit provided, the _max_samples_per_instance_ value is considered, which is `400` by default).

If an amount of _samples_ is provided, the publisher application has a timeout (by default is `10` seconds) to make it wait a short period of time after sending all expected samples.
That allows to perform a late-join check: when a new subscriber matches with the publisher that has already sent all its samples, the subscriber receives all samples for all instances, even though the total amount of samples received is higher than the _History QoS depth_ set, due to each _History QoS depth_ is independent per each instance.

## Run the example

To launch this example, two different terminals are required.
One of them will run the publisher example application, and the other will run the subscriber application.

### Topic instances publisher

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./topic_instances publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

* Windows

    ```powershell
    example_path> topic_instances.exe publisher
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

Regardless of which application is run first, since the publisher will not start sending data until a subscriber is discovered, the expected output both for publishers and subscribers is a first displayed message acknowledging the match, followed by the amount of samples sent or received until Ctrl+C is pressed.

In this example, five different messages are sent per sample, each one associated to different keys (colors).

### Topic instances publisher

```shell
Publisher running. Please press Ctrl+C to stop the Publisher at any time.
Publisher matched.
RED Shape with size 30 at X:113, Y:113 SENT
BLUE Shape with size 30 at X:113, Y:113 SENT
GREEN Shape with size 30 at X:113, Y:113 SENT
YELLOW Shape with size 30 at X:113, Y:113 SENT
ORANGE Shape with size 30 at X:113, Y:113 SENT
RED Shape with size 30 at X:116, Y:113 SENT
BLUE Shape with size 30 at X:116, Y:113 SENT
GREEN Shape with size 30 at X:116, Y:113 SENT
YELLOW Shape with size 30 at X:116, Y:113 SENT
ORANGE Shape with size 30 at X:116, Y:113 SENT
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
ORANGE Shape with size 30 at X:113, Y:113 RECEIVED
RED Shape with size 30 at X:116, Y:113 RECEIVED
BLUE Shape with size 30 at X:116, Y:113 RECEIVED
GREEN Shape with size 30 at X:116, Y:113 RECEIVED
YELLOW Shape with size 30 at X:116, Y:113 RECEIVED
ORANGE Shape with size 30 at X:116, Y:113 RECEIVED
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
ORANGE Shape with size 30 at X:122, Y:113 SENT
Publisher unmatched.
```

## Instances configuration

Using argument **``-i``** ``<number>`` or **``--instances``** ``<number>`` will configure the amount of instances to be deployed.
This argument is available in both publisher and subscriber.

There is the option of forcing a specific key (color) transmission on the publisher side, by using the argument **``-c``** ``<color>`` or **``--color``** ``<color>``, together with **``-i 1``** or **``--instances 1``**.

### Topic instances publisher

```shell
user@machine:example_path$ ./topic_instances publisher -c MAGENTA -i 1
Publisher running. Please press Ctrl+C to stop the Publisher at any time.
Publisher matched.
MAGENTA Shape with size 30 at X:113, Y:113 SENT
MAGENTA Shape with size 30 at X:116, Y:113 SENT
MAGENTA Shape with size 30 at X:119, Y:113 SENT
MAGENTA Shape with size 30 at X:122, Y:113 SENT
...
```

### Topic instances subscriber

```shell
user@machine:example_path$ ./topic_instances subscriber -i 1
Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
Subscriber matched.
MAGENTA Shape with size 30 at X:113, Y:113 RECEIVED
MAGENTA Shape with size 30 at X:116, Y:113 RECEIVED
MAGENTA Shape with size 30 at X:119, Y:113 RECEIVED
MAGENTA Shape with size 30 at X:122, Y:113 RECEIVED
...
```

## Shape Configuration

Using argument **``-n``** ``<topic_name>`` or **``--name``** ``<topic_name>`` will configure both publisher and subscriber with the topic name for the shapes communication.
This example restricts those names to be ``Square``, ``Triangle`` or ``Circle``.

The arguments **``--size``**, **``--steps``**, **``--width``** and **``--height``** configures the publisher to set the shape size, the movement steps of the shape per send, and the space width and height bounds where the shape moves, respectively.

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

Although the _topic instances_ example > _ShapesDemo_ app compatibility is direct, it is needed to configure the _ShapesDemo_ publisher with **``TRANSIENT_LOCAL``** Durability QoS to perform communication _ShapesDemo_ app > _topic instances_ example, because the default configuration is _``VOLATILE``_ and it is incompatible with the **``TRANSIENT_LOCAL``** subscriber from the example.
Refer to the [compatibility rule](https://fast-dds.docs.eprosima.com/en/stable/fastdds/dds_layer/core/policy/standardQosPolicies.html#durability-compatibilityrule) section for more information.
