------------------------
|   DEADLINE EXAMPLE   |
------------------------

1 - Application description
---------------------------

This example illustrates the Deadline QoS feature on a FastRTPS Application.

To launch this example open two different consoles:

In the first one launch: ./DeadlineQoSExample publisher
In the second one launch: ./DeadlineQoSExample subscriber

2 - Application behaviour
-------------------------

The application will use a topic with three different keys,
one of which (the third one) sends a sample only half of the times.

By default the deadline period is set to 2000 ms and the publisher write rate to 1000 ms, so
that the third instance misses the deadline 50% of the times.

The deadline period can be set by providing a second argument to the binary. For example:

./DeadlineQoSExample publisher 1000
./DeadlineQosExample subscriber 1000

will set a deadline period of 1000 ms in both publisher and subscriber. Additionally, the rate at which the
publisher writes can be changed by providing a third argument. For example:

./DeadlineQoSExample publisher 1000 500

will make the publisher write samples every 500 ms.
