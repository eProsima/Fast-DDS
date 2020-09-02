# Custom Listeners  example

This example illustrates how to create custom listeners for different DDS entities.
When multiple listeners are configured to respond to the same callback the lowest member will be executed.
In lowest to highest order we have:

DataReaderListener -> SubscriberListener -> DomainParticipantListener

DataWriterListener -> PublisherListener -> DomainParticipantListener


To launch this test open two different consoles:

In the first one launch: `./DDSCustomListener publisher 1`
In the second one launch: `./DDSCustomListener subscriber 1`

The above will launch publisher and subscriber with the listener of the bottom most entities activated.

## Application behaviour

In this application, the publisher sends a number of samples to the subscriber. In the publisher application both the
DomainParticipant and the DataWriter have listeners with the `on_publication_matched` callback implemented defined.
When the ENABLE_LOWEST_LISTENER argument is set to false the DataWriterListener is given a `StatusMask::none()` which
disables all of its callbacks.
These callbacks are therefore intercepted by the next implemented callback. In this case the DomainParticipant.
The subscriber works similarly with the `on_subscription_matched` and `on_data_available`
callbacks.

When a listener is executing a callback it will identify itself in parenthesis. Whether or not the lowest listener is
used can be changed with the ENABLE_LOWEST_LISTENER option.


```bash
$ ./DDSCustomListener publisher|subscriber ENABLE_LOWEST_LISTENER
```

