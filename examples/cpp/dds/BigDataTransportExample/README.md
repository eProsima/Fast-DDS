# BigData Configuration Example

This example reuses the code of the Advanced Configuration Example to test large data communication using UDP for Metatraffic Multicast and TCP for Metatraffic Unicast and Default Unicast.

## Execution instructions

To launch this test open two different consoles:

In the first one launch: FASTRTPS_DEFAULT_PROFILES_FILE=/home/carlos/fastdds_ws/src/fastrtps/examples/cpp/dds/BigDataTransportExample/participant_config_pub.xml ./BigDataTransportExample publisher --xml-profile bigdata_pub_profile -i 10000
In the second one: FASTRTPS_DEFAULT_PROFILES_FILE=/home/carlos/fastdds_ws/src/fastrtps/examples/cpp/dds/BigDataTransportExample/participant_config_sub.xml ./BigDataTransportExample subscriber --xml-profile bigdata_sub_profile


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
