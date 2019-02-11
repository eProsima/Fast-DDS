To launch this test open two different consoles:

In the first one launch: HelloWorldExampleTCP publisher (or HelloWorldExampleTCP.exe publisher on windows).
In the second one: HelloWorldExampleTCP subscriber.


This example includes additional options to show the capabilities of the TCP Transport on Fast-RPTS,
such as WAN and TLS.

Usage: HelloWorldExampleTCP <publisher|subscriber>

General options:
  -h            --help              Produce help message.
  -t            --tls               Use TLS.

Publisher options:
  -s <num>,     --samples=<num>     Number of samples (0, default, infinite).
  -i <num>,     --interval=<num>    Time between samples in milliseconds
                                    (Default: 100).
  -a <address>, --address=<address> Public IP Address of the publisher.
  -p <num>,     --port=<num>        Physical Port to listening incoming
                                    connections.

Subscriber options:
  -a <address>, --address=<address> IP Address of the publisher.
  -p <num>,     --port=<num>        Physical Port where the publisher is
                                    listening for connections.


WAN Example:

HelloWorldExampleTCP publisher -a <PUBLIC_WAN_ADDR> -p <PORT>
HelloWorldExampleTCP subscriber -a <SERVER_ADDR> -p <PORT>

    For example:
        HelloWorldExampleTCP publisher -a 80.88.150.120 -p 5500
        HelloWorldExampleTCP subscriber -a 80.88.150.120 -p 5500


TLS Example:

HelloWorldExampleTCP publisher -t
HelloWorldExampleTCP subscriber -t