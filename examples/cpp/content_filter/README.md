# Content filter example

The *eProsima Fast DDS content filter* example is a simple application intended to demonstrate how to use Content Filtered Topics.

This example is part of the suite of examples designed by eProsima that aims to illustrate the features and possible configurations of DDS deployments through *eProsima Fast DDS*.

In this case, the *content filter* example show how to use Content Filtered Topics by including two different Filter Factories: the default SQL filter and a custom filter defined in the example.

* [Description of the example](#description-of-the-example)
* [Run the example](#run-the-example)

## Description of the example

Each example application (publisher and subscriber) creates different nested DDS entities: domain participant, publisher, and data writer; and domain participant, subscriber, and data reader, respectively.
In both cases, the three DDS entities (domain participant, publisher/subscriber and data writer/data reader) load their default configuration from the environment.

Content filters can be applied on either the DataWriter's or the DataReader's side. The filter is defined on the DataReader side and during discovery, the DataWriter can retrieve the filter expression from the DataReader. Applying the filter on the writer side can reduce network bandwidth usage but may lead to higher CPU usage on the writer.

This example shows how to define two different types of filter on DataReader side; i.e. the default SQL filter and a custom filter:

* The default SQL filter is a default filter of Fast DDS, whose behaviour is defined by a filter expression, a string using a subset of SQL syntax. For further information regarding default SQL filters please refer to the [Default SQL-like filter](https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/topic/contentFilteredTopic/defaultFilter.html#the-default-sql-like-filter) documentation.
The filter expression is passed as argument during the filter creation and, in this example, the user can set the expression through CLI.
The default expression is keeping only the messages with index between %0 and %1, where %0 and %1 are the indeces of the parameter list passed as argument during the filter creation.
In the example, they correspond to *lower_bound* and *upper_bound*.

* The custom filter is defined in CustomContentFilter.hpp. It filters out all the messages whose index number is between *lower_bound* and *upper_bound*, keeping only the ones outside the defined range. For further information regarding fustom fontent filters please refer to the [Using custom filters](https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/topic/contentFilteredTopic/customFilters.html#using-custom-filters)

For both filters, the user can set the upper and lower bound of the range to filter through CLI with the arguments ``--lower-bound`` and ``--upper-bound``.
By default, *lower_bound* = 5 and *upper_bound* = 9.

A DataWriter will take on the responsibility of filter evaluation instead of the DataReader when all the following criteria are met; otherwise, the DataReader will receive all the samples and then will filter them out:

* The DataWriter has infinite liveliness, as defined by the LivelinessQosPolicy.

* The communication between the DataWriter and the DataReader is neither intra-process nor involves data-sharing.

* The DataReader is not utilizing multicast (`default_multicast_locator_list` is empty).

* The number of DataReaders the DataWriter is filtering for does not exceed the limit set in the `reader_filters_allocation`. Setting a maximum value of 0 disables filter evaluation on the writer side.

For further information regarding content filtering on writer side please refer to the [Conditions for writer side filtering](https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/topic/contentFilteredTopic/writerFiltering.html#conditions-for-writer-side-filtering)

By default, in this example, the filtering is performed on both DataReader and DataWriter side.

## Run the example

To launch this example, two different terminals are required.
One of them will run the publisher example application, and the other will run the subscriber application.

### Content filter publisher

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

### Content filter subscriber

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
user@machine:example_path$ ./content_filter publisher

Publisher running. Please press Ctrl+C to stop the Publisher at any time.
Publisher matched.
Message: 'Hello world' with index: '1' SENT
Message: 'Hello world' with index: '2' SENT
Message: 'Hello world' with index: '3' SENT
...
```

### Content filter subscriber: default filter

```shell
user@machine:example_path$ ./content_filter subscriber

Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
Subscriber matched.
Message: 'Hello world' with index: '5' RECEIVED
Message: 'Hello world' with index: '6' RECEIVED
Message: 'Hello world' with index: '7' RECEIVED
Message: 'Hello world' with index: '8' RECEIVED
Message: 'Hello world' with index: '9' RECEIVED
```

### Content filter subscriber: default filter with expression

```shell
user@machine:example_path$ ./content_filter subscriber --filter-expression "index > %0" -lb 7

Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
Subscriber matched.
Message: 'Hello world' with index: '8' RECEIVED
Message: 'Hello world' with index: '9' RECEIVED
Message: 'Hello world' with index: '10' RECEIVED
Message: 'Hello world' with index: '11' RECEIVED
...
```

### Content filter subscriber: custom filter

```shell
user@machine:example_path$ ./content_filter subscriber --filter-kind custom

Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
Subscriber matched.
Message: 'Hello world' with index: '1' RECEIVED
Message: 'Hello world' with index: '2' RECEIVED
Message: 'Hello world' with index: '3' RECEIVED
Message: 'Hello world' with index: '4' RECEIVED
Message: 'Hello world' with index: '10' RECEIVED
Message: 'Hello world' with index: '11' RECEIVED
...
```
