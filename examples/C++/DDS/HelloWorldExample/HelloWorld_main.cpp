// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file HelloWorld_main.cpp
 *
 */

#include "HelloWorldPublisher.h"
#include "HelloWorldSubscriber.h"
#include "arg_configuration.h"

#include <string>

enum Type
{   publisher,
    subscriber
};

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

    std::cout << "Starting " << std::endl;
    Type type = publisher;
    std::string topic_name = "HelloWorldTopic";
    int count = 0;
    long sleep = 100;
    int num_wait_matched = 0;
    int domain = 0;
    bool async = false;
    std::string transport;
    bool reliable = false;
    bool transient = false; // transient local
    if (argc > 1)
    {
        if (!strcmp(argv[1], "publisher"))
        {
            type = publisher;
        }
        else if (!strcmp(argv[1], "subscriber"))
        {
            type = subscriber;
        }
        else if (!(strcmp(argv[1], "-h") && strcmp(argv[1], "--help")))
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return 0;
        }
        else
        {
            std::cerr << "ERROR: first argument can only be <publisher|subscriber>" << std::endl;
            option::printUsage(fwrite, stdout, usage, columns);
            return 1;
        }

        argc -= (argc > 0);
        argv += (argc > 0); // skip program name argv[0] if present
        --argc; ++argv; // skip pub/sub argument
        option::Stats stats(usage, argc, argv);
        std::vector<option::Option> options(stats.options_max);
        std::vector<option::Option> buffer(stats.buffer_max);
        option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);

        if (parse.error())
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return 1;
        }

        if (options[optionIndex::HELP])
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

                case optionIndex::TOPIC:
                    topic_name = std::string(opt.arg);
                    break;

                case optionIndex::DOMAIN:
                    domain = strtol(opt.arg, nullptr, 10);
                    break;

                case optionIndex::SAMPLES:
                    count = strtol(opt.arg, nullptr, 10);
                    break;

                case optionIndex::INTERVAL:
                    if (type == publisher)
                    {
                        sleep = strtol(opt.arg, nullptr, 10);
                    }
                    else
                    {
                        print_warning("publisher", opt.name);
                    }
                    break;

                case optionIndex::WAIT:
                    if (type == publisher)
                    {
                        num_wait_matched = strtol(opt.arg, nullptr, 10);
                    }
                    else
                    {
                        print_warning("publisher", opt.name);
                    }
                    break;

                case optionIndex::ASYNC:
                    if (type == publisher)
                    {
                        async = true;
                    }
                    else
                    {
                        print_warning("publisher", opt.name);
                    }
                    break;

                case optionIndex::TRANSPORT:
                    transport = std::string(opt.arg);
                    break;

                case optionIndex::RELIABLE:
                    reliable = true;
                    break;

                case optionIndex::TRANSIENT_LOCAL:
                    transient = true;
                    break;

                case optionIndex::UNKNOWN_OPT:
                    std::cerr << "ERROR: " << opt.name << " is not a valid argument." << std::endl;
                    option::printUsage(fwrite, stdout, usage, columns);
                    return 1;
                    break;
            }
        }
    }
    else
    {
        std::cerr << "ERROR: <publisher|subscriber> argument is required." << std::endl;
        option::printUsage(fwrite, stdout, usage, columns);
        return 1;
    }

    switch (type)
    {
        case publisher:
        {
            HelloWorldPublisher mypub;
            if (mypub.init(topic_name, static_cast<uint32_t>(domain), static_cast<uint32_t>(num_wait_matched), async,
                    transport, reliable, transient))
            {
                mypub.run(static_cast<uint32_t>(count), static_cast<uint32_t>(sleep));
            }
            break;
        }
        case subscriber:
        {
            HelloWorldSubscriber mysub;
            if (mysub.init(topic_name, static_cast<uint32_t>(count), static_cast<uint32_t>(domain), transport,
                    reliable, transient))
            {
                mysub.run(static_cast<uint32_t>(count));
            }
            break;
        }
    }
    return 0;
}
