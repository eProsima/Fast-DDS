// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "deadlinepayloadPublisher.h"
#include "deadlinepayloadSubscriber.h"


#include <fastrtps/Domain.h>
#include <fastrtps/log/Log.h>
#include "asio.hpp"

using namespace eprosima;
using namespace eprosima::fastrtps;
using namespace asio;

/**
 * @brief Parses command line arguments
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @param type Publisher or subscriber
 * @param deadline_ms Deadline value read from command line arguments (populated if specified)
 * @param sleep_ms Writer sleep read from command line arguments (populated if specified)
 * @param samples Number of samples read from command line arguments (populated if specified)
 * @return True if command line arguments were parsed succesfully and execution can continue
 */
bool parse_arguments(
        int argc,
        char** argv,
        int& type,
        int& deadline_ms,
        int& sleep_ms,
        int& samples)
{
    if (argc == 1)
    {
        // No arguments provided
        return false;
    }

    for (int i=0; i<argc; i++)
    {
        if (!strcmp(argv[i], "--help"))
        {
            // --help command found
            return false;
        }
    }

    if (strcmp(argv[1], "publisher") == 0)
    {
        type = 1;

        if (argc != 2 && argc != 4 && argc != 6 && argc != 8)
        {
            // Incorrect number of command line arguments for publisher
            return false;
        }

        int count = 2;
        while (count < argc)
        {
            if (!strcmp(argv[count], "--deadline"))
            {
                deadline_ms = atoi(argv[count + 1]);
            }
            else if (!strcmp(argv[count], "--sleep"))
            {
                sleep_ms = atoi(argv[count + 1]);
            }
            else if (!strcmp(argv[count], "--samples"))
            {
                samples = atoi(argv[count + 1]);
            }
            else
            {
                std::cout << "Unknown command line option " << argv[count] << " for publisher" << std::endl;
                return false;
            }
            count = count + 2;
        }
        return true;
    }

    if (strcmp(argv[1], "subscriber") == 0)
    {
        type = 2;

        if (argc != 2 && argc != 4)
        {
            // Incorrect number of command line arguments for subscriber
            return false;
        }

        if (argc > 2)
        {
            if (!strcmp(argv[2], "--deadline"))
            {
                deadline_ms = atoi(argv[3]);
            }
            else
            {
                std::cout << "Unknown command line option " << argv[2] << " for publisher" << std::endl;
                return false;
            }
        }
        return true;
    }

    return false;
}

int main(int argc, char** argv)
{
    int deadline_ms = 2000;
    int sleep_ms = 1000;
    int samples = 0;
    int type = 1;

    if (!parse_arguments(argc,
                         argv,
                         type,
                         deadline_ms,
                         sleep_ms,
                         samples))
    {
        std::cout << "Usage: " << std::endl;
        std::cout << argv[0] << " publisher [--deadline <deadline_ms>] [--sleep <writer_sleep_ms>] [--samples <samples>]" << std::endl;
        std::cout << "OR" << std::endl;
        std::cout << argv[0] << " subscriber [--deadline <deadline_ms>]" << std::endl;
        return 0;
    }

    // Register the type being used

    switch(type)
    {
    case 1:
    {
        deadlinepayloadPublisher mypub;
        if (mypub.init(deadline_ms))
        {
            mypub.run(sleep_ms, samples);
        }
        break;
    }
    case 2:
    {
        deadlinepayloadSubscriber mysub;
        if (mysub.init(deadline_ms))
        {
            mysub.run();
        }
        break;
    }
    }

    return 0;
}
