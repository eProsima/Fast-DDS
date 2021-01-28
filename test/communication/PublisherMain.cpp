#include "Publisher.hpp"

#include <fastrtps/Domain.h>

int main(
        int argc,
        char** argv)
{
    int arg_count = 1;
    bool exit_on_lost_liveliness = false;
    bool fixed_type = false;
    uint32_t seed = 7800, wait = 0;
    char* xml_file = nullptr;
    uint32_t samples = 4;
    std::string magic;

    while (arg_count < argc)
    {
        if (strcmp(argv[arg_count], "--exit_on_lost_liveliness") == 0)
        {
            exit_on_lost_liveliness = true;
        }
        else if (strcmp(argv[arg_count], "--fixed_type") == 0)
        {
            std::cout << "--fixed_type set: using FixedSizedType" << std::endl;
            fixed_type = true;
        }
        else if (strcmp(argv[arg_count], "--seed") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--seed expects a parameter" << std::endl;
                return -1;
            }

            seed = strtol(argv[arg_count], nullptr, 10);
        }
        else if (strcmp(argv[arg_count], "--wait") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--wait expects a parameter" << std::endl;
                return -1;
            }

            wait = strtol(argv[arg_count], nullptr, 10);
        }
        else if (strcmp(argv[arg_count], "--samples") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--samples expects a parameter" << std::endl;
                return -1;
            }

            samples = strtol(argv[arg_count], nullptr, 10);
        }
        else if (strcmp(argv[arg_count], "--magic") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--magic expects a parameter" << std::endl;
                return -1;
            }

            magic = argv[arg_count];
        }
        else if (strcmp(argv[arg_count], "--xmlfile") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--xmlfile expects a parameter" << std::endl;
                return -1;
            }

            xml_file = argv[arg_count];
        }
        else
        {
            std::cout << "Wrong argument " << argv[arg_count] << std::endl;
            return -1;
        }

        ++arg_count;
    }

    if (xml_file)
    {
        eprosima::fastrtps::Domain::loadXMLProfilesFile(xml_file);
    }

    Publisher publisher(exit_on_lost_liveliness);

    if (publisher.init(seed, magic, fixed_type))
    {
        if (wait > 0)
        {
            publisher.wait_discovery(wait);
        }

        publisher.run(samples);
        return 0;
    }

    return -1;
}
