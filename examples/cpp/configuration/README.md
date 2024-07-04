# Configuration example

The *eProsima Fast DDS configuration* example is an application intended to be a DDS deployment configuration playground.

This example is part of the suite of examples designed by eProsima that aims to illustrate the features and possible configurations of DDS deployments through *eProsima Fast DDS*.

In this case, the *configuration* example allows a long list of configurable options to show how the DDS entities behave under different configurations, and includes a set of meta-examples to illustrate those behaviors.
e simplest deployment of a Fast DDS publisher and subscriber.

* [Description of the example](#description-of-the-example)
* [Run the example](#run-the-example)
* [Builtin transports](#builtin-transports)
* [Deadline QoS](#deadline-qos)
* [Disable positive ACKs QoS](#disable-positive-acks-qos)
* [Durability QoS](#durability-qos)
* [History QoS](#history-qos)
* [Lifespan QoS](#lifespan-qos)
* [Liveliness QoS](#liveliness-qos)
* [Ownership QoS](#ownership-qos)
* [Partition QoS](#partition-qos)
* [Publish mode QoS](#publish-mode-qos)
* [Reliability QoS](#reliability-qos)
* [Resource limits QoS](#resource-limits-qos)
* [XML profiles configuration](#xml-configuration)

## Description of the example

Each example application (publisher and subscriber) creates the required DDS entities per case.
Both publisher and subscriber inherit from the corresponding listener class, overriding the listener's method associated to each event.
When an event occurs, the callback method is triggered.

For a complete demo, both publisher and subscriber implements all event callbacks.

Moreover, this example extends the configuration options of a simple data type (check ``configuration.idl`` file) by letting the user specify properties of entities such as durability, reliability, or specify the transport protocol to be used, among other possibilities.
This could be useful, for example, to quickly test whether two endpoints are compatible and hence would match.

Additionally, the message data type includes a data sequence which size can be set by the user, allowing to send large data between endpoints.

**Note**: Due to the nature of the data type (not bounded), this example will not use data sharing mechanism.

## Run the example

To launch this example, two different terminals are required. One of them will run the publisher example application, and the other will run the subscriber application.

### Configuration publisher

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./configuration publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

* Windows

    ```powershell
    example_path> configuration.exe publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

### Configuration subscriber

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./configuration subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

* Windows

    ```powershell
    example_path> configuration.exe subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

All the example available arguments can be queried running the executable with the **``-h``** or **``--help``** argument.

## Expected output

Regardless of which application is run first, since the publisher will not start sending data until a subscriber is discovered, the expected output both for publishers and subscribers is a first displayed message acknowledging the match, followed by the amount of samples sent or received until Ctrl+C is pressed.

### Configuration publisher

```shell
Publisher running. Please press Ctrl+C to stop the Publisher at any time.
Publisher matched.
Sample: 'Configuration' with index: '1' (10 Bytes) SENT
Sample: 'Configuration' with index: '2' (10 Bytes) SENT
Sample: 'Configuration' with index: '3' (10 Bytes) SENT
...
```

### Configuration subscriber

```shell
Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
Subscriber matched.
Sample: 'Configuration' with index: '1' (10 Bytes) RECEIVED
Sample: 'Configuration' with index: '2' (10 Bytes) RECEIVED
Sample: 'Configuration' with index: '3' (10 Bytes) RECEIVED
...
```

When Ctrl+C is pressed to stop one of the applications, the other one will show the unmatched status, displaying an informative message, and it will stop sending / receiving samples. The following is a possible output of the publisher application when stopping the subscriber app.

```shell
Sample: 'Configuration' with index: '8' (10 Bytes) SENT
Sample: 'Configuration' with index: '9' (10 Bytes) SENT
Sample: 'Configuration' with index: '10' (10 Bytes) SENT
Sample: 'Configuration' with index: '11' (10 Bytes) SENT
Publisher unmatched.
```

## Builtin transports

Using argument **``-t``** ``<transport>`` or **``--transport``** ``<transport>`` will configure the internal DomainParticipant using the selected builtin transport:

* **``SHM``** option instantiates a shared memory transport.
* **``UDPv4``** option instantiates a UDP for IPv4 transport.
* **``DEFAULT``** option instantiates both SHM and UDPv4 transports.
  SHM transport has priority over the UDPv4 transport, meaning that SHM will always be used when possible.
* **``LARGE_DATA``** option instantiates UDPv4, TCPv4, and SHM transports.
  However, UDPv4 will only be used for multicast announcements during the participant discovery phase (PDP), while the participant liveliness and the application data delivery occurs over TCPv4 or SHM.

Argument **``--ttl``** ``<num>`` configures the number of multicast discovery Time To Live (TTL) hops.
It can be necessary to update this argument if the connection is deployed in different subnets.

## Deadline QoS

Using argument **``--deadline``** ``<period>`` will configure the corresponding endpoint to trigger a callback when the frequency of sending / receiving new samples falls below the given threshold.
That callbacks are ``on_offered_deadline_missed`` or the publisher case, and ``on_requested_deadline_missed`` on the subscriber case.

Moreover, there is a compatibility rule between data readers and data writers, where the offered deadline period (writer side) must be less or equal to the requested deadline period (reader side).

Otherwise, the entities are considered incompatible (and they will not match).

The argument **``-i``** ``<period>`` or **``--interval``** ``<period>`` configures the **publisher** application with the sending samples period (in milliseconds).
It should be always **greater** than the deadline period, otherwise ``on_offered_deadline_missed`` will be triggered any time a sample is sent.

If **``--deadline``** is not configured, it takes the _eProsima Fast DDS_ default value (infinite).

## Disable positive ACKs QoS

Using argument **``--disable-positive-ack``** will configure  the corresponding endpoint to **not** exchange positive acknowledge messages.
It would only take place in a reliable communication, where both endpoints are configured as **``RELIABLE``** [reliability QoS](#reliability-qos).

That configuration would reduce the amount of traffic, because the data reader will send only ACK messages when a sample is lost, whereas with the default value (positive ACKs are enabled) the reader will also send ACK messages any time it receives (and acknowledges) a sample.

Moreover, there is a compatibility rule between data readers and data writers, where this condition is checked to ensure the expected behavior.
The following table represents the compatibility matrix (compatible ✔️ vs incompatible ✖️):

<table>
  <tr style="text-align:center">
    <td colspan="2" rowspan="2"></td>
    <th colspan="2" style="text-align:center">Data writer</th>
  </tr>
  <tr style="text-align:center">
    <td>Positive ACKs <i>enabled</i></td>
    <td>Positive ACKs <i>disabled</i></td>
  </tr>
  <tr style="text-align:center">
    <th rowspan="2" style="text-align:center">Data reader</th>
    <td>Positive ACKs <i>enabled</i></td>
    <td>✔️</td>
    <td>✔️</td>
  </tr>
  <tr style="text-align:center">
    <td>Positive ACKs <i>disabled</i></td>
    <td>✖️</td>
    <td>✔️</td>
  </tr>
</table>

The argument **``--ack-keep-duration``** ``<duration>`` configures the **publisher** application with the duration (in milliseconds) it keeps the data before considering it as acknowledged.

If **``--ack-keep-duration``** is not configured, it takes the _eProsima Fast DDS_ default value (infinite).

## Durability QoS

Using argument **``--transient-local``** will configure the corresponding endpoint with **``TRANSIENT_LOCAL``** durability QoS.
If the argument is not provided, by default is configured as **``VOLATILE``**.

As long as a data writer can send samples even if there are no data readers on the network, a data reader that joins after some data has been written (late-joiner data reader) could be interested in accessing to those previous samples.
The durability QoS defines how the system manages those samples that existed before the late-joiner data reader.

Whereas **``VOLATILE``** ignores past samples, **``TRANSIENT_LOCAL``** fills the data reader history with past samples (if any).

**Note**: **``TRANSIENT_LOCAL``** durability QoS configuration requires the [reliability QoS](#reliability-qos) to be set as **``RELIABLE``**.

Moreover, there is a compatibility rule between data readers and data writers, where the durability QoS kind is checked to ensure the expected behavior.
The following table represents the compatibility matrix (compatible ✔️ vs incompatible ✖️):

<table>
  <tr style="text-align:center">
    <td colspan="2" rowspan="2"></td>
    <th colspan="2" style="text-align:center">Data writer durability QoS kind</th>
  </tr>
  <tr style="text-align:center">
    <td>Volatile</td>
    <td>Transient local</td>
  </tr>
  <tr style="text-align:center">
    <th rowspan="2" style="text-align:center">Data reader<br>durability QoS kind</th>
    <td>Volatile</td>
    <td>✔️</td>
    <td>✔️</td>
  </tr>
  <tr style="text-align:center">
    <td>Transient local</td>
    <td>✖️</td>
    <td>✔️</td>
  </tr>
</table>

## History QoS

Using argument **``--keep-all``** will configure the corresponding endpoint to keep all the values until they can be delivered to the existing subscribers.
This **``KEEP_ALL``** history QoS option does not have numeric argument, because the history depth in limited only by the [resource limit QoS](#resource-limits-qos).

On the other hand, using the argument **``-k``** ``<depth>`` or **``--keep-last``** ``<depth>`` will configure the endpoint's history QoS as **``KEEP_LAST``**, with the given value as depth.
This configuration only attempts to keep the most recent values of the instance and discard the oldest values.
The maximum number of samples to keep and deliver is defined by the depth, which must be consistent with the [resource limits](#resource-limits-qos) (``0 <`` **``depth``** ``<= max_samples_per_instance``, ``max_samples_per_instance * max_instances <= max_samples``).

## Lifespan QoS

Using argument **``--lifespan``** ``<period>`` will configure the corresponding endpoint to remove samples from their history after a certain period of time (in  milliseconds).
That period is infinite by default, so the samples are not removed unless this policy is modified.

## Liveliness QoS

Using argument **``-l``** ``<duration>``  or **``--liveliness``** ``<duration>`` will configure the corresponding endpoint to wait that lease duration before considering that a data writer is no longer alive.

Also, the argument **``--liveliness-assert``** ``<period>`` configures the period between consecutive liveliness messages sent by the participant.

Finally, the argument **``--liveliness-kind``** ``<kind>`` establishes how a data writer is considered alive:

* **``AUTOMATIC``**: the entities within the remote participant are considered alive as long as the local process where the participant is running, and the link connecting it to remote participants, exist.

The two Manual modes require that the **publisher** application asserts the liveliness periodically before the lease duration timer expires.
Publishing any new data value implicitly asserts the data writer's liveliness, but it can be done explicitly too.

* **``MANUAL_BY_PARTICIPANT``**: If one of the entities in the publishing side asserts its liveliness, the service deduces that all other entities within the same DomainParticipant are also alive.

* **``MANUAL_BY_TOPIC``**: requires at least one instance within the data writer is asserted to consider it alive.

**Note**: The **``--liveliness-assert``** ``<period>`` configuration is only taken into account when the liveliness QoS kind is **``AUTOMATIC``** or **``MANUAL_BY_PARTICIPANT``**.
Also, this ``<period>`` must be **lower** than the liveliness lease duration.

The lease duration of the **publisher** application must be **greater** than the lease duration of the **subscriber** application, otherwise the endpoints are considered incompatible.

Moreover, there is a *liveliness kind* compatibility rule between data readers and data writers, where the kind is checked to ensure the expected behavior.
The following table represents the compatibility matrix (compatible ✔️ vs incompatible ✖️):

<table>
  <tr style="text-align:center">
    <td colspan="2" rowspan="2"></td>
    <th colspan="3" style="text-align:center">Data writer liveliness QoS kind</th>
  </tr>
  <tr style="text-align:center">
    <td>Automatic</td>
    <td>Manual by participant</td>
    <td>Manual by topic</td>
  </tr>
  <tr style="text-align:center">
    <th rowspan="3" style="text-align:center">Data reader<br>liveliness QoS kind</th>
    <td>Automatic</td>
    <td>✔️</td>
    <td>✔️</td>
    <td>✔️</td>
  </tr>
  <tr style="text-align:center">
    <td>Manual by participant</td>
    <td>✖️</td>
    <td>✔️</td>
    <td>✔️</td>
  </tr>
  <tr style="text-align:center">
    <td>Manual by topic</td>
    <td>✖️</td>
    <td>✖️</td>
    <td>✔️</td>
  </tr>
</table><br>

The argument **``-i``** ``<period>`` or **``--interval``** ``<period>`` configures the **publisher** application with the sending samples period (in milliseconds).
It should be always **lower** than the liveliness lease duration, otherwise liveliness will be lost after sending each sample and recovered when sending the next sample.


If **``--liveliness``**  or  **``--liveliness-assert``** are not configured, they take the _eProsima Fast DDS_ default values (infinite).

## Ownership QoS

Using argument **``-o``** or **``--ownership``** will configure the corresponding endpoint with **``EXCLUSIVE``** [ownership QoS kind](https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/core/policy/standardQosPolicies.html#ownershipqospolicykind).
If the argument is not provided, by default is configured as **``SHARED``**.

Whereas **``SHARED``** allows multiple data writer to update the same instance of data, **``EXCLUSIVE``** forces each instance of data to be updated only by one data writer.
The owner can be changed dynamically according to the highest ownership QoS strength between the alive data writers.

That strength can be changed only in the **publisher** application using the argument **``--ownership-strength``** ``<number>``.

Moreover, there is a compatibility rule between data readers and data writers, where the ownership QoS kind is checked to ensure the expected behavior.
The following table represents the compatibility matrix (compatible ✔️ vs incompatible ✖️):

<table>
  <tr style="text-align:center">
    <td colspan="2" rowspan="2"></td>
    <th colspan="2" style="text-align:center">Data writer ownership QoS kind</th>
  </tr>
  <tr style="text-align:center">
    <td>Shared</td>
    <td>Exclusive</td>
  </tr>
  <tr style="text-align:center">
    <th rowspan="2" style="text-align:center">Data reader<br>ownership QoS kind</th>
    <td>Shared</td>
    <td>✔️</td>
    <td>✖️</td>
  </tr>
  <tr style="text-align:center">
    <td>Exclusive</td>
    <td>✖️</td>
    <td>✔️</td>
  </tr>
</table>

## Partition QoS

Using argument **``-p``** ``<string>``  or **``--partition``** ``<string>`` will configure logical partitions inside the physical partitions (DDS domains).

Endpoints will need to share same domain, topic, and partition to match.
Empty string (default value ``''``) is also considered as partition, and it matches with other partition names (using same matching rules as string-matching and regular-expression-matching).

Usage: **``--partition``** ``partition1``, **``--partition``** ``'partition1;partition2'``, **``--partition``** ``"partition1;partition2;partition3"``.

## Publish mode QoS

Using argument **``-a``** or **``--async``** configures the **publisher** application to use the **``ASYNCHRONOUS``** publish mode.
If the argument is not provided, by default is configured as **``SYNCHRONOUS``**.

Whereas **``SYNCHRONOUS``** publish mode QoS sends data in the context of the user thread that calls the write operation, **``ASYNCHRONOUS``** uses an internal thread which takes the responsibility of sending the data asynchronously (the write operation returns before the data is actually sent).

## Reliability QoS

Using argument **``-r``** or **``--reliable``** will configure the corresponding endpoint with **``RELIABLE``** reliability QoS.
If the argument is not provided, by default is configured as **``BEST_EFFORT``**.

Whereas **``BEST_EFFORT``** do not retransmit missing samples, **``RELIABLE``** expects an arrival confirmation sample (acknowledge sample or ACK) per sample sent.

**Note**: **``RELIABLE``** option may block the write operation if certain scenarios are met ([resource limits](#resource-limits-qos) reached for instance).

Moreover, there is a compatibility rule between data readers and data writers, where the reliability QoS kind is checked to ensure the expected behavior.
The following table represents the compatibility matrix (compatible ✔️ vs incompatible ✖️):

<table>
  <tr style="text-align:center">
    <td colspan="2" rowspan="2"></td>
    <th colspan="2" style="text-align:center">Data writer reliability QoS kind</th>
  </tr>
  <tr style="text-align:center">
    <td>Best effort</td>
    <td>Reliable</td>
  </tr>
  <tr style="text-align:center">
    <th rowspan="2" style="text-align:center">Data reader<br>reliability QoS kind</th>
    <td>Best effort</td>
    <td>✔️</td>
    <td>✔️</td>
  </tr>
  <tr style="text-align:center">
    <td>Reliable</td>
    <td>✖️</td>
    <td>✔️</td>
  </tr>
</table>

## Resource limits QoS

Using argument **``--max-samples``** ``<number>`` will configure the corresponding endpoint with the given resource limit.
The remain resource limits QoS (max instances and max samples per instance) are configured likewise using **``--max-instances``** ``<number>`` and **``--max-samples-per-instance``** ``<number>``, respectively.

* **``--max-samples``** controls the maximum number of samples that the endpoint can manage across all the instances associated with it.
* **``--max-instances``** controls the maximum number of instances that the endpoint can manage.
* **``--max-samples-per-instance``** controls the maximum number of samples within an instance that the endpoint can manage.

Providing ``0`` or lower values will set the setting as infinite resources in both three cases.
Check [history QoS](#history-qos) and [reliability QoS](#reliability-qos) sections regarding constraints and compatibilities.

## XML profiles configuration

Using argument **``--profile-participant``** ``<profile_name>`` will configure the internal DomainParticipant using the profile name loaded from an XML file.
It works the same way for the data reader (**``--profile-reader``** ``<reader_profile_name>``) and data writer (**``--profile-writer``** ``<writer_profile_name>``).
To load XML files check [Fast DDS documentation](https://fast-dds.docs.eprosima.com/en/latest/fastdds/xml_configuration/xml_configuration.html).

Loading example XML configuration [file](configuration_profile.xml) and running this example with **``--profile-reader configuration_datareader_profile``** or **``--profile-writer configuration_datawriter_profile``** will create the corresponding endpoint with the following QoS:

* ``TRANSIENT_LOCAL`` Durability QoS
* ``KEEP_LAST 100`` History QoS
* ``RELIABLE`` Reliability QoS
* ``max_samples 100`` Resource limits QoS
