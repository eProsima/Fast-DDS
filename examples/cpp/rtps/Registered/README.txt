To launch this test open two different consoles:

In the first one launch: ./Registered writer (or Registered.exe writer on windows).
In the second one: ./Registered reader (or Registered.exe reader on windows).

** 2 Discovery servers - 2 Clients Example Case **

This section describes how to setup the 2 DS 2 Clients Demo testing scenario for
the p2p_lease_assesment feature.

Terminal 1
-----------

Run inside .../install/fastrtps/bin :

source ../../setup.bash

./fast-discovery-server -x <path_to_DS1Profile.xml>

Terminal 2
-----------

Run inside .../install/fastrtps/bin :

source ../../setup.bash

./fast-discovery-server -x <path_to_DS2Profile.xml>

Terminal 3
-----------

Note: if ROS_DISCOVERY_SERVER is not exported, the discovery mode is set to SIMPLE

source .../install/setup.bash

Execute the commands in .../install/fastrtps/examples/cpp/rtps/Registered/bin

export ROS_DISCOVERY_SERVER="127.0.0.1:14520"

./Registered reader

Terminal 4
-----------

Note: if ROS_DISCOVERY_SERVER is not exported, the discovery mode is set to SIMPLE

source .../install/setup.bash

Execute the commands in .../install/fastrtps/examples/cpp/rtps/Registered/bin

export ROS_DISCOVERY_SERVER=";127.0.0.1:14521"

./Registered writer -s 10 -i 1000


