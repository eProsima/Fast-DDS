# Content filter example

The *eProsima Fast DDS content filter* example is a simple application intended to demonstrate how to use Content Filtered Topics.

This example is part of the suite of examples designed by eProsima that aims to illustrate the features and possible configurations of DDS deployments through *eProsima Fast DDS*.

In this case, the *content filter* example show how to use Content Filtered Topics by including two different Filter Factories: the default SQL filter and a custom filter defined in the example.

* [Description of the example](#description-of-the-example)
* [Run the example](#run-the-example)

## Description of the example

Each example application (publisher and subscriber) creates different nested DDS entities: domain participant, publisher, and data writer; and domain participant, subscriber, and data reader, respectively.
In both cases, the three DDS entities (domain participant, publisher/subscriber and data writer/data reader) load their default configuration from the environment.

Content filters can be applied on either the DataWriter's or the DataReader's side. During discovery, the DataWriter retrieves the filter expression from the DataReader. Applying the filter on the writer side can reduce network bandwidth usage but may lead to higher CPU usage on the writer.

This particular example shows how to includes two different kinds of filter; i.e. the default SQL filter and a custom filter:

* The default filter filters out all the messages whose index number is less than *lower_bound* and greater than *upper_bound*, keeping only the ones in the defined range.

* The custom filter filters out all the messages whose index number is between *lower_bound* and *upper_bound*, keeping only the ones outside the defined range.

For both filters, it is possible to set the upper and lower bound of the range to filter through the arguments ``--lower-bound`` and ``--upper-bound``.
By default, *lower_bound* = 5 and *upper_bound* = 9.

A DataWriter will take on the responsibility of filter evaluation instead of the DataReader when all the following criteria are met; otherwise, the DataReader will receive all the samples and then will filter them out:

* The DataWriter has infinite liveliness, as defined by the LivelinessQosPolicy.

* The communication between the DataWriter and the DataReader is neither intra-process nor involves data-sharing.

* The DataReader is not utilizing multicast.

* The number of DataReaders the DataWriter is filtering for does not exceed the limit set in the `reader_filters_allocation`. Setting a maximum value of 0 disables filter evaluation on the writer side.

By default, the filtering is performed on both sides.

## Run the example

To launch this example, two different terminals are required.
One of them will run the publisher example application, and the other will run the subscriber application.

### Hello world publisher

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./content_filter publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

* Windows

    ```powershell
    example_path> content_filter.exe publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

### Hello world subscriber

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./content_filter subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

* Windows

    ```powershell
    example_path> content_filter.exe subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

All the example available flags can be queried running the executable with the ``-h`` or ``--help`` flag.

### Expected output

Regardless of which application is run first, since the publisher will not start sending data until a subscriber is discovered, the expected output both for publishers and subscribers is a first displayed message acknowledging the match, followed by the amount of samples sent or received until Ctrl+C is pressed.

### Content filter publisher

```shell
Publisher running. Please press Ctrl+C to stop the Publisher at any time.
Publisher matched.
Message: 'Hello world' with index: '1' SENT
Message: 'Hello world' with index: '2' SENT
Message: 'Hello world' with index: '3' SENT
...
```

### Content filter subscriber with default filter

```shell
Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
Subscriber matched.
Message: 'Hello world' with index: '5' RECEIVED
Message: 'Hello world' with index: '6' RECEIVED
Message: 'Hello world' with index: '7' RECEIVED
Message: 'Hello world' with index: '8' RECEIVED
Message: 'Hello world' with index: '9' RECEIVED
```

### Content filter subscriber with custom filter

```shell
Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
Subscriber matched.
Message: 'Hello world' with index: '1' RECEIVED
Message: 'Hello world' with index: '2' RECEIVED
Message: 'Hello world' with index: '3' RECEIVED
Message: 'Hello world' with index: '4' RECEIVED
Message: 'Hello world' with index: '10' RECEIVED
...
```

When Ctrl+C is pressed to stop one of the applications, the other one will show the unmatched status, displaying an informative message, and it will stop sending / receiving messages.
The following is a possible output of the publisher application when stopping the subscriber app.

```shell
...
Message: 'Hello world' with index: '8' SENT
Message: 'Hello world' with index: '9' SENT
Message: 'Hello world' with index: '10' SENT
Message: 'Hello world' with index: '11' SENT
Publisher unmatched.
```

