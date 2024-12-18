eProsima Discovery Server auxiliary generator tool. Version 1.0.1
It can be used for both deploying Servers and inspecting active ones.

Usage: fastdds discovery [optional parameters]

General options:
  -h  --help        Produce help message.

  -v  --version     Show Fast DDS version information.

  -l  --udp-address IPv4/IPv6 address chosen to listen the clients.
                    Defaults to any (0.0.0.0/::0). Instead of an
                    address, a name can be specified.

  -p  --udp-port    UDP port chosen to listen the clients. Defaults to
                    11811.

  -t  --tcp-address IPv4/IPv6 address chosen to listen the clients
                    using TCP transport. Defaults to any
                    (0.0.0.0/::0). Instead of an address, a name
                    can be specified.

  -q  --tcp-port    TCP port chosen to listen the clients. Defaults to
                    42100.

  -b  --backup      Creates a server with a backup file associated.

  -x  --xml-file    Gets config from XML file. If there is any
                    argument in common with the config of the XML, the
                    XML argument will be overriden. A XML file with
                    several profiles will take the profile with
                    "is_default_profile="true"" unless another
                    profile using uri with "@" character is defined.

  -i  --server-id   Unique server identifier. Its functionality its
                    deprecated. It can be used to select a fixed GUID.

  -e  --examples    List usage examples of eProsima Discovery Server
                    tool.

Daemon options:
  auto          Handles the daemon start-up automatically.

  start         Starts the Discovery Server daemon with the remote connections
                specified. Example: "start -d 2 127.0.0.1:3"

  stop          Stops the Discovery Server daemon if it is active.

  add           Adds new remotes Discovery Servers to the local server. This
                will connect both servers and their sub-networks without
                modifying existing remote servers.

  set           Rewrite the remotes Discovery Servers connected to the local
                server. This will replace existing remote servers with the new
                connections.

  list          List local active discovery servers created with the CLI Tool.

  Daemon parameters:

  -d  --domain      Selects the domain of the server to target for this action.
                    It is equivalent to specify the domain with the ROS_DOMAIN_ID
                    environment variable.
