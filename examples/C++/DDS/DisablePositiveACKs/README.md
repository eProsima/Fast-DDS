# Disable Positive ACKs QoS example

This example illustrates how to use the Disable Positive ACKs QoS extension in a Fast DDS application. When using this QoS, Acknack messages are only sent by the reader if it is missing any samples. In this way, strict reliability is no longer maintained but the writer keeps samples in its history for a sufficient duration (or keep duration) so that readers can negatively acknowledge it. After this duration, the sample is considered to be acknowledged by all readers and is removed from the writer history.

To launch this test open two different consoles:

In the first one launch: ./DDSDisablePositiveACKsQoS publisher --disable  
In the second one launch: ./DDSDisablePositiveACKsQoS subscriber --disable

The above will launch publisher and subscriber using this QoS, i.e. positive acks will not be sent.

## Application behaviour

In this application, the publisher sends a number of samples and periodic heartbeats to the subscriber, and the subscriber expects to receive a number of samples from the publisher. The best way to test this example is to analyze the network traffic. When this QoS is enabled no ack messages will be exchanged (unless samples are not received by the subscriber). When this QoS is disabled, ack messages will be sent in response to heartbeats.

By default the QoS is disabled, the publisher sends 20 samples every 1000 milliseconds and the keep duration is set to 5000 milliseconds. The subscriber waits until 20 samples are received. You can change these defaults by executing:

./DDSDisablePositiveACKsQoS publisher [--disable] [--keep_duration &lt;keep_duration_ms&gt;] [--sleep &lt;writer_sleep_ms&gt;] [--samples &lt;samples&gt;]  
./DDSDisablePositiveACKsQoS subscriber [--disable] [--samples &lt;samples&gt;]

for publisher and subscriber respectively, where the option &lt;enable_QoS&gt; will enable the QoS (i.e. positive akcs will not be sent), &lt;keep_duration_ms&gt; will set the duration for which to keep the samples, &lt;sleep_ms&gt; determines the amount of time to wait before the publisher sends a new sample, and &lt;samples&gt; sets the total number of samples to be sent/received.
