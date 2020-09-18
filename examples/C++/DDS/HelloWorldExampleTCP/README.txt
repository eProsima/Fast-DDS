To launch this test open two different consoles:

In the first one launch: ./DDSHelloWorldExampleTCP publisher (or DDSHelloWorldExampleTCP.exe publisher on windows).
In the second one: ./DDSHelloWorldExampleTCP subscriber (or DDSHelloWorldExampleTCP.exe subscriber on windows).


This example includes additional options to show the capabilities of the TCP Transport on Fast DDS,
such as WAN and TLS. In this example the publisher will work as a TCP server and the subscriber as a
TCP client.

Usage: DDSHelloWorldExampleTCP <publisher|subscriber>

General options:
  -h            --help              Produce help message.
  -t            --tls               Use TLS.

Publisher options:
  -s <num>,     --samples=<num>     Number of samples (0, default, infinite).
  -i <num>,     --interval=<num>    Time between samples in milliseconds
                                    (Default: 100).
  -a <address>, --address=<address> Public IP Address of the publisher (Default: None).
  -p <num>,     --port=<num>        Physical Port to listening incoming
                                    connections (Default: 5100).

Subscriber options:
  -a <address>, --address=<address> IP Address of the publisher (Default: 127.0.0.1).
  -p <num>,     --port=<num>        Physical Port where the publisher is
                                    listening for connections (Default: 5100).


WAN Example:

DDSHelloWorldExampleTCP publisher -a <PUBLIC_WAN_ADDR> -p <PORT>
DDSHelloWorldExampleTCP subscriber -a <SERVER_ADDR> -p <PORT>

    For example:
        DDSHelloWorldExampleTCP publisher -a 80.88.150.120 -p 5500
        DDSHelloWorldExampleTCP subscriber -a 80.88.150.120 -p 5500


TLS Example:

DDSHelloWorldExampleTCP publisher -t
DDSHelloWorldExampleTCP subscriber -t
