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
                  --connection-address=<IPaddress[:port number]>
                    Server address (Default address: 127.0.0.1, default port:
                    60006).
                  --tcp
                    Use TCP transport (UDP by default).

Subscriber options:
  -t <topic_name> --topic=<topic_name>
                    Topic name (Default: HelloWorldTopic).
  -s <num>        --samples=<num>
                    Number of samples to wait for (Default: 0 => infinite
                    samples).
                  --connection-address=<IPaddress[:port number]>
                    Server address (Default address: 127.0.0.1, default port:
                    60006).
                  --tcp
                    Use TCP transport (UDP by default).

DiscoveryServer options:
                  --listening-address=<IPaddress[:port number]>
                    Server address (Default address: 127.0.0.1, default port:
                    60006).
                  --tcp
                    Use TCP transport (UDP by default).
```
