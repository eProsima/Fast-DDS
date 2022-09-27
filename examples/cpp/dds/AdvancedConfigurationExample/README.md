# Advanced Configuration Example

This example extends the configuration options of a trivial HelloWorld by letting the user specify properties of
entities such as durability, reliability or specify the transport protocol to be used, among other possibilities. This
could be useful, for example, to quickly test whether two endpoints are compatible and hence would match.

## Execution instructions

To launch this test open two different consoles:

In the first one launch: ./AdvancedConfigurationExample publisher (or AdvancedConfigurationExample.exe publisher on windows).
In the second one: ./AdvancedConfigurationExample subscriber (or AdvancedConfigurationExample.exe subscriber on windows).

## Arguments

First argument is `publisher` or `subscriber` and then the rest of arguments are read unordered

```sh
Usage: AdvancedConfigurationExample <publisher|subscriber>

General options:
  -h              --help
                    Produce help message.

Publisher options:
  -t <topic_name> --topic=<topic_name>
                    Topic name (Default: HelloWorldTopic).
  -d <id>         --domain=<id>
                    DDS domain ID (Default: 0).
  -w <num>        --wait=<num>
                    Number of matched subscribers required to publish (Default:
                    0 => does not wait).
  -s <num>        --samples=<num>
                    Number of samples to send (Default: 0 => infinite samples).
  -i <num>        --interval=<num>
                    Time between samples in milliseconds (Default: 100).
  -a              --async
                    Asynchronous publish mode (synchronous by default).
                  --transport=<shm|udp|udpv6>
                    Use only shared-memory, UDPv4, or UDPv6 transport.If not
                    set, use Fast DDS default transports (depending on the
                    scenario it will use the most efficient one: data-sharing
                    delivery mechanism > shared-memory > UDP).
  -o              --ownership
                    Use Topic with EXCLUSIVE_OWNERSHIP (SHARED_OWNERSHIP by
                    default).
                  --strength=<num>
                    Set this Publisher strength. Set Topic with
                    EXCLUSIVE_OWNERSHIP. Default: 0

Subscriber options:
  -t <topic_name> --topic=<topic_name>
                    Topic name (Default: HelloWorldTopic).
  -d <id>         --domain=<id>
                    DDS domain ID (Default: 0).
  -s <num>        --samples=<num>
                    Number of samples to wait for (Default: 0 => infinite
                    samples).
                  --transport=<shm|udp|udpv6>
                    Use only shared-memory, UDPv4, or UDPv6 transport.If not
                    set, use Fast DDS default transports (depending on the
                    scenario it will use the most efficient one: data-sharing
                    delivery mechanism > shared-memory > UDP).
  -o              --ownership
                    Use Topic with EXCLUSIVE_OWNERSHIP (SHARED_OWNERSHIP by
                    default).

QoS options:
  -r              --reliable
                    Set reliability to reliable (best-effort by default).
                  --transient
                    Set durability to transient local (volatile by default,
                    ineffective when not reliable).
  -p <str>        --partitions=<str>
                    Partitions to match separated by ';'. Single or double
                    quotes required with multiple partitions. With empty string
                    ('') no partitions used. (Default: '').

Discovery options:
                  --ttl
                    Set multicast discovery Time To Live on IPv4 or Hop Limit
                    for IPv6. If not set, uses Fast-DDS default (1 hop).
                    Increase it to avoid discovery issues on scenarios with
                    several routers. Maximum: 255.
```
