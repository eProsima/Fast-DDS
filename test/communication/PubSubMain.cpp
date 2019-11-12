#include "Publisher.hpp"
#include "Subscriber.hpp"

#include <fastrtps/Domain.h>

int main(
        int argc,
        char** argv)
{
    int arg_count = 1;
    bool notexit = false;
    uint32_t seed = 7800, wait = 0;
    uint32_t samples = 4;
    uint32_t publishers = 1;
    char* xml_file = nullptr;
    std::string magic;

    while (arg_count < argc)
    {
        if (strcmp(argv[arg_count], "--notexit") == 0)
        {
            notexit = true;
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
        else if (strcmp(argv[arg_count], "--publishers") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--publishers expects a parameter" << std::endl;
                return -1;
            }

            publishers = strtol(argv[arg_count], nullptr, 10);
        }

        ++arg_count;
    }

    if (xml_file)
    {
        eprosima::fastrtps::Domain::loadXMLProfilesFile(xml_file);
    }

    Publisher publisher(false);
    Subscriber subscriber(publishers, samples);

    if (publisher.init(seed, magic))
    {
        if (subscriber.init(seed, magic))
        {
            if (wait > 0)
            {
                publisher.wait_discovery(wait);
            }

            do
            {
                publisher.run(samples, 1);
            }
            while (!subscriber.run_for(notexit, std::chrono::milliseconds(100)));

            return 0;
        }
    }

    return -1;
}
