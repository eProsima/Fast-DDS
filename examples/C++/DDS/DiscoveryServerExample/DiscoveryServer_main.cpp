#include <iostream>
#include <string>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/ServerAttributes.h>

#include "DiscoveryServer_Server.hpp"
#include "DiscoveryServer_Client.hpp"

using eprosima::fastdds::dds::Log;

int main(
        int argc,
        char** argv)
{
    Log::SetVerbosity(Log::Kind::Info);
    Log::SetCategoryFilter(std::regex("RTPS_QOS_CHECK"));
    std::cout << "Starting DiscoveryServer Example" << std::endl;
    int type = 1;
    std::string locator;
    std::string guid_prefix = eprosima::fastdds::rtps::DEFAULT_ROS2_SERVER_GUIDPREFIX;

    if (argc > 1)
    {
        if (strcmp(argv[1], "server") == 0)
        {
            type = 1;
            if (argc >= 3)
            {
                locator = argv[2];
                std::cout << "Server listening locator: " << locator << std::endl;
                if (argc == 4)
                {
                    guid_prefix = argv[3];
                    std::cout << "Server guid prefix: " << guid_prefix << std::endl;
                }
            }
            else
            {
                std::cout << "server option requires listening address: IPaddress | IPaddress:port" << std::endl;
            }
        }
        else if (strcmp(argv[1], "client") == 0)
        {
            type = 2;
        }
        else
        {
            std::cout << "Incorrect option. server or client argument needed" << std::endl;
            return 0;
        }

    }
    else
    {
        std::cout << "server or client argument needed" << std::endl;
        Log::Reset();
        return 0;
    }

    switch (type)
    {
        case 1:
            {
                DiscoveryServer_Server server;
                if (server.init(locator, guid_prefix))
                {
                    server.run();
                }
                break;
            }
        case 2:
            {
                DiscoveryServer_Client client;
                if (client.init())
                {
                    client.run();
                }
                break;
            }
        default:
            break;
    }
    Log::Reset();
    return 0;
}
