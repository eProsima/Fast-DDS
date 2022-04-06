# Content Filtered Topic Example

This example extends the HelloWorld example to show how to use Content Filtered Topics.
The example includes the use of two different Filter Factories.
On the one hand the default SQL filter and on the other hand a custom filter defined in the example.

## Execution instructions

To launch this example open three different consoles:

In the first one launch the Subscriber using the default SQL filter: ./DDSContentFilteredTopicExample default_subscriber
In the second one launch the Subscriber using the custom filter: ./DDSContentFilteredTopicExample custom_subscriber
Finally, in the third terminal launch the Publisher: ./DDSContentFilteredTopicExample publisher

The default Subscriber should received only the samples between indexes 5 and 9.
Custom Subscriber should received samples which index is lower than 3 and greater than 5.

## Arguments

First argument is mandatory and it should be one of the following values: `publisher`, `default_subscriber` and `custom_subscriber`.
The `publisher` can be launched specifying the number of samples to be sent (10 by default) and the sleep between samples in [ms] (by default 100 ms) in that order.