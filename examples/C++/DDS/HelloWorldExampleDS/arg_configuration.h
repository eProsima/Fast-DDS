// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file arg_configuration.h
 *
 */

#ifndef ARG_CONFIGURATION_H_
#define ARG_CONFIGURATION_H_

#include <string>
#include <iostream>
#include <regex>

#include <optionparser.h>

struct Arg : public option::Arg
{
    static void print_error(
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
            print_error("Unknown option '", option, "'\n");
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
            print_error("Option '", option, "' requires an argument\n");
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
            print_error("Option '", option, "' requires a numeric argument\n");
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
            print_error("Option '", option, "' requires a string argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Locator(
            const option::Option& option,
            bool msg)
    {
        if (option.arg != 0)
        {
            // we must check if it is a correct ip address plus port number
            if (std::regex_match(option.arg, ipv4))
            {
                return option::ARG_OK;
            }
        }
        if (msg)
        {
            print_error("Option '", option, "' requires an IPaddress[:portnumber] argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    static const std::regex ipv4;
};

enum  optionIndex
{
    UNKNOWN_OPT,
    HELP,
    TOPIC,
    WAIT,
    SAMPLES,
    INTERVAL,
    LOCATOR
};

const option::Descriptor usage[] = {
    { UNKNOWN_OPT, 0, "", "",                Arg::None,
      "Usage: HelloWorldExampleDS <publisher|subscriber|server>\n\nGeneral options:" },
    { HELP,    0, "h", "help",               Arg::None,      "  -h \t--help  \tProduce help message." },

    { UNKNOWN_OPT, 0, "", "",                Arg::None,      "\nPublisher options:"},
    { TOPIC, 0, "t", "topic",                  Arg::String,
      "  -t <topic_name> \t--topic=<topic_name>  \tTopic name (Default: HelloWorldTopic)." },
    { WAIT, 0, "w", "wait",                 Arg::Numeric,
      "  -w <num> \t--wait=<num> \tNumber of matched subscribers required to publish"
      "(Default: 0 => does not wait)." },
    { SAMPLES, 0, "s", "samples",              Arg::Numeric,
      "  -s <num> \t--samples=<num>  \tNumber of samples to send (Default: 0 => infinite samples)." },
    { INTERVAL, 0, "i", "interval",            Arg::Numeric,
      "  -i <num> \t--interval=<num>  \tTime between samples in milliseconds (Default: 100)." },
    { LOCATOR, 0, "", "ip",                 Arg::Locator,
      "  \t--ip=<IPaddress[:port number]>  \tServer address (Default address: 127.0.0.1, default port: 60006)." },

    { UNKNOWN_OPT, 0, "", "",                Arg::None,      "\nSubscriber options:"},
    { TOPIC, 0, "t", "topic",                  Arg::String,
      "  -t <topic_name> \t--topic=<topic_name>  \tTopic name (Default: HelloWorldTopic)." },
    { SAMPLES, 0, "s", "samples",              Arg::Numeric,
      "  -s <num> \t--samples=<num>  \tNumber of samples to wait for (Default: 0 => infinite samples)." },
    { LOCATOR, 0, "", "ip",                 Arg::Locator,
      "  \t--ip=<IPaddress[:port number]>  \tServer address (Default address: 127.0.0.1, default port: 60006)." },

    { UNKNOWN_OPT, 0, "", "",                Arg::None,      "\nDiscoveryServer options:"},
    { LOCATOR, 0, "", "ip",                 Arg::Locator,
      "  \t--ip=<IPaddress[:port number]>  \tServer address (Default address: 127.0.0.1, default port: 60006)." },

    { 0, 0, 0, 0, 0, 0 }
};

/*static*/ const std::regex Arg::ipv4(R"(^((?:[0-9]{1,3}\.){3}[0-9]{1,3})?:?(?:(\d+))?$)");

void print_warning(
        std::string type,
        const char* opt)
{
    std::cerr << "WARNING: " << opt << " is a " << type << " option, ignoring argument." << std::endl;
}

#endif /* ARG_CONFIGURATION_H_ */