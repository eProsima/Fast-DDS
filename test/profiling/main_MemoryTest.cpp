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

#include <bitset>
#include <cstdint>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

#include <optionparser.hpp>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include "MemoryTestPublisher.h"
#include "MemoryTestSubscriber.h"

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable:4512)
#endif // if defined(_MSC_VER)

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

#if FASTDDS_IS_BIG_ENDIAN_TARGET
const Endianness_t DEFAULT_ENDIAN = BIGEND;
#else
const Endianness_t DEFAULT_ENDIAN = LITTLEEND;
#endif // if FASTDDS_IS_BIG_ENDIAN_TARGET

#if defined(_WIN32)
#define COPYSTR strcpy_s
#else
#define COPYSTR strcpy
#endif // if defined(_WIN32)

namespace option = eprosima::option;

struct Arg : public option::Arg
{
    static void printError(
            const char* msg1,
            const option::Option& opt,
            const char* msg2)
    {
        fprintf(stderr, "%s", msg1);
        fwrite(opt.name, opt.namelen, 1, stderr);
        fprintf(stderr, "%s", msg2);
    }

    static option::ArgStatus Unknown(
            const option::Option& option,
            bool msg)
    {
        if (msg)
        {
            printError("Unknown option '", option, "'\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Required(
            const option::Option& option,
            bool msg)
    {
        if (option.arg != 0 && option.arg[0] != 0)
        {
            return option::ARG_OK;
        }

        if (msg)
        {
            printError("Option '", option, "' requires an argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Numeric(
            const option::Option& option,
            bool msg)
    {
        char* endptr = 0;
        if (option.arg != 0 && strtol(option.arg, &endptr, 10))
        {
        }
        if (endptr != option.arg && *endptr == 0)
        {
            return option::ARG_OK;
        }

        if (msg)
        {
            printError("Option '", option, "' requires a numeric argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus String(
            const option::Option& option,
            bool msg)
    {
        if (option.arg != 0)
        {
            return option::ARG_OK;
        }
        if (msg)
        {
            printError("Option '", option, "' requires a numeric argument\n");
        }
        return option::ARG_ILLEGAL;
    }

};

enum  optionIndex
{
    UNKNOWN_OPT,
    HELP,
    RELIABILITY,
    SAMPLES,
    SEED,
    SUBSCRIBERS,
    ECHO_OPT,
    HOSTNAME,
    EXPORT_CSV,
    EXPORT_PREFIX,
    USE_SECURITY,
    CERTS_PATH,
    XML_FILE,
    DATA_SIZE,
    DYNAMIC_TYPES,
    TIME
};

const option::Descriptor usage[] = {
    { UNKNOWN_OPT, 0, "", "",                Arg::None,
      "Usage: MemoryTest <publisher|subscriber>\n\nGeneral options:" },
    { HELP,    0, "h", "help",               Arg::None,      "  -h \t--help  \tProduce help message." },
    { RELIABILITY, 0, "r", "reliability",      Arg::Required,
      "  -r <arg>, \t--reliability=<arg>  \tSet reliability (\"reliable\"/\"besteffort\")."},
    { SAMPLES, 0, "s", "samples",              Arg::Numeric,   "  -s <num>, \t--samples=<num>  \tNumber of samples." },
    { SEED, 0, "", "seed",                     Arg::Numeric,   "  \t--seed=<num>  \tNumber of subscribers." },
    { TIME, 0, "t", "time",                   Arg::Numeric,
      "  -t <num>, \t--time=<num>  \tTime of the test in seconds." },
    { UNKNOWN_OPT, 0, "", "",                Arg::None,      "\nPublisher options:"},
    { SUBSCRIBERS, 0, "n", "subscribers",      Arg::Numeric,
      "  -n <num>,   \t--subscribers=<arg>  \tSeed to calculate domain and topic, to isolate test." },
    { UNKNOWN_OPT, 0, "", "",                Arg::None,      "\nSubscriber options:"},
    { ECHO_OPT, 0, "e", "echo",               Arg::Required,
      "  -e <arg>, \t--echo=<arg>  \tEcho mode (\"true\"/\"false\")." },
    { HOSTNAME, 0, "", "hostname",             Arg::None,      "" },
    { EXPORT_CSV, 0, "", "export_csv",         Arg::None,      "" },
    { EXPORT_PREFIX, 0, "", "export_prefix",   Arg::String,    "\t--export_prefix \tFile prefix for the CSV file." },
#if HAVE_SECURITY
    {
        USE_SECURITY, 0, "", "security",      Arg::Required,
        "  --security <arg>  \tEcho mode (\"true\"/\"false\")."
    },
    { CERTS_PATH, 0, "", "certs",           Arg::Required,      "  --certs <arg>  \tPath where located certificates." },
#endif // if HAVE_SECURITY
    {
        XML_FILE, 0, "", "xml",               Arg::String,    "\t--xml \tXML Configuration file."
    },
    { DATA_SIZE, 0, "", "size",             Arg::Numeric,   "\t--size\tData size." },
    { DYNAMIC_TYPES, 0, "", "dynamic_types", Arg::None,      "\t--dynamic_types \tUse dynamic types." },
    { 0, 0, 0, 0, 0, 0 }
};

const int c_n_samples = 10000;

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

    bool pub_sub = false;
    int sub_number = 1;
    int n_samples = c_n_samples;
#if HAVE_SECURITY
    bool use_security = false;
    std::string certs_path;
#endif // if HAVE_SECURITY
    bool echo = false;
    bool reliable = false;
    uint32_t seed = 80;
    bool hostname = false;
    bool export_csv = false;
    bool dynamic_types = false;
    uint32_t data_size = 16;
    uint32_t test_time_sec = 5;
    std::string export_prefix = "";
    std::string sXMLConfigFile = "";

    argc -= (argc > 0);
    argv += (argc > 0); // skip program name argv[0] if present
    if (argc > 0)
    {
        if (strcmp(argv[0], "publisher") == 0)
        {
            pub_sub = true;
        }
        else if (strcmp(argv[0], "subscriber") == 0)
        {
            pub_sub = false;
        }
        else
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return 0;
        }
    }
    else
    {
        option::printUsage(fwrite, stdout, usage, columns);
        return 0;
    }

    argc -= (argc > 0); argv += (argc > 0); // skip pub/sub argument
    option::Stats stats(usage, argc, argv);
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);

    if (parse.error())
    {
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
            case HELP:
                // not possible, because handled further above and exits the program
                break;
            case RELIABILITY:
                if (strcmp(opt.arg, "reliable") == 0)
                {
                    reliable = true;
                }
                else if (strcmp(opt.arg, "besteffort") == 0)
                {
                    reliable = false;
                }
                else
                {
                    option::printUsage(fwrite, stdout, usage, columns);
                    return 0;
                }
                break;

            case SEED:
                seed = strtol(opt.arg, nullptr, 10);
                break;
            case SAMPLES:
                n_samples = strtol(opt.arg, nullptr, 10);
                break;

            case SUBSCRIBERS:
                sub_number = strtol(opt.arg, nullptr, 10);
                break;

            case ECHO_OPT:
                if (strcmp(opt.arg, "true") == 0)
                {
                    echo = true;
                }
                else if (strcmp(opt.arg, "false") == 0)
                {
                    echo = false;
                }
                else
                {
                    option::printUsage(fwrite, stdout, usage, columns);
                    return 0;
                }
                break;

            case HOSTNAME:
                hostname = true;
                break;

            case EXPORT_CSV:
                export_csv = true;
                break;

            case EXPORT_PREFIX:
            {
                if (opt.arg != nullptr)
                {
                    export_prefix = opt.arg;
                }
                else
                {
                    option::printUsage(fwrite, stdout, usage, columns);
                    return 0;
                }
                break;
            }

            case DATA_SIZE:
                data_size = strtol(opt.arg, nullptr, 10);
                break;

            case XML_FILE:
                if (opt.arg != nullptr)
                {
                    sXMLConfigFile = opt.arg;
                }
                else
                {
                    option::printUsage(fwrite, stdout, usage, columns);
                    return 0;
                }
                break;

            case DYNAMIC_TYPES:
                dynamic_types = true;
                break;

            case TIME:
                test_time_sec = strtol(opt.arg, nullptr, 10);
                break;

#if HAVE_SECURITY
            case USE_SECURITY:
                if (strcmp(opt.arg, "true") == 0)
                {
                    use_security = true;
                }
                else if (strcmp(opt.arg, "false") == 0)
                {
                    use_security = false;
                }
                else
                {
                    option::printUsage(fwrite, stdout, usage, columns);
                    return -1;
                }
                break;

            case CERTS_PATH:
                certs_path = opt.arg;
                break;
#endif // if HAVE_SECURITY

            case UNKNOWN_OPT:
                option::printUsage(fwrite, stdout, usage, columns);
                return 0;
                break;
        }
    }

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

#if HAVE_SECURITY
    if (use_security)
    {
        if (certs_path.empty())
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return -1;
        }

        sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                "builtin.PKI-DH"));
        sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                "file://" + certs_path + "/maincacert.pem"));
        sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                "file://" + certs_path + "/mainsubcert.pem"));
        sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                "file://" + certs_path + "/mainsubkey.pem"));
        sub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                "builtin.AES-GCM-GMAC"));
        sub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
        sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
        sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

        pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                "builtin.PKI-DH"));
        pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                "file://" + certs_path + "/maincacert.pem"));
        pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                "file://" + certs_path + "/mainpubcert.pem"));
        pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                "file://" + certs_path + "/mainpubkey.pem"));
        pub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                "builtin.AES-GCM-GMAC"));
        pub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
        pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
        pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");
    }
#endif // if HAVE_SECURITY

    // Load an XML file with predefined profiles for publisher and subscriber
    if (sXMLConfigFile.length() > 0)
    {
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_XML_profiles_file(sXMLConfigFile);
    }

    if (pub_sub)
    {
        std::cout << "Performing test with " << sub_number << " subscribers and " << n_samples << " samples" <<
            std::endl;
        MemoryTestPublisher memoryPub;
        memoryPub.init(sub_number, n_samples, reliable, seed, hostname, export_csv, export_prefix,
                pub_part_property_policy, pub_property_policy, sXMLConfigFile, data_size, dynamic_types);
        memoryPub.run(test_time_sec);
    }
    else
    {
        MemoryTestSubscriber memorySub;
        memorySub.init(echo, n_samples, reliable, seed, hostname, sub_part_property_policy, sub_property_policy,
                sXMLConfigFile, data_size, dynamic_types);
        memorySub.run();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::cout << "EVERYTHING STOPPED FINE" << std::endl;

    return 0;
}

#if defined(_MSC_VER)
#pragma warning (pop)
#endif // if defined(_MSC_VER)
