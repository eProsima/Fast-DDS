# Deadline QoS example

## Application description

This example illustrates the Deadline QoS feature on a Fast DDS Application.

To launch this example open two different consoles:

In the first one launch: ./DeadlineQoSExample publisher  
In the second one launch: ./DeadlineQoSExample subscriber

## Application behaviour

The application will use a topic with three different keys,
one of which (the third one) sends a sample only half of the times.

By default the deadline period is set to 2000 ms and the publisher write rate to 1000 ms, so
that the third instance misses the deadline 50% of the time. The number of samples that will be written is set to 10. 

The default behaviour can be changed by providing the following command line arguments:

./DeadlineQoSExample publisher [--deadline &lt;deadline_ms&gt;] [--sleep &lt;writer_sleep_ms&gt;] [--samples &lt;samples&gt;]  
./DeadlineQoSExample subscriber [--deadline &lt;deadline_ms&gt;]

For example:

./DeadlineQoSExample publisher --deadline 1000 --sleep 500 --samples 5  
./DeadlineQosExample subscriber --deadline 1000

will setup the publisher and subscriber to use a deadline period of 1000 ms, the publisher will write a new sample every 500 ms for the first two keys and every 1000 ms for the third one, and a total of 5 samples will be sent.