## Peer-to-peer lease duration test

This test demonstrates the Peer-to-peer participant lease duration assessment capabilities.
This can be enabled for a participant by adding the propagating `ds_p2p_lease_assessment` user property to the participant UserPropertyQos.
Using the Fast DDS' RTPS API, this can be done like so:

```cpp

RTPSParticipantAttributes PParam;
eprosima::fastrtps::rtps::RTPSParticipant* mp_participant;
PParam.builtin.discovery_config.leaseDuration = 10.0;
PParam.builtin.discovery_config.leaseDuration_announcementperiod = 1.0;
PParam.properties.properties().emplace_back("ds_p2p_lease_assessment","true","true");
mp_participant = RTPSDomain::createParticipant(0, PParam);

```

### Test configuration

This test uses an application with the required modifications to assert its lease duration on a peer-to-peer basis as well as two Fast DDS Discovery Servers using XML Profiles.

Client-Server Layout is as follows:

```
| Reader Client | ---> | Server 1 | ---> | Server 2 | <--- | Writer Client |
```

Both the Reader and the Writer clients can be configured to use peer-to-peer lease assessment via a CLI option.


### Building this example

There are several ways in which this example can be built.

#### Build alongside Fast DDS

To build and install all the examples while building Fast DDS, make sure to pass both the `-DCOMPILE_EXAMPLES=ON` and `-DINSTALL_EXAMPLES=ON` options to CMake.

#### Build standalone

From a terminal containing `fastrtps-config.cmake` in the `$PATH` and `libfastrtps` in the `$LD_LIBRARY_PATH`, run:

```bash
cd <this directory>
mkdir build && cd build
cmake ..
cmake --build .
```

### Launching Reader/Writer application

To launch the example application, from a terminal run:

```bash
# Writer mode
./Registered writer
```
```bash
# Reader mode
./Registered reader
```

Running the `Registered` application with no arguments provides a list of all available parameters.

### Complete test case

This section describes how to setup the 2 Discovery Servers - 2 Clients Demo testing scenario for the peer-to-peer lease assessment feature.
This test is run from four terminals, two will be launching the Discovery Servers while the other two will be launching the `Registered` application with different parameters.
All step-by-step instructions for each terminal assume that the `CMAKE_INSTALL_PREFIX` folder for Fast DDS is the starting location. If using Colcon this will be the `install/fastrtps` folder.
Profile files for the Discovery Servers will be located in `examples/cpp/rtps/Registered`.

#### Terminal 1 - Discovery Server 1

```bash

cd bin
source ../../setup.bash
./fast-discovery-server -x <path_to_DS1Profile.xml> (with the default paths, ../examples/cpp/rtps/Registered/DS1Profile.xml)
```

#### Terminal 2 - Discovery Server 2

```bash

cd bin
source ../../setup.bash
./fast-discovery-server -x <path_to_DS2Profile.xml> (with the default paths, ../examples/cpp/rtps/Registered/DS2Profile.xml)
```

#### Terminal 3 - Reader Client

Note: if ROS_DISCOVERY_SERVER is not exported, the discovery mode is set to SIMPLE

```bash

source ../setup.bash
cd  examples/cpp/rtps/Registered/bin
export ROS_DISCOVERY_SERVER="127.0.0.1:14520"
./Registered reader --ds-p2p

```

#### Terminal 4 - Writer Client

Note: if ROS_DISCOVERY_SERVER is not exported, the discovery mode is set to SIMPLE

```bash

source ../setup.bash
cd  examples/cpp/rtps/Registered/bin
export ROS_DISCOVERY_SERVER=";127.0.0.1:14521"
./Registered writer -s 1000 -i 1000
```

#### Test results

Matching and unmatching of participants is shown on the terminals with outputs such as this one:

```
Participant with name RTPSParticipant and GUID 01.0f.bf.dc.6c.18.1c.d8.01.00.00.00|0.0.1.c1 matched
```

Both clients and servers are configured to have a lease duration of 10 seconds, with announcements every second.

Sending a SIGINT signal to the Discovery Server 1 process to kill it (via the use of the `kill` command) will show the following trace on both the Discovery Server 2 and the Reader process when 10 seconds have passed.

```
Participant with name RTPSParticipant and GUID 44.53.00.5f.45.50.52.4f.53.49.4d.41|0.0.1.c1 dropped.
```

Reader-Writer communication will not be impacted.

Killing the Reader process via Ctrl+C or another kill command will have similar results when 10 seconds have passed.

```
Participant with name RTPSParticipant and GUID 01.0f.bf.dc.26.18.8f.27.01.00.00.00|0.0.1.c1 dropped.
```

Writer process will be unmatched when the lease duration expires.