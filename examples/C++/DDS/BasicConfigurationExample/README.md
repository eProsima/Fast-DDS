# Basic Configuration Example

This example extends the configuration options of a trivial HelloWorld by letting the user specify properties of
entities such as durability, reliability or specify the transport protocol to be used, among other possibilites. This
could be useful, for example, to quickly test whether two endpoints are compatible and hence would match.

## Execution instructions

To launch this test open two different consoles:

In the first one launch: ./BasicConfigurationExample publisher (or BasicConfigurationExample.exe publisher on windows).
In the second one: ./BasicConfigurationExample subscriber (or BasicConfigurationExample.exe subscriber on windows).

## Arguments

First argument is `publisher` or `subscriber` and then the rest of arguments are read unordered

```sh
Usage: BasicConfigurationExample <publisher|subscriber>

General options:
  -h              --help                Produce help message.

Publisher options:
  -t <topic_name> --topic=<topic_name>  Topic name (Default: HelloWorldTopic).
  -d <id>         --domain=<id>         DDS domain ID (Default: 0).
  -w <num>        --wait=<num>          Number of matched subscribers required
                                        to publish(Default: 0 => does not wait).
  -s <num>        --samples=<num>       Number of samples to send (Default: 0 =>
                                        infinite samples).
  -i <num>        --interval=<num>      Time between samples in milliseconds
                                        (Default: 100).
  -a              --async               Asynchronous publish mode (synchronous
                                        by default).
                  --transport=<shm|udp> Use shared-memory|UDP transport
                                        (Default: data-sharing > shared-memory >
                                        UDP in this order of priority depending
                                        on execution context).

Subscriber options:
  -t <topic_name> --topic=<topic_name>  Topic name (Default: HelloWorldTopic).
  -d <id>         --domain=<id>         DDS domain ID (Default: 0).
  -s <num>        --samples=<num>       Number of samples to wait for (Default:
                                        0 => infinite samples).
                  --transport=<shm|udp> Use shared-memory|UDP transport
                                        (Default: data-sharing > shared-memory >
                                        UDP in this order of priority depending
                                        on execution context).

QoS options:
  -r              --reliable            Set reliability to reliable (best-effort
                                        by default).
                  --transient           Set durability to transient local
                                        (volatile by default, ineffective when
                                        not reliable).
```