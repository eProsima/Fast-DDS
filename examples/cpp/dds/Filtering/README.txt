Subscribing to a subset of the Topic updates: Filtering
-------------------------------------------------------

A typical use case on a DDS middleware is subscribing to a subset of the published samples, or filtering the data by content, by time, or both.

- Content Based Filter: The subscriber request a subset of the samples based on the content. As an example, imagine a Radar Track topic and a station (subscriber) monitoring a specific area: we can use the track coordinates to filter.

- Time Based Filter: The subscriber requests just one sample in a given period. Imagine a Topic with a high frequency of updates, and a subscriber in charge of representing these updates in a GUI. This subscriber does not need all the updates, but just a reasonable rate of updates for the human eye.

A straightforward solution is to just discard not required samples in the subscriber side, but that is not always a good solution. If your subscriber is using a wireless low bandwidth link, or is a low power device, you don't want to spend valuable bandwidth or CPU time. In that case, you need publisher filtering.

Publisher filtering:
--------------------

To enable publisher filtering you should make use of the partition Qos (Quality of Service) of the publisher and subscriber. This QoS parameter allows you to "partition" your topic. The publisher and subscriber will match only if they have a common partition, and the partition QoS allows you to specify a list of partitions giving you full flexibility.

The example:
------------

In the example we will filter by time. We create two publishers on the same topic:

- Fast Publisher: publishing on the partition "Fast_Partition", a sample per second.
- Slow Publisher: publishing on the partition "Slow_Partition", a sample every 5 seconds.

And in the subscriber side, we create a subscriber on one of these partitions.

To run the example, execute an instance of the publishers:

	./DDSFilteringExample publisher

And two instances of the subscriber, on the two different partitions:

	./DDSFilteringExample subscriber slow
	./DDSFilteringExample subscriber fast

You will see the slow partition subscriber getting just one sample every 5 seconds, and the fast partition subscriber every second. You can now use wireshark to see that the filtering is happening in the publishing side, due to the partitions mechanism.

