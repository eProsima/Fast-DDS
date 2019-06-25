## Liveliness QoS example

This example illustrates the Liveliness QoS feature on a Fast-RTPS application.

To launch this example open two different consoles:

In the first one launch: ./LivelinessQoS publisher
In the second one launch: ./LivelinessQoS subscriber

## Application behaviour

When running a publisher, the application uses by default a liveliness kind of `AUTOMATIC_LIVELINESS_QOS` and a lease
duration of 100ms. In addition, it publishes 10 samples, one per second.

When running a subscriber, the application uses by default the same liveliness kind and lease duration as the default
publisher.

The default behaviour can be changed by providing the following command line arguments:

./LivelinessQoS publisher [--lease_duration &lt;lease_duration_ms&gt;] [--kind &lt;kind&gt;] [--sleep &lt;writer_sleep_ms&gt;] [--samples &lt;samples&gt;]  
./LivelinessQoS subscriber [--lease_duration &lt;lease_duration_ms&gt;] [--kind &lt;kind&gt;]

For example:

./LivelinessQoS publisher --lease_duration 1000 --kind AUTOMATIC --sleep 500 --samples 5  
./LivelinessQoS subscriber --lease_duration 1000 --kind AUTOMATIC

will setup the publisher and subscriber to use a lease duration of 1000 ms with kind AUTOMATIC, the publisher will
write a new sample every 500 ms, and a total of 5 samples will be sent.

Possible values for liveliness kind are AUTOMATIC, MANUAL_BY_PARTICIPANT or MANUAL_BY_TOPIC.

## Examples

### Publisher writing too slow

Execute the example with the following parameters:

./LivelinessQoS publisher --lease_duration 100 --kind MANUAL_BY_TOPIC --sleep 500 --samples 5  
./LivelinessQoS subscriber --lease_duration 300 --kind MANUAL_BY_TOPIC

As the lease duration for both publisher and subscriber is smaller than the publisher write rate,
liveliness will be lost after sending a sample and recovered when writing the next one.

### Publisher writing fast enough

Execute the example with the following parameters:

./LivelinessQoS publisher --lease_duration 600 --kind MANUAL_BY_TOPIC --sleep 500 --samples 5  
./LivelinessQoS subscriber --lease_duration 600 --kind MANUAL_BY_TOPIC

In this case, as the lease duration is greater than the publishwer write rate, liveliness will only be lost after
sending the last sample.
