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

#include "ThroughputTypes.hpp"
#include "ThroughputPublisher.hpp"
#include "ThroughputSubscriber.hpp"

#include "../optionparser.h"

#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstdint>

#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/Domain.h>
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

    static void print_error(const char* msg1, const option::Option& opt, const char* msg2)
    {
        fprintf(stderr, "%s", msg1);
        fwrite(opt.name, opt.namelen, 1, stderr);
        fprintf(stderr, "%s", msg2);
    }

    static option::ArgStatus Unknown(const option::Option& option, bool msg)
    {
        if (msg)
        {
            print_error("Unknown option '", option, "'\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Required(const option::Option& option, bool msg)
    {
        if (option.arg != 0 && option.arg[0] != 0)
        {
            return option::ARG_OK;
        }

        if (msg)
        {
            print_error("Option '", option, "' requires an argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Numeric(const option::Option& option, bool msg)
    {
        char* endptr = 0;
        if (option.arg != 0 && strtol(option.arg, &endptr, 10))
        {
        };
        if (endptr != option.arg && *endptr == 0)
        {
            return option::ARG_OK;
        }

        if (msg)
        {
            print_error("Option '", option, "' requires a numeric argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus String(const option::Option& option, bool msg)
    {
        if (option.arg != 0)
        {
            return option::ARG_OK;
        }
        if (msg)
        {
            print_error("Option '", option, "' requires a numeric argument\n");
        }
        return option::ARG_ILLEGAL;
    }
};

enum  optionIndex {
    UNKNOWN_OPT,
    HELP,
    RELIABILITY,
    SEED,
    TIME,
    RECOVERY_TIME,
    RECOVERIES,
    DEMAND,
    MSG_SIZE,
    FILE_R,
    HOSTNAME,
    EXPORT_CSV,
    USE_SECURITY,
    CERTS_PATH,
    XML_FILE,
    DYNAMIC_TYPES,
    FORCED_DOMAIN,
    SUBSCRIBERS
};

enum TestAgent
{
    PUBLISHER,
    SUBSCRIBER,
    BOTH
};

const option::Descriptor usage[] = {
    { UNKNOWN_OPT,   0, "",  "",                Arg::None,     "Usage: ThroughputTest <publisher|subscriber|both>\n\nGeneral options:" },
    { HELP,          0, "h", "help",            Arg::None,     "  -h         --help                   Produce help message." },
    { RELIABILITY,   0, "r", "reliability",     Arg::Required, "  -r <arg>,  --reliability=<arg>      Set reliability (\"reliable\"/\"besteffort\")."},
    { SEED,          0, "",  "seed",            Arg::Numeric,  "             --seed=<num>             Seed to calculate domain and topic, to isolate test." },
    { HOSTNAME,      0, "",  "hostname",        Arg::None,     "             --hostname               Append hostname to the topic." },
    { XML_FILE,      0, "",  "xml",             Arg::String,   "             --xml                    XML Configuration file." },
    { DYNAMIC_TYPES, 0, "",  "dynamic_types",   Arg::None,     "             --dynamic_types          Use dynamic types." },
    { FORCED_DOMAIN, 0, "",  "domain",          Arg::Numeric,  "             --domain                 Set the domain to connect." },
#if HAVE_SECURITY
    { USE_SECURITY,  0, "",  "security",        Arg::Required, "             --security <arg>         Echo mode (\"true\"/\"false\")." },
    { CERTS_PATH,    0, "",  "certs",           Arg::Required, "             --certs <arg>            Path where located certificates." },
#endif
    { UNKNOWN_OPT,   0, "",  "",                Arg::None,     "\nPublisher/Both options:"},
    { SUBSCRIBERS,     0, "n", "subscribers",     Arg::Numeric,  "  -n <num>,    --subscribers=<arg>   Number of subscribers." },
    { TIME,          0, "t", "time",            Arg::Numeric,  "  -t <num>,  --time=<num>             Time of the test in seconds." },
    { RECOVERY_TIME, 0, "",  "recovery_time",   Arg::Numeric,  "             --recovery_time=<num>    If a demand takes shorter to send than <recovery_time>, sleep the rest" },
    { RECOVERIES,    0, "",  "recoveries_file", Arg::String,   "             --recoveries_file=<num>  A CSV file with recovery times" },
    { DEMAND,        0, "d", "demand",          Arg::Numeric,  "  -d <num>,  --demand=<num>           Number of samples sent in block (Defaults: 10000)." },
    { MSG_SIZE,      0, "s", "msg_size",        Arg::Numeric,  "  -s <num>,  --msg_size=<num>         Size of the message in bits (Defaults: 1024)." },
    { FILE_R,        0, "f", "file",            Arg::Required, "  -f <arg>,  --file=<arg>             File to read the payload demands from." },
    { EXPORT_CSV,    0, "",  "export_csv",      Arg::String,   "             --export_csv             Flag to export a CVS file." },
    { UNKNOWN_OPT,   0, "",   "",               Arg::None,     "\nNote:\nIf no demand or msg_size is provided the .csv file is used.\n"},
    { 0, 0, 0, 0, 0, 0 }
};


int main(int argc, char** argv)
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

    TestAgent test_agent = TestAgent::BOTH;
    uint32_t test_time_sec = 5;
    uint32_t recovery_time_ms = 5;
    int demand = 10000;
    int msg_size = 1024*1024;
    bool reliable = false;
    uint32_t seed = 80;
    bool hostname = false;
    std::string export_csv = "";
    std::string file_name = "";
    std::string xml_config_file = "";
    std::string recoveries_file = "";
    bool dynamic_types = false;
    int forced_domain = -1;
    uint32_t subscribers = 1;
#if HAVE_SECURITY
    bool use_security = false;
    std::string certs_path;
#endif

    argc -= (argc > 0); argv += (argc > 0); // skip program name argv[0] if present
    if (argc)
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

            case TIME:
                test_time_sec = strtol(opt.arg, nullptr, 10);
                break;

            case RECOVERY_TIME:
                recovery_time_ms = strtol(opt.arg, nullptr, 10);
                break;

            case RECOVERIES:
                recoveries_file = opt.arg;
                break;

            case DEMAND:
                demand = strtol(opt.arg, nullptr, 10);
                break;

            case MSG_SIZE:
                msg_size = strtol(opt.arg, nullptr, 10);
                break;

            case FILE_R:
                file_name = opt.arg;
                break;

            case HOSTNAME:
                hostname = true;
                break;

            case SUBSCRIBERS:
                subscribers = strtol(opt.arg, nullptr, 10);
                break;

            case EXPORT_CSV:
                if (opt.arg != nullptr)
                {
                    export_csv = opt.arg;
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
        std::cout << "certs_path: " << certs_path << std::endl;

        sub_part_property_policy.properties().emplace_back(Property(
            "dds.sec.auth.plugin",
            "builtin.PKI-DH"));
        sub_part_property_policy.properties().emplace_back(Property(
            "dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + certs_path + "/maincacert.pem"));
        sub_part_property_policy.properties().emplace_back(Property(
            "dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + certs_path + "/mainsubcert.pem"));
        sub_part_property_policy.properties().emplace_back(Property(
            "dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + certs_path + "/mainsubkey.pem"));
        sub_part_property_policy.properties().emplace_back(Property(
            "dds.sec.crypto.plugin",
            "builtin.AES-GCM-GMAC"));
        sub_part_property_policy.properties().emplace_back(
            "rtps.participant.rtps_protection_kind",
            "ENCRYPT");
        sub_property_policy.properties().emplace_back(
            "rtps.endpoint.submessage_protection_kind",
            "ENCRYPT");
        sub_property_policy.properties().emplace_back(
            "rtps.endpoint.payload_protection_kind",
            "ENCRYPT");

        pub_part_property_policy.properties().emplace_back(Property(
            "dds.sec.auth.plugin",
            "builtin.PKI-DH"));
        pub_part_property_policy.properties().emplace_back(Property(
            "dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + certs_path + "/maincacert.pem"));
        pub_part_property_policy.properties().emplace_back(Property(
            "dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + certs_path + "/mainpubcert.pem"));
        pub_part_property_policy.properties().emplace_back(Property(
            "dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + certs_path + "/mainpubkey.pem"));
        pub_part_property_policy.properties().emplace_back(Property(
            "dds.sec.crypto.plugin",
            "builtin.AES-GCM-GMAC"));
        pub_part_property_policy.properties().emplace_back(
            "rtps.participant.rtps_protection_kind",
            "ENCRYPT");
        pub_property_policy.properties().emplace_back(
            "rtps.endpoint.submessage_protection_kind",
            "ENCRYPT");
        pub_property_policy.properties().emplace_back(
            "rtps.endpoint.payload_protection_kind",
            "ENCRYPT");
    }
#endif

    // The presence of a demands file overrides specific demands and payloads.
    if (file_name != "")
    {
        demand = 0;
        msg_size = 0;
    }

    // Load an XML file with predefined profiles for publisher and subscriber
    if (xml_config_file.length() > 0)
    {
        xmlparser::XMLProfileManager::loadXMLFile(xml_config_file);
    }

    uint8_t return_code = 0;

    if (test_agent == TestAgent::PUBLISHER)
    {
        std::cout << "Starting throughput test publisher agent" << std::endl;
        ThroughputPublisher throughput_publisher(reliable, seed, hostname, export_csv, pub_part_property_policy,
                pub_property_policy, xml_config_file, file_name, recoveries_file, dynamic_types, forced_domain);

        if (throughput_publisher.ready())
        {
            throughput_publisher.run(test_time_sec, recovery_time_ms, demand, msg_size, subscribers);
        }
        else
        {
            return_code = 1;
        }

    }
    else if (test_agent == TestAgent::SUBSCRIBER)
    {
        std::cout << "Starting throughput test subscriber agent" << std::endl;
        ThroughputSubscriber throughput_subscriber(reliable, seed, hostname, sub_part_property_policy,
                sub_property_policy, xml_config_file, dynamic_types, forced_domain);

        if (throughput_subscriber.ready())
        {
            throughput_subscriber.run();
        }
        else
        {
            return_code = 1;
        }

    }
    else if (test_agent == TestAgent::BOTH)
    {
        std::cout << "Starting throughput test shared process mode" << std::endl;

        // Initialize publisher
        ThroughputPublisher throughput_publisher(reliable, seed, hostname, export_csv, pub_part_property_policy,
                pub_property_policy, xml_config_file, file_name, recoveries_file, dynamic_types, forced_domain);

        // Initialize subscribers
        std::vector<std::shared_ptr<ThroughputSubscriber>> throughput_subscribers;
        
        bool are_subscribers_ready = true;
        for (uint32_t i=0; i < subscribers; i++)
        {
            throughput_subscribers.push_back(std::make_shared<ThroughputSubscriber>(reliable, 
                seed, hostname, sub_part_property_policy, sub_property_policy, 
                xml_config_file, dynamic_types, forced_domain));

            are_subscribers_ready &= throughput_subscribers.back()->ready();
        }

        // Spawn run threads
        if (throughput_publisher.ready() && are_subscribers_ready)
        {
            std::thread pub_thread(&ThroughputPublisher::run, &throughput_publisher, test_time_sec, recovery_time_ms,
                    demand, msg_size, subscribers);

            std::vector<std::thread> sub_threads;

            for (auto& sub : throughput_subscribers)
            {
                sub_threads.emplace_back(&ThroughputSubscriber::run, sub.get());
            }

            pub_thread.join();

            for (auto& sub : sub_threads)
            {
                sub.join();
            }
        }
        else
        {
            return_code = 1;
        }
    }

    Domain::stopAll();
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
