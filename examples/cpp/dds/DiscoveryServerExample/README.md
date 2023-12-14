# Discovery Server Example

This example demonstrates how communication between a publisher and subscriber can be established through the Discovery
Server mechanism.

## Execution instructions

To launch this test open three different consoles:

In the first one launch: ./DiscoveryServerExample publisher (or DiscoveryServerExample.exe publisher on windows).
In the second one: ./DiscoveryServerExample subscriber (or DiscoveryServerExample.exe subscriber on windows).
In the third one: ./DiscoveryServerExample server (or DiscoveryServerExample.exe server on windows).

## Arguments

First argument is `publisher`, `subscriber` or `server` and then the rest of arguments are read unordered

```sh
Usage: DiscoveryServerExample <publisher|subscriber|server>

General options:
  -h              --help
                    Produce help message.

Publisher options:
  -t <topic_name> --topic=<topic_name>
                    Topic name (Default: HelloWorldTopic).
  -s <num>        --samples=<num>
                    Number of samples to send (Default: 0 => infinite samples).
  -i <num>        --interval=<num>
                    Time between samples in milliseconds (Default: 100).
  -c <IPaddress>  --connection-address=<IPaddress>
                    Server address (Default address: 127.0.0.1).
  -p <num>        --connection-port=<num>
                    Server listening port (Default port: 16166).
                  --transport=<udpv4|udpv6|tcpv4|tcpv6>
                    Use Transport Protocol [udpv4|udpv6|tcpv4|tcpv6] (Default: udpv4).
  -d <num>        --connection-discovery-server-id <num>
                    Id of the Discovery Server to connect with. GUID will be
                    calculated from id (Default: 0).

Subscriber options:
  -t <topic_name> --topic=<topic_name>
                    Topic name (Default: HelloWorldTopic).
  -s <num>        --samples=<num>
                    Number of samples to wait for (Default: 0 => infinite
                    samples).
  -c <IPaddress>  --connection-address=<IPaddress>
                    Server address (Default address: 127.0.0.1).
  -p <num>        --connection-port=<num>
                    Server listening port (Default port: 16166).
                  --transport=<udpv4|udpv6|tcpv4|tcpv6>
                    Use Transport Protocol [udpv4|udpv6|tcpv4|tcpv6] (Default: udpv4).
  -d <num>        --connection-discovery-server-id <num>
                    Id of the Discovery Server to connect with. GUID will be
                    calculated from id (Default: 0).

DiscoveryServer options:
                  --listening-address=<IPaddress>
                    Server address (Default address: 127.0.0.1).
                  --id=<num>
                    Id of this Discovery Server.
                    GUID will be calculated from id (Default: 0).
                  --listening-port=<num>
                    Server listening port (Default port: 16166).
                  --transport=<udpv4|udpv6|tcpv4|tcpv6>
                    Use Transport Protocol [udpv4|udpv6|tcpv4|tcpv6] (Default: udpv4).
  -c <IPaddress>  --connection-address=<IPaddress>
                    Server address (Default address: 127.0.0.1).
  -p <num>        --connection-port=<num>
                    Server listening port (Default port: 16166).
  -d <num>        --connection-discovery-server-id <num>
                    Id of the Discovery Server to connect with. GUID will be
                    calculated from id (if not set, this DS will not connect
                    to other server).
  -z <num>        --timeout <num>
                    Number of seconds before finish the process (Default: 0 = till ^C).
```
