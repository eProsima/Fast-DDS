// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file PuckApp_main.cpp
 *
 */

#include <string>

#include "arg_configuration.h"
#include "PuckAppPublisher.h"
#include "PuckAppSubscriber.h"

int main(
        int argc,
        char** argv)
{
    int columns;

#if defined(_WIN32)
    char* buf = nullptr;
    size_t sz = 0;
    if (_dupenv_s(&buf, &sz, "COLUMNS") == 0 && buf != nullptr)
    {
        columns = strtol(buf, nullptr, 10);
        free(buf);
    }
    else
    {
        columns = 80;
    }
#else
    columns = getenv("COLUMNS") ? atoi(getenv("COLUMNS")) : 80;
#endif // if defined(_WIN32)

    std::string topic_name = "HelloWorldTopic";
    int domain = 0;

    // Discovery Server
    std::cmatch mr;
    std::string server_address = "127.0.0.1";   // default ip address
    uint16_t server_port = 60006;   // default physical port

    if (argc > 1)
    {
        // check if first argument is help, needed because we skip it when parsing
        if (!(strcmp(argv[1], "-h") && strcmp(argv[1], "--help")))
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return 0;
        }

        argc -= (argc > 0);
        argv += (argc > 0); // skip program name argv[0] if present
        option::Stats stats(usage, argc, argv);
        std::vector<option::Option> options(stats.options_max);
        std::vector<option::Option> buffer(stats.buffer_max);
        option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);

        if (parse.error())
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return 1;
        }

        if (options[HELP])
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return 0;
        }

        for (int i = 0; i < parse.optionsCount(); ++i)
        {
            option::Option& opt = buffer[i];
            switch (opt.index())
            {
                case optionIndex::HELP:
                    // not possible, because handled further above and exits the program
                    break;

                case optionIndex::DOMAIN_ID:
                    domain = strtol(opt.arg, nullptr, 10);
                    break;

                case optionIndex::SERVER_LOCATOR:
                    if (regex_match(opt.arg, mr, Arg::ipv4))
                    {
                        std::cmatch::iterator it = mr.cbegin();
                        server_address = (++it)->str();
                        if (server_address.empty())
                        {
                            std::cerr << "ERROR: " << opt.arg << " is an invalid server address." << std::endl;
                            option::printUsage(fwrite, stdout, usage, columns);
                            return 1;
                        }

                        if ((++it)->matched)
                        {
                            int port_int = std::stoi(it->str());
                            if (port_int > 1000 && port_int <= 65535)
                            {
                                server_port = static_cast<uint16_t>(port_int);
                            }
                            else
                            {
                                std::cerr << "ERROR: " << port_int << " is an invalid port number." << std::endl;
                                option::printUsage(fwrite, stdout, usage, columns);
                                return 1;
                            }
                        }
                    }
                    break;

                case optionIndex::UNKNOWN_OPT:
                    std::cerr << "ERROR: " << opt.name << " is not a valid argument." << std::endl;
                    option::printUsage(fwrite, stdout, usage, columns);
                    return 1;
                    break;
            }
        }
    }

    PuckAppPublisher mypub;
    PuckAppSubscriber mysub;

    if (!mypub.init(topic_name, server_address, server_port))
    {
        return 1;
    }

    if (!mysub.init(topic_name, static_cast<uint32_t>(domain), &mypub))
    {
        return 1;
    }

    mysub.run();

    return 0;
}
