# Lifespan QoS example

This example illustrates how to use the Lifespan QoS in a Fast DDS application.

To launch this test open two different consoles:

In the first one launch: ./DDSLifespanQoSExample publisher  
In the second one launch: ./DDSLifespanQoSExample subscriber

## Application behaviour

The publisher will send the specified number of samples and, after waiting for a given amount of time, it will try to remove all samples from its history.
If the lifespan is smaller than the wait time, no changes will exist in the history, and the publisher will report that 0 samples were removed. On the contrary, if the lifespan is greater than the wait time, the publisher will be able to remove changes, and the number of samples removed will be
reported.

The subscriber waits until it receives a specified number of samples from the publisher, then waits for a given amount of time, and finally tries to take data from the history. Similarly, if the lifespan is smaller than the wait time, the subscriber will not be able to take data from the history.

The default parameters are 500ms for the lifespan, 2000 ms for the wait time and 10 for the number of samples. You can change these defaults by executing:

./DDSLifespanQoSExample publisher &lt;lifespan_ms&gt; &lt;sleep_ms&gt; &lt;samples&gt;  
./DDSLifespanQoSExample subscriber &lt;lifespan_ms&gt; &lt;sleep_ms&gt;

for publisher and subscriber respectively.
