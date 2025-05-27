# RPC example

The *eProsima Fast DDS RPC* example shows how to create a service oriented architecture using the *Remote Procedure Call* communication pattern over Fast DDS.
Remote Procedure Calls (see RPC over DDS specification), also known as RPC, is a type of bidirectional communication used in a request-reply pattern.
The RPC architecture is based on the client-server model: the client sends a request to the server, and the server sends one or more responses (replies) back to the client.
[eProsima FastDDS-Gen](https://github.com/eProsima/Fast-DDS-Gen) tool allows the generation of source code for a RPC over DDS application from an IDL file.
The IDL file must contain the operation that can be called on both the client and server sides, and the parameters that can be passed to them.
These operations are specified using the concept of interfaces defined in the OMG IDL specification.

* [Description of the example](#description-of-the-example)
* [Run the example](#run-the-example)

## Description of the example

To exemplify the use of RPC over DDS, this example implements a calculator service.
This service allows clients to perform some arithmetic operations such as addition, subtraction, accumulation, stack-addition, and filtering of a list of numbers.
This *rpc* example shows the two operating modes of *RPC* over DDS:

* Simple request operations:

  ```mermaid
    sequenceDiagram
    participant Client
    participant Client Request Writer
    participant Client Reply Reader

    participant Server
    participant Server Request Reader
    participant Server Reply Writer

    Client->>Client: Wait for server availability
    Client->>Client Request Writer: request
    Client Request Writer->>Server Request Reader: request
    Server Request Reader->>Server: calculate response
    Server->>Server Reply Writer: response
    Server Reply Writer->>Client Reply Reader: response
    Client Reply Reader->>Client: response
    ```

    The following operations available in this mode within the client are:

    ```
        -a <num_1> <num_2>, --addition <num_1> <num_2>      Adds two numbers
                                                            [-2^31 <= <num_i> <= 2^31-1]

        -s <num_1> <num_2>, --substraction <num_1> <num_2>  Substracts two numbers
                                                            [-2^31 <= <num_i> <= 2^31-1]

        -r, --representation-limits                         Computes the representation
                                                            limits of a 32-bit integer

        -f <num>, --fibonacci <num>                         Returns a feed of results
                                                            with the <num> first elements
                                                            of the Fibonacci sequence
    ```

* Streaming operations where data is exchanged between a client and a server in a continuous stream of messages,
  rather than a single, discrete request-response exchange thanks to the `@feed` annotation in the IDL files.
  Please refer to the [RPC documentation](https://fast-dds.docs.eprosima.com/en/latest/fastdds/rpc_dds/rpc_dds_intro.html) for more information about this feature.
  In this case, the clients can take one of the following operations:

  ```
        --sum-all                                       Sum all the values provided
                                                        in the input feed

        --accumulator                                   Return a feed of results
                                                        with the sum of all received
                                                        values from an input feed

        --filter <filter_kind>                          Return a feed of results
                                                        with the values that match
                                                        the input filter kind
                                                        [<filter_kind> = 0, 1, 2]
                                                        [0 = EVEN,
                                                         1 = ODD,
                                                         2 = PRIME]
  ```

## Run the example

To launch this example, two different terminals are required.
One of them will run the server application, and the other will run the client application.
Mind that it is possible to run multiple server instances and client instances simultaneously.

### Server

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./calculator server
    2025-05-27T15:41:55.633 [INFO] [ServerApp] Server initialized with ID: 01.0f.d3.e9.2b.1b.c9.02.00.00.00.00
    2025-05-27T15:41:55.633 [INFO] [main] Server running. Please press Ctrl+C to stop the Server at any time.
    ```

* Windows

    ```powershell
    example_path> calculator.exe server
    2025-05-27T15:41:55.633 [INFO] [ServerApp] Server initialized with ID: 01.0f.e2.d3.6f.1b.c9.02.00.00.00.00
    2025-05-27T15:41:55.633 [INFO] [main] Server running. Please press Ctrl+C to stop the Server at any time.
    ```

### Client

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./calculator client --addition 1 2
    2025-05-27T15:44:13.446 [INFO] [ClientApp] Client initialized with ID: 01.0f.d3.e9.ed.21.30.62.00.00.00.00
    2025-05-27T15:44:13.446 [INFO] [main] Client running. Please press Ctrl+C to stop the Client at any time.
    2025-05-27T15:44:13.446 [DEBUG] [ClientApp] Trying to reach server, attempt 1/10
    2025-05-27T15:44:14.448 [INFO] [ClientApp] Server reachable
    2025-05-27T15:44:14.449 [INFO] [ClientApp] Addition result: 1 + 2 = 3
    2025-05-27T15:44:14.449 [INFO] [ClientApp] Operation finished. Stopping client execution...
    2025-05-27T15:44:14.449 [INFO] [ClientApp] Client execution stopped
    ```

* Windows

    ```powershell
    example_path> calculator.exe client --addition 1 2
    2025-05-27T15:44:13.446 [INFO] [ClientApp] Client initialized with ID: 01.0f.d3.d7.21.35.33.68.00.00.00.00
    2025-05-27T15:44:13.446 [INFO] [main] Client running. Please press Ctrl+C to stop the Client at any time.
    2025-05-27T15:44:13.446 [DEBUG] [ClientApp] Trying to reach server, attempt 1/10
    2025-05-27T15:44:14.448 [INFO] [ClientApp] Server reachable
    2025-05-27T15:44:14.449 [INFO] [ClientApp] Addition result: 1 + 2 = 3
    2025-05-27T15:44:14.449 [INFO] [ClientApp] Operation finished. Stopping client execution...
    2025-05-27T15:44:14.449 [INFO] [ClientApp] Client execution stopped
    ```
