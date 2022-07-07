// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_TYPEINTROSPECTIONEXAMPLE_ARG_CONFIGURATION_H_
#define _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_TYPEINTROSPECTIONEXAMPLE_ARG_CONFIGURATION_H_

#include <iostream>
#include <string>

#include <optionparser.hpp>

namespace option = eprosima::option;

constexpr const char* HELLO_WORLD_DATA_TYPE_ARG = "hw";
constexpr const char* ARRAY_DATA_TYPE_ARG = "array";
constexpr const char* STRUCT_DATA_TYPE_ARG = "struct";
constexpr const char* PLAIN_DATA_TYPE_ARG = "plain";
constexpr const char* KEY_DATA_TYPE_ARG = "key";
constexpr const char* COMPLEX_DATA_TYPE_ARG = "complex";

constexpr const char* GENERATOR_DATA_TYPE_GEN_ARG = "gen";
constexpr const char* GENERATOR_DATA_TYPE_CODE_ARG = "code";
constexpr const char* GENERATOR_DATA_TYPE_XML_ARG = "xml";

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

    static option::ArgStatus DataType(
            const option::Option& option,
            bool msg)
    {
        if (option.arg != 0)
        {
            std::string data_type = std::string(option.arg);
            if (data_type != HELLO_WORLD_DATA_TYPE_ARG &&
                data_type != ARRAY_DATA_TYPE_ARG &&
                data_type != STRUCT_DATA_TYPE_ARG &&
                data_type != PLAIN_DATA_TYPE_ARG &&
                data_type != KEY_DATA_TYPE_ARG &&
                data_type != COMPLEX_DATA_TYPE_ARG)
            {
                if (msg)
                {
                    print_error("Option '", option, "' only accepts <hw|array|struct|plain|key|complex> values\n");
                }
                return option::ARG_ILLEGAL;
            }
            return option::ARG_OK;
        }
        if (msg)
        {
            print_error("Option '", option, "' requires a string argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Generator(
            const option::Option& option,
            bool msg)
    {
        if (option.arg != 0)
        {
            std::string data_type = std::string(option.arg);
            if (data_type != GENERATOR_DATA_TYPE_GEN_ARG &&
                data_type != GENERATOR_DATA_TYPE_CODE_ARG &&
                data_type != GENERATOR_DATA_TYPE_XML_ARG)
            {
                if (msg)
                {
                    print_error("Option '", option, "' only accepts <gen|xml|code> values\n");
                }
                return option::ARG_ILLEGAL;
            }
            return option::ARG_OK;
        }
        if (msg)
        {
            print_error("Option '", option, "' requires a string argument\n");
        }
        return option::ARG_ILLEGAL;
    }
};

enum optionIndex
{
    UNKNOWN_OPT,
    HELP,
    TOPIC_NAME,
    DOMAIN_ID,
    DATA_TYPE,
    DATA_TYPE_GENERATOR,
    SAMPLES,
    TYPE_OBJECT,
    TYPE_INFORMATION,
};

const option::Descriptor usage[] = {
    { UNKNOWN_OPT, 0, "", "",                Arg::None,
      "Usage: TypeIntrospectionExample <publisher|subscriber>\n\nGeneral options:" },
    { HELP,    0, "h", "help",               Arg::None,      "  -h \t--help  \tProduce help message." },

    { UNKNOWN_OPT, 0, "", "",                Arg::None,      "\nPublisher options:"},

    { TOPIC_NAME, 0, "t", "topic",                  Arg::String,
      "  -t <topic_name> \t--topic=<topic_name>  \tTopic name (Default: TypeIntrospectionTopic)." },
    { DATA_TYPE, 0, "x", "type",                  Arg::DataType,
      "  -x <data_type_name> \t--type=<data_type_name>  \tTopic Data Type name (Default: hw). "
      "hw      -> HelloWorld data type (one string and one integer). "
      "array   -> Data type with an array (one uint and an array of size 3 integer). "
      "struct  -> Complex data type with an internal struct (3 integers). "
      "plain   -> HelloWorld struct (index and string) but string is an array of 20 chars, so data type is Plain. "
      "key     -> Struct with an index, a keyed string value and 1 integer. "
      "complex -> Complex data type with 1 index, 1 substructure with 3 integers and an array of size 3 substructures, "
      "with a string and a char each. "
    },
    { DATA_TYPE_GENERATOR, 0, "g", "generator",                  Arg::Generator,
      "  -g <generator_name> \t--generator=<generator_name>  \tData Type Generator (Default: gen). "
      "gen   -> Use IDL file and Fast DDS Gen to generate Data Type introspection information. "
      "xml   -> Use XML file to generate Data Type introspection information. "
      "code  -> Use DynamicTypes API to generate Data Type introspection information. "
    },
    { DOMAIN_ID, 0, "d", "domain",                Arg::Numeric,
      "  -d <id> \t--domain=<id>  \tDDS domain ID (Default: 0)." },
    { SAMPLES, 0, "s", "samples",              Arg::Numeric,
      "  -s <num> \t--samples=<num>  \tNumber of samples to send (Default: 0 => infinite samples)." },
    { TYPE_OBJECT, 0, "o", "type-object",              Arg::None,
      "  -o \t--type-object=  \tUse Type Object to send Data Type Introspection info." },
    { TYPE_INFORMATION, 0, "i", "type-information",              Arg::None,
      "  -i \t--type-information=  \tUse Type Information to send Data Type Introspection info." },

    { UNKNOWN_OPT, 0, "", "",                Arg::None,      "\nSubscriber options:"},

    { TOPIC_NAME, 0, "t", "topic",                  Arg::String,
      "  -t <topic_name> \t--topic=<topic_name>  \tTopic name (Default: TypeIntrospectionTopic)." },
    { DOMAIN_ID, 0, "d", "domain",                Arg::Numeric,
      "  -d <id> \t--domain=<id>  \tDDS domain ID (Default: 0)." },
    { SAMPLES, 0, "s", "samples",              Arg::Numeric,
      "  -s <num> \t--samples=<num>  \tNumber of samples to wait for (Default: 0 => infinite samples)." },
    { TYPE_OBJECT, 0, "o", "type-object",              Arg::None,
      "  -o \t--type-object=  \tUse Type Object to send Data Type Introspection info." },
    { TYPE_INFORMATION, 0, "i", "type-information",              Arg::None,
      "  -i \t--type-information=  \tUse Type Information to send Data Type Introspection info." },

    { 0, 0, 0, 0, 0, 0 }
};

void print_warning(
        std::string type,
        const char* opt)
{
    std::cerr << "WARNING: " << opt << " is a " << type << " option, ignoring argument." << std::endl;
}

#endif /* _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_TYPEINTROSPECTIONEXAMPLE_ARG_CONFIGURATION_H_ */
