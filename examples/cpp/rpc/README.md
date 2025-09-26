# RPC example

The *RPC over DDS* example shows how to create a service oriented architecture using the *Remote Procedure Call* communication pattern over Fast DDS.
The RPC architecture is based on the client-server model.
The client sends a request to the server, and the server sends one or more responses (replies) back to the client.
[eProsima FastDDS-Gen](https://github.com/eProsima/Fast-DDS-Gen) tool allows the generation of source code for a RPC over DDS application from an IDL file.
The IDL file must define an interface with the operations that can be called on the client and executed by the server.
These operations are specified using the concept of interfaces defined in the OMG IDL specification.

Please refer to the [RPC over DDS](https://fast-dds.docs.eprosima.com/en/latest/fastdds/rpc_dds/rpc_dds_intro.html) section in the Fast DDS documentation for further information on this topic.

* [Description of the example](#description-of-the-example)
* [Run the example](#run-the-example)

## Description of the example

To exemplify the use of RPC over DDS, this example implements a calculator service.
This service allows clients to perform some arithmetic operations such as addition, subtraction, accumulation, stack-addition, and filtering of a list of numbers.
This *rpc* example shows the two operating modes of *RPC* over DDS:

* Simple request operations:

    The following operations available in this mode within the client are:

    ```
        -a <num_1> <num_2>, --addition <num_1> <num_2>      Adds two numbers
                                                            [-2^31 <= <num_i> <= 2^31-1]

        -s <num_1> <num_2>, --substraction <num_1> <num_2>  Substracts two numbers
                                                            [-2^31 <= <num_i> <= 2^31-1]

        -r, --representation-limits                         Computes the representation
                                                            limits of a 32-bit integer
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
