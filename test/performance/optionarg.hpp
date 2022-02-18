#ifndef _TEST_PERFORMANCE_OPTIONARG_HPP_
#define _TEST_PERFORMANCE_OPTIONARG_HPP_

#include <optionparser.hpp>

#include <stdio.h>
#include <cstdint>
#include <iostream>
#include <string.h>

#ifdef WIN32
#define strncasecmp _strnicmp
#endif // ifdef WIN32

namespace option = eprosima::option;

struct Arg : public option::Arg
{
    enum class EnablerValue : int32_t
    {
        NO_SET,
        ON,
        OFF
    };

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
            print_error("Option '", option, "' requires a numeric argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Enabler(
            const option::Option& option,
            bool msg)
    {
        if (nullptr == option.arg ||
                (0 == strncasecmp(option.arg, "on", 2) ||
                0 == strncasecmp(option.arg, "off", 3)))
        {
            return option::ARG_OK;
        }
        if (msg)
        {
            print_error("Option '", option, "' supports values 'on' or 'off'\n");
        }
        return option::ARG_ILLEGAL;
    }

};

#endif // _TEST_PERFORMANCE_OPTIONARG_HPP_
