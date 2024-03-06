#include <iostream>

struct subscriber_config
{
    bool use_waitset;
};

struct hello_world_config
{
    std::string entity;
    subscriber_config sub_config;
};


void print_help()
{
    std::cout << "Usage: hello_world [entity] [options]\n"
              << "Entities:                       Mandatory field\n"
              << "  publisher\n"
              << "  subscriber\n"
              << "Options:                        Optional fields:\n"
              << "  -w, --waitset                 Use waitset read condition (subscriber only)\n"
              << "  -h, --help                    Print this help message\n";
}

hello_world_config parse_cli_options (
        int argc,
        char* argv[])
{
    hello_world_config config;
    config.entity = "";
    config.sub_config.use_waitset = false;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help")
        {
            print_help();
            exit(EXIT_SUCCESS);
        }
        else if (arg == "publisher" || arg == "subscriber")
        {
            config.entity = arg;
        }
        else if (arg == "-w" || arg == "--waitset")
        {
            if (config.entity == "subscriber")
            {
                config.sub_config.use_waitset = true;
            }
            else
            {
                EPROSIMA_LOG_ERROR(CLI_PARSE, "waitset can only be used with the subscriber entity");
                print_help();
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(CLI_PARSE, "unknown option " + arg);
            print_help();
            exit(EXIT_FAILURE);
        }
    }

    if (config.entity == "")
    {
        EPROSIMA_LOG_ERROR(CLI_PARSE, "entity not specified");
        print_help();
        exit(EXIT_FAILURE);
    }

    return config;
}
