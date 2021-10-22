# Helloworld Example Discovery Server

This example demonstrates how communication between a publisher and subscriber can be established through the Discovery
Server mechanism.

## Execution instructions

To launch this test open three different consoles:

In the first one launch: ./HelloWorldExampleDiscoveryServer publisher (or HelloWorldExampleDiscoveryServer.exe publisher on windows).
In the second one: ./HelloWorldExampleDiscoveryServer subscriber (or HelloWorldExampleDiscoveryServer.exe subscriber on windows).
In the third one: ./HelloWorldExampleDiscoveryServer server (or HelloWorldExampleDiscoveryServer.exe server on windows).

## Arguments

First argument is `publisher`, `subscriber` or `server` and then the rest of arguments are read unordered

```sh
Usage: HelloWorldExampleDiscoveryServer <publisher|subscriber|server>

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
