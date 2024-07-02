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
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Colors.hpp>
#include <fastdds/dds/log/Log.hpp>

#include "../optionarg.hpp"
#include "LatencyTestPublisher.hpp"
#include "LatencyTestSubscriber.hpp"

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable:4512)
#endif // if defined(_MSC_VER)

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

#if defined(_WIN32)
#define COPYSTR strcpy_s
#else
#define COPYSTR strcpy
#endif // if defined(_WIN32)

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
    FILE_R,
    DATA_SHARING,
    DATA_LOAN,
    SHARED_MEMORY
};

enum TestAgent
{
    PUBLISHER,
    SUBSCRIBER,
    BOTH
};

const option::Descriptor usage[] = {
    { UNKNOWN_OPT,     0, "",  "",                Arg::None,
      "Usage: LatencyTest <publisher|subscriber|both>\n\nGeneral options:" },
    { HELP,            0, "h", "help",            Arg::None,
      "  -h           --help                Produce help message." },
    { RELIABILITY,     0, "r", "reliability",     Arg::Required,
      "  -r <arg>,    --reliability=<arg>   Set reliability (\"reliable\"/\"besteffort\")."},
    { SAMPLES,         0, "s", "samples",         Arg::Numeric,
      "  -s <num>,    --samples=<num>       Number of samples." },
    { SEED,            0, "",  "seed",            Arg::Numeric,
      "               --seed=<num>          Seed to calculate domain and topic." },
    { HOSTNAME,        0, "",  "hostname",        Arg::None,
      "               --hostname            Append hostname to the topic." },
    { XML_FILE,        0, "",  "xml",             Arg::String,
      "               --xml                 XML Configuration file." },
    { FORCED_DOMAIN,   0, "",  "domain",          Arg::Numeric,  "               --domain              RTPS Domain." },
    { FILE_R,        0, "f", "file",            Arg::Required,
      "  -f <arg>,  --file=<arg>             File to read the payload demands from." },
    { DYNAMIC_TYPES,   0, "",  "dynamic_types",   Arg::None,
      "               --dynamic_types       Use dynamic types." },
    { DATA_SHARING,  0, "d", "data_sharing",    Arg::Enabler,
      "               --data_sharing=[on|off]             Explicitly enable/disable data sharing feature." },
    { DATA_LOAN,        0, "l", "data_loans",            Arg::None,
      "               --data_loans          Use loan sample API." },
    { SHARED_MEMORY,    0, "", "shared_memory", Arg::Enabler,
      "               --shared_memory=[on|off]             Explicitly enable/disable shared memory transport." },
#if HAVE_SECURITY
    {
        USE_SECURITY,    0, "",  "security",        Arg::Required,
        "               --security <arg>      Enable/disable DDS security (\"true\"/\"false\")."
    },
    { CERTS_PATH,      0, "",  "certs",           Arg::Required,
      "               --certs <arg>         Path where located certificates." },
#endif // if HAVE_SECURITY
    {
        UNKNOWN_OPT,     0, "",  "",                Arg::None,     "\nPublisher/Both options:"
    },
    { SUBSCRIBERS,     0, "n", "subscribers",     Arg::Numeric,
      "  -n <num>,    --subscribers=<arg>   Number of subscribers." },
    { EXPORT_CSV,      0, "",  "export_csv",      Arg::None,
      "               --export_csv          Flag to export a CSV file." },
    { EXPORT_RAW_DATA, 0, "",  "export_raw_data", Arg::String,
      "               --export_raw_data     File name to export all raw data as CSV." },
    { EXPORT_PREFIX,   0, "",  "export_prefix",   Arg::String,
      "               --export_prefix       File prefix for the CSV file." },
    { UNKNOWN_OPT,     0, "",  "",                Arg::None,     "\nSubscriber options:"},
    { ECHO_OPT,        0, "e", "echo",            Arg::Required,
      "  -e <arg>,    --echo=<arg>          Echo mode (\"true\"/\"false\")." },
    { 0, 0, 0, 0, 0, 0 }
};

bool load_demands_payload(
        const std::string& demands_file,
        std::vector<uint32_t>& demands)
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
            if (payload < 16)
            {
                std::cout << "Payload must be a positive number greater or equal to 16" << std::endl;
                return false;
            }

            demands.push_back(payload);

            start = end + DELIM.length();
            end = line.find(DELIM, start);
            if (end == std::string::npos)
            {
                more = false;
                std::istringstream n_iss(line.substr(start, end - start));
                if (n_iss >> payload)
                {
                    demands.push_back(payload);
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

    using Log = eprosima::fastdds::dds::Log;
    Log::SetVerbosity(Log::Kind::Info);
    Log::SetCategoryFilter(std::regex("LatencyTest"));

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

    TestAgent test_agent = TestAgent::PUBLISHER;
    int subscribers = 1;
    int samples = 10000;
#if HAVE_SECURITY
    bool use_security = false;
    std::string certs_path;
#endif // if HAVE_SECURITY
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
    Arg::EnablerValue data_sharing = Arg::EnablerValue::NO_SET;
    bool data_loans = false;
    Arg::EnablerValue shared_memory = Arg::EnablerValue::NO_SET;

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
#endif // if HAVE_SECURITY
            case FILE_R:
                demands_file = opt.arg;
                break;
            case DATA_SHARING:
                if (0 == strncasecmp(opt.arg, "on", 2))
                {
                    data_sharing = Arg::EnablerValue::ON;
                }
                else
                {
                    data_sharing = Arg::EnablerValue::OFF;
                }
                break;
                break;
            case DATA_LOAN:
                data_loans = true;
                break;
            case SHARED_MEMORY:
                if (0 == strncasecmp(opt.arg, "on", 2))
                {
                    shared_memory = Arg::EnablerValue::ON;
                }
                else
                {
                    shared_memory = Arg::EnablerValue::OFF;
                }
                break;
            case UNKNOWN_OPT:
            default:
                option::printUsage(fwrite, stdout, usage, columns);
                return 0;
                break;
        }
    }

#if HAVE_SECURITY
    // Check parameters validity
    if (use_security)
    {
        if (test_agent == TestAgent::BOTH)
        {
            EPROSIMA_LOG_ERROR(LatencyTest, "Intra-process delivery NOT supported with security");
            return 1;
        }
        else if (Arg::EnablerValue::ON == data_sharing)
        {
            EPROSIMA_LOG_ERROR(LatencyTest, "Sharing sample APIs NOT supported with RTPS encryption");
            return 1;
        }
    }
#endif // if HAVE_SECURITY

    if ((Arg::EnablerValue::ON == data_sharing || data_loans) && dynamic_types)
    {
        EPROSIMA_LOG_ERROR(LatencyTest, "Sharing sample APIs NOT supported with dynamic types");
        return 1;
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
#endif // if HAVE_SECURITY

    // Load an XML file with predefined profiles for publisher and subscriber
    if (xml_config_file.length() > 0)
    {
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_XML_profiles_file(xml_config_file);
    }

    LatencyDataSizes data_sizes;

    if (!demands_file.empty())
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
                dynamic_types, data_sharing, data_loans, shared_memory, forced_domain, data_sizes))
        {
            latency_publisher.run();
            latency_publisher.destroy_user_entities();
        }
        else
        {
            return_code = 1;
        }

    }
    else if (test_agent == TestAgent::SUBSCRIBER)
    {
        LatencyTestSubscriber latency_subscriber;
        if (latency_subscriber.init(echo, samples, reliable, seed, hostname, sub_part_property_policy,
                sub_property_policy,
                xml_config_file, dynamic_types, data_sharing, data_loans, shared_memory, forced_domain, data_sizes))
        {
            latency_subscriber.run();
            latency_subscriber.destroy_user_entities();
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
                        xml_config_file, dynamic_types, data_sharing, data_loans, shared_memory, forced_domain,
                        data_sizes);

        // Initialize subscribers
        std::vector<std::shared_ptr<LatencyTestSubscriber>> latency_subscribers;

        bool sub_init = true;
        for (int i = 0; i < subscribers; i++)
        {
            latency_subscribers.push_back(std::make_shared<LatencyTestSubscriber>());
            sub_init &= latency_subscribers.back()->init(echo, samples, reliable, seed, hostname,
                            sub_part_property_policy,
                            sub_property_policy, xml_config_file, dynamic_types, data_sharing, data_loans,
                            shared_memory,
                            forced_domain, data_sizes);
        }

        // Spawn run threads
        if (pub_init && sub_init)
        {
            std::thread pub_thread(&LatencyTestPublisher::run, &latency_publisher);

            std::vector<std::thread> sub_threads;
            for (auto& sub : latency_subscribers)
            {
                sub_threads.emplace_back(&LatencyTestSubscriber::run, sub.get());
            }

            pub_thread.join();

            for (auto& sub : sub_threads)
            {
                sub.join();
            }

            for (auto& sub : latency_subscribers)
            {
                sub->destroy_user_entities();
            }

            latency_publisher.destroy_user_entities();
        }
        else
        {
            return_code = 1;
        }
    }

    if (return_code == 0)
    {
        std::cout << C_GREEN << "EVERYTHING STOPPED FINE" << C_DEF << std::endl;
    }
    else
    {
        std::cout << C_RED << "SOMETHING WENT WRONG" << C_DEF << std::endl;
    }

    return return_code;
}

#if defined(_MSC_VER)
#pragma warning (pop)
#endif // if defined(_MSC_VER)
