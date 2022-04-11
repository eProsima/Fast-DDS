# Content Filtered Topic Example

This example extends the HelloWorld example to show how to use Content Filtered Topics.
It does so by including two different Filter Factories: on the one hand, the default SQL filter; on the other hand, a custom filter defined in the example.

## Execution instructions (Linux platform)

To launch this example, open three different terminals:

In the first one launch the Subscriber using the default SQL filter:

```
./DDSContentFilteredTopicExample --subscriber
```

In the second one launch the Subscriber using the custom filter:

```
./DDSContentFilteredTopicExample --subscriber -f custom
```

Finally, in the third terminal launch the Publisher:

```
./DDSContentFilteredTopicExample --publisher
```

The Subscriber with the default filter should received only the samples between indexes 5 and 9, while the Subscriber with the custom filter should received samples which index is lower than 3 and greater than 5.

In order to know further possible arguments please run

```
./DDSContentFilteredTopicExample --help
```
