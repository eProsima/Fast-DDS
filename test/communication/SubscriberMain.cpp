#include "Subscriber.hpp"

#include <fastrtps/Domain.h>

int main(
        int argc,
        char** argv)
{
    int arg_count = 1;
    bool notexit = false;
    bool fixed_type = false;
    uint32_t seed = 7800;
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
        else if (strcmp(argv[arg_count], "--fixed") == 0)
        {
            std::cout << "--fixed set: using FixedSizedType" << std::endl;
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

    Subscriber subscriber(publishers, samples);

    if (subscriber.init(seed, magic, fixed_type))
    {
        return subscriber.run(notexit) ? 0 : -1;
    }

    return -1;
}
