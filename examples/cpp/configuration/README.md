# Configuration example

The *eProsima Fast DDS configuration* example is an application intended to be a DDS deployment configuration playground.

This example is part of the suite of examples designed by eProsima that aims to illustrate the features and possible configurations of DDS deployments through *eProsima Fast DDS*.

In this case, the *configuration* example allows a long list of configurable options to show how the DDS entities behave under different configurations, and includes a set of meta-examples to illustrate those behaviors.
e simplest deployment of a Fast DDS publisher and subscriber.

* [Description of the example](#description-of-the-example)
* [Run the example](#run-the-example)
* TODO list meta-examples
* [XML Configuration](#xml-configuration)

## Description of the example

TODO (this is pasted from previous readme)

This example extends the configuration options of a trivial HelloWorld by letting the user specify properties of entities such as durability, reliability or specify the transport protocol to be used, among other possibilities.
This could be useful, for example, to quickly test whether two endpoints are compatible and hence would match.
Additionally, the message type includes a data sequence which size can be set by the user, allowing to send large data between endpoints.
Note: Due to the nature of the data type (not bounded), this example will not use data sharing.

## Run the example

To launch this example, two different terminals are required. One of them will run the publisher example application, and the other will run the subscriber application.

### Configuration publisher

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./configuration publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

* Windows

    ```powershell
    example_path> configuration.exe publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

### Configuration subscriber

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./configuration subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

* Windows

    ```powershell
    example_path> configuration.exe subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

All the example available flags can be queried running the executable with the ``-h`` or ``--help`` flag.

### Expected output

Regardless of which application is run first, since the publisher will not start sending data until a subscriber is discovered, the expected output both for publishers and subscribers is a first displayed message acknowledging the match, followed by the amount of samples sent or received until Ctrl+C is pressed.

### Configuration publisher

```shell
Publisher running. Please press Ctrl+C to stop the Publisher at any time.
Publisher matched.
Sample: 'Configuration' with index: '1' (10 Bytes) SENT
Sample: 'Configuration' with index: '2' (10 Bytes) SENT
Sample: 'Configuration' with index: '3' (10 Bytes) SENT
...
```

### Configuration subscriber

```shell
Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
Subscriber matched.
Sample: 'Configuration' with index: '1' (10 Bytes) RECEIVED
Sample: 'Configuration' with index: '2' (10 Bytes) RECEIVED
Sample: 'Configuration' with index: '3' (10 Bytes) RECEIVED
...
```

When Ctrl+C is pressed to stop one of the applications, the other one will show the unmatched status, displaying an informative message, and it will stop sending / receiving messages. The following is a possible output of the publisher application when stopping the subscriber app.

```shell
Sample: 'Configuration' with index: '8' (10 Bytes) SENT
Sample: 'Configuration' with index: '9' (10 Bytes) SENT
Sample: 'Configuration' with index: '10' (10 Bytes) SENT
Sample: 'Configuration' with index: '11' (10 Bytes) SENT
Publisher unmatched.
```

## TODO list meta-examples


## XML Configuration

TODO refactor this (from previous readme)

Using argument `--xml-profile <profile_name>` will configure the internal DomainParticipant using the profile name loaded from an XML file.
To load XML files check [Fast DDS documentation](https://fast-dds.docs.eprosima.com/en/latest/fastdds/xml_configuration/xml_configuration.html).
Loading example XML configuration [file](shm_off.xml) and calling this example with `--xml-profile no_shm_participant_profile` will disable Shared Memory Transport for the internal DomainParticipant created.

This code presents how to run a publisher with this example without Shared Memory:

```sh
# From AdvancedConfigurationExample installation dir
FASTDDS_DEFAULT_PROFILES_FILE=shm_off.xml ./AdvancedConfigurationExample publisher --xml-profile no_shm_participant_profile
```
