# Advanced Configuration Example

This example extends the configuration options of a trivial HelloWorld by letting the user specify properties of
entities such as durability, reliability or specify the transport protocol to be used, among other possibilities. This
could be useful, for example, to quickly test whether two endpoints are compatible and hence would match.
Additionally, the message type includes a data sequence which size can be set by the user, allowing to send large data between endpoints.
Note: Due to the nature of the data type (not bounded), this example will not use data sharing.

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
                    Topic name (Default: AdvancedConfigurationTopic).
  -d <id>         --domain=<id>
                    DDS domain ID (Default: 0).
  -w <num>        --wait=<num>
                    Number of matched subscribers required to publish (Default:
                    0 => does not wait).
  -m <num>        --msg-size=<num>
                    Size in bytes of the data to send (Default 10).
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
                    Topic name (Default: AdvancedConfigurationTopic).
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
  -x <str>        --xml-profile <str>
                    Profile name to configure DomainParticipant.

Discovery options:
                  --ttl
                    Set multicast discovery Time To Live on IPv4 or Hop Limit
                    for IPv6. If not set, uses Fast-DDS default (1 hop).
                    Increase it to avoid discovery issues on scenarios with
                    several routers. Maximum: 255.
```

### XML Configuration

Using argument `--xml-profile <profile_name>` will configure the internal DomainParticipant using the profile name loaded from an XML file.
To load XML files check [Fast DDS documentation](https://fast-dds.docs.eprosima.com/en/latest/fastdds/xml_configuration/xml_configuration.html).
Loading example XML configuration [file](shm_off.xml) and calling this example with `--xml-profile no_shm_participant_profile` will disable Shared Memory Transport for the internal DomainParticipant created.

This code presents how to run a publisher with this example without Shared Memory:

```sh
# From AdvancedConfigurationExample installation dir
FASTRTPS_DEFAULT_PROFILES_FILE=shm_off.xml ./AdvancedConfigurationExample publisher --xml-profile no_shm_participant_profile
```
