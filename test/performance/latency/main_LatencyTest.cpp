﻿// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "LatencyTestPublisher.hpp"
#include "LatencyTestSubscriber.hpp"
#include "../optionparser.h"

#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstdint>
#include <fstream>

#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/Domain.h>
#include <fastrtps/fastrtps_dll.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable:4512)
#endif

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

#if defined(_WIN32)
#define COPYSTR strcpy_s
#else
#define COPYSTR strcpy
#endif


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
    EXPORT_RAW_DATA,
    EXPORT_PREFIX,
    USE_SECURITY,
    CERTS_PATH,
    XML_FILE,
    DYNAMIC_TYPES,
    FORCED_DOMAIN,
    FILE_R
};

enum TestAgent
{
    PUBLISHER,
    SUBSCRIBER,
    BOTH
};

const option::Descriptor usage[] = {
    { UNKNOWN_OPT,     0, "",  "",                Arg::None,     "Usage: LatencyTest <publisher|subscriber|both>\n\nGeneral options:" },
    { HELP,            0, "h", "help",            Arg::None,     "  -h           --help                Produce help message." },
    { RELIABILITY,     0, "r", "reliability",     Arg::Required, "  -r <arg>,    --reliability=<arg>   Set reliability (\"reliable\"/\"besteffort\")."},
    { SAMPLES,         0, "s", "samples",         Arg::Numeric,  "  -s <num>,    --samples=<num>       Number of samples." },
    { SEED,            0, "",  "seed",            Arg::Numeric,  "               --seed=<num>          Seed to calculate domain and topic." },
    { HOSTNAME,        0, "",  "hostname",        Arg::None,     "               --hostname            Append hostname to the topic." },
    { XML_FILE,        0, "",  "xml",             Arg::String,   "               --xml                 XML Configuration file." },
    { FORCED_DOMAIN,   0, "",  "domain",          Arg::Numeric,  "               --domain              RTPS Domain." },
    { DYNAMIC_TYPES,   0, "",  "dynamic_types",   Arg::None,     "               --dynamic_types       Use dynamic types." },
#if HAVE_SECURITY
    { USE_SECURITY,    0, "",  "security",        Arg::Required, "               --security <arg>      Echo mode (\"true\"/\"false\")." },
    { CERTS_PATH,      0, "",  "certs",           Arg::Required, "               --certs <arg>         Path where located certificates." },
#endif
    { UNKNOWN_OPT,     0, "",  "",                Arg::None,     "\nPublisher/Both options:"},
    { SUBSCRIBERS,     0, "n", "subscribers",     Arg::Numeric,  "  -n <num>,    --subscribers=<arg>   Number of subscribers." },
    { EXPORT_CSV,      0, "",  "export_csv",      Arg::None,     "               --export_cvs          Flag to export a CSV file." },
    { EXPORT_RAW_DATA, 0, "",  "export_raw_data", Arg::String,   "               --export_raw_data     File name to export all raw data as CSV." },
    { EXPORT_PREFIX,   0, "",  "export_prefix",   Arg::String,   "               --export_prefix       File prefix for the CSV file." },
    { UNKNOWN_OPT,     0, "",  "",                Arg::None,     "\nSubscriber options:"},
    { ECHO_OPT,        0, "e", "echo",            Arg::Required, "  -e <arg>,    --echo=<arg>          Echo mode (\"true\"/\"false\")." },
    { FILE_R,        0, "f", "file",            Arg::Required, "  -f <arg>,  --file=<arg>             File to read the payload demands from." },
    { 0, 0, 0, 0, 0, 0 }
};

bool load_demands_payload(const std::string& demands_file, std::vector<uint32_t>& demands)
{
    demands.clear();

    std::ifstream fi(demands_file);

    std::cout << "Reading demands file: " << demands_file << std::endl;
    std::string DELIM = ";";
    if (!fi.is_open())
    {
        std::cout << "Could not open demands file: " << demands_file << " , closing." << std::endl;
        return false;
    }

    std::string line;
    size_t start;
    size_t end;
    bool more = true;
    while (std::getline(fi, line))
    {
        start = 0;
        end = line.find(DELIM);
        more = true;
        while (more)
        {
            std::istringstream iss(line.substr(start, end - start));

            uint32_t payload;
            iss >> payload;
            if (payload < 12)
            {
                std::cout << "Minimum payload is 16 bytes" << std::endl;
                return false;
            }

            demands.push_back(payload-4);

            start = end + DELIM.length();
            end = line.find(DELIM, start);
            if (end == std::string::npos)
            {
                more = false;
                std::istringstream n_iss(line.substr(start, end - start));
                if (n_iss >> payload)
                {
                    demands.push_back(payload-4);
                }
            }
        }
    }
    fi.close();

    return true;
}

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
#endif

    TestAgent test_agent = TestAgent::PUBLISHER;
    int subscribers = 1;
    int samples = 10000;
#if HAVE_SECURITY
    bool use_security = false;
    std::string certs_path;
#endif
    bool echo = true;
    bool reliable = false;
    uint32_t seed = 80;
    bool hostname = false;
    bool export_csv = false;
    std::string export_prefix = "";
    std::string raw_data_file = "";
    std::string xml_config_file = "";
    bool dynamic_types = false;
    int forced_domain = -1;
    std::string demands_file = "";

    argc -= (argc > 0);
    argv += (argc > 0); // skip program name argv[0] if present
    if (argc > 0)
    {
        if (strcmp(argv[0], "publisher") == 0)
        {
            test_agent = TestAgent::PUBLISHER;
        }
        else if (strcmp(argv[0], "subscriber") == 0)
        {
            test_agent = TestAgent::SUBSCRIBER;
        }
        else if (strcmp(argv[0], "both") == 0)
        {
            test_agent = TestAgent::BOTH;
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
                samples = strtol(opt.arg, nullptr, 10);
                break;
            case SUBSCRIBERS:
                subscribers = strtol(opt.arg, nullptr, 10);
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
            case EXPORT_RAW_DATA:
                raw_data_file = opt.arg;
                break;
            case EXPORT_PREFIX:
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
            case XML_FILE:
                if (opt.arg != nullptr)
                {
                    xml_config_file = opt.arg;
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
            case FORCED_DOMAIN:
                forced_domain = strtol(opt.arg, nullptr, 10);
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
#endif
            case FILE_R:
                demands_file = opt.arg;
                break;
            case UNKNOWN_OPT:
                option::printUsage(fwrite, stdout, usage, columns);
                return 0;
                break;
        }
    }

    PropertyPolicy pub_part_property_policy;
    PropertyPolicy sub_part_property_policy;
    PropertyPolicy pub_property_policy;
    PropertyPolicy sub_property_policy;

#if HAVE_SECURITY
    if (use_security)
    {
        if (certs_path.empty())
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return -1;
        }

        sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin", "builtin.PKI-DH"));
        sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                "file://" + certs_path + "/maincacert.pem"));
        sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                "file://" + certs_path + "/mainsubcert.pem"));
        sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                "file://" + certs_path + "/mainsubkey.pem"));
        sub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC"));
        sub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

        sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
        sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

        pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin", "builtin.PKI-DH"));
        pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                "file://" + certs_path + "/maincacert.pem"));
        pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                "file://" + certs_path + "/mainpubcert.pem"));
        pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                "file://" + certs_path + "/mainpubkey.pem"));
        pub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC"));
        pub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
        pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
        pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");
    }
#endif

    // Load an XML file with predefined profiles for publisher and subscriber
    if (xml_config_file.length() > 0)
    {
        xmlparser::XMLProfileManager::loadXMLFile(xml_config_file);
    }

    LatencyDataSizes data_sizes;

    if(!demands_file.empty())
    {
        load_demands_payload(demands_file, data_sizes.sample_sizes());
    }
    
    uint8_t return_code = 0;

    if (test_agent == TestAgent::PUBLISHER)
    {
        std::cout << "Performing test with " << subscribers << " subscribers and " << samples << " samples"
                << std::endl;
        LatencyTestPublisher latency_publisher;
        if (latency_publisher.init(subscribers, samples, reliable, seed, hostname, export_csv, export_prefix,
                raw_data_file, pub_part_property_policy, pub_property_policy, xml_config_file,
                dynamic_types, forced_domain, data_sizes))
        {
            latency_publisher.run();
        }
        else
        {
            return_code = 1;
        }

    }
    else if (test_agent == TestAgent::SUBSCRIBER)
    {
        LatencyTestSubscriber latency_subscriber;
        if (latency_subscriber.init(echo, samples, reliable, seed, hostname, sub_part_property_policy, sub_property_policy,
                xml_config_file, dynamic_types, forced_domain, data_sizes))
        {
            latency_subscriber.run();
        }
        else
        {
            return_code = 1;
        }

    }
    else if (test_agent == TestAgent::BOTH)
    {
        std::cout << "Performing intraprocess test with " << subscribers << " subscribers and " << samples <<
                " samples" << std::endl;

        // Initialize publisher
        LatencyTestPublisher latency_publisher;
        bool pub_init = latency_publisher.init(subscribers, samples, reliable, seed, hostname, export_csv,
                export_prefix, raw_data_file, pub_part_property_policy, pub_property_policy,
                xml_config_file, dynamic_types, forced_domain, data_sizes);

        // Initialize subscriber
        LatencyTestSubscriber latency_subscriber;
        bool sub_init = latency_subscriber.init(echo, samples, reliable, seed, hostname, sub_part_property_policy,
                sub_property_policy, xml_config_file, dynamic_types, forced_domain, data_sizes);

        // Spawn run threads
        if (pub_init && sub_init)
        {
            std::thread pub_thread(&LatencyTestPublisher::run, &latency_publisher);
            std::thread sub_thread(&LatencyTestSubscriber::run, &latency_subscriber);
            pub_thread.join();
            sub_thread.join();
        }
        else
        {
            return_code = 1;
        }

    }

    if (return_code == 0)
    {
        std::cout << "EVERYTHING STOPPED FINE" << std::endl;
    }
    else
    {
        std::cout << "SOMETHING WENT WRONG" << std::endl;
    }

    return return_code;
}

#if defined(_MSC_VER)
#pragma warning (pop)
#endif
