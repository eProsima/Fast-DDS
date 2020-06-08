eProsima Server-Client discovery auxiliary generator tool version 1.0.0

Usage: fast-discovery-server -id {0-255} [optional parameters]
General options:
  -h  --help       Produce help message.

  -i  --server-id  Mandatory unique server identifier. Specifies zero based
                   server position in ROS_DISCOVERY_SERVER environment variable.

  -l  --ip-address Server interface chosen to listen the clients. Defaults
                   to any (0.0.0.0)

  -p  --port       UDP port chosen to listen the clients. Defaults to 11811

  -b  --backup     Creates a server with a backup file associated.

Examples:
      1. Launch a default server with id 0 (first on ROS_DISCOVERY_SERVER)
         listening on all available interfaces on UDP port 11811. Only one
         server can use default values per machine.

      $ fast-discovery-server -i 0

      2. Launch a default server with id 1 (second on ROS_DISCOVERY_SERVER)
         listening on localhost with UDP port 14520. Only localhost clients
         can reach the server using as ROS_DISCOVERY_SERVER=;127.0.0.1:14520

      $ fast-discovery-server -i 1 -l 127.0.0.1 -p 14520

      3. Launch a default server with id 3 (third on ROS_DISCOVERY_SERVER)
         listening on Wi-Fi (192.168.36.34) and Ethernet (172.20.96.1) local
         interfaces with UDP ports 8783 and 51083 respectively
         (addresses and ports are made up for the example).

      $ fast-discovery-server -i 1 -l 192.168.36.34 -p 14520 -l 172.20.96.1 -p 51083

      4. Launch a default server with id 4 (fourth on ROS_DISCOVERY_SERVER)
         listening on 172.30.144.1 with UDP port 12345 and provided with a
         backup file. If the server crashes it will automatically restore its
         previous state when reenacted.

      $ fast-discovery-server -i 1 -l 172.30.144.1 -p 12345 -b

