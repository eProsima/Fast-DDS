# RTPS example

The *eProsima Fast DDS RTPS* example is a simple application intended to demonstrate a basic RTPS deployment.

This example is part of the suite of examples designed by eProsima that aims to illustrate the features and possible configurations of DDS and RTPS deployments through *eProsima Fast DDS*.

In this case, the *RTPS* example describes the simplest deployment of a Fast DDS reader and writer.

* [Description of the example](#description-of-the-example)
* [Run the example](#run-the-example)

## Description of the example

Each example application creates a different entity in the RTPS (Real-Time Publish-Subscribe) layer: writer and reader;
The lower-level RTPS layer in eProsima Fast DDS implements the RTPS protocol as defined in the standard.
This layer offers more granular control over the communication protocol's internals compared to the higher-level DDS (Data Distribution Service) Layer, giving advanced users greater command over the library's features.

The serialization of the payload is done on the writer side and is designed to convert a `HelloWorld` object into a serialized data format.
This is useful for transmitting the object over a network or saving it in a compact form for later use.
The deserialization of the payload on the reader side consists of converting a serialized data payload into a usable `HelloWorld` object.
This is typically used in applications where data is transmitted in a compact format and needs to be deserialized back into its original form for further processing.


## Run the example

To launch this example, two different terminals are required.
One of them will run the reader example application, and the other will run the writer application.

### RTPS writer

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./rtps writer
    RTPS Writer running. Please press Ctrl+C to stop the RTPS Writer at any time.
    ```

* Windows

    ```powershell
    example_path> rtps.exe writer
    RTPS Writer running. Please press Ctrl+C to stop the RTPS Writer at any time.
    ```

### RTPS reader

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./rtps reader
    Registering RTPS Reader
    RTPS Reader running. Please press Ctrl+C to stop the RTPS Reader at any time.
    ```

* Windows

    ```powershell
    example_path> rtps.exe reader
    Registering RTPS Reader
    RTPS Reader running. Please press Ctrl+C to stop the RTPS Reader at any time.
    ```

All the example available flags can be queried running the executable with the ``-h`` or ``--help`` flag.

### Expected output

Regardless of which application is run first, since the publisher will not start sending data until a subscriber is discovered, the expected output both for publishers and subscribers is a first displayed message acknowledging the match, followed by the amount of samples sent or received until Ctrl+C is pressed.

### RTPS writer

```shell
Remote endpoint with GUID 01.0f.da.70.2b.e0.c8.b2.00.00.00.00|0.0.1.4 matched.
Message Hello World with index 1 SENT
Message Hello World with index 2 SENT
Message Hello World with index 3 SENT
...
```

### RTPS reader

```shell
Remote endpoint with GUID 01.0f.da.70.f5.ea.d7.9b.00.00.00.00|0.0.1.3 matched.
Message: Hello World with index 1 RECEIVED
Message: Hello World with index 2 RECEIVED
Message: Hello World with index 3 RECEIVED
...
```

When Ctrl+C is pressed to stop one of the applications, the other one will show the unmatched status, displaying an informative message, and it will stop sending / receiving messages.
The following is a possible output of the writer application when stopping the reader app.

```shell
...
Message Hello World with index 8 SENT
Message Hello World with index 9 SENT
Message Hello World with index 10 SENT
Remote endpoint with GUID 01.0f.da.70.2b.e0.c8.b2.00.00.00.00|0.0.1.4 unmatched.
```
