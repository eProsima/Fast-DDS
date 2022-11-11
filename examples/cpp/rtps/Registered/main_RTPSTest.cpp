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

#include <stdio.h>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstdint>
#include <sstream>

#include "fastdds/rtps/attributes/RTPSParticipantAttributes.h"
#include "fastrtps/log/Log.h"
#include "fastrtps/rtps/RTPSDomain.h"
#include "arg_configuration.h"
#include "TestWriterRegistered.h"
#include "TestReaderRegistered.h"

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;
using namespace std;

enum EntityType
{
    WRITER = 1,
    READER
};

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

    int type;
    uint16_t samples = 100;
    uint16_t interval = 100;
    bool enable_ds_p2p_ppty = false;

    argc -= (argc > 0);
    argv += (argc > 0); // skip program name argv[0] if present

    option::Stats stats(true, usage, argc, argv);
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(true, usage, argc, argv, &options[0], &buffer[0]);

    if (parse.error())
    {
        option::printUsage(fwrite, stdout, usage, columns);
        return 1;
    }

    if (options[optionIndex::HELP])
    {
        option::printUsage(fwrite, stdout, usage, columns);
        return 0;
    }

    cout << "Starting RTPS example" << endl;

    try
    {
        if (parse.nonOptionsCount() != 1)
        {
            throw 1;
        }

        const char* type_name = parse.nonOption(0);

        // make sure is the first option.
        // type_name and buffer[0].name reference the original command line char array
        // type_name must precede any other arguments in the array.
        // Note buffer[0].arg may be null for non-valued options and is not reliable for
        // testing purposes.
        if (parse.optionsCount() && type_name >= buffer[0].name)
        {
            throw 1;
        }

        if (strcmp(type_name, "writer") == 0)
        {
            type = EntityType::WRITER;
        }
        else if (strcmp(type_name, "reader") == 0)
        {
            type = EntityType::READER;
        }
        else
        {
            throw 1;
        }
    }
    catch (int error)
    {
        std::cerr << "ERROR: first argument must be <writer|reader> followed by - or -- options" << std::endl;
        option::printUsage(fwrite, stdout, usage, columns);
        return error;
    }

    for (int i = 0; i < parse.optionsCount(); ++i)
    {
        option::Option& opt = buffer[i];
        switch (opt.index())
        {
            case optionIndex::HELP:
                // not possible, because handled further above and exits the program
                break;

            case optionIndex::DSP2P:
                enable_ds_p2p_ppty = true;
                break;

            case optionIndex::SAMPLES:
                samples = strtol(opt.arg, nullptr, 10);
                break;

            case optionIndex::INTERVAL:
                interval = strtol(opt.arg, nullptr, 10);
                break;
        }
    }

    switch (type)
    {
        case 1:
        {
            TestWriterRegistered TW;
            if (TW.init(enable_ds_p2p_ppty) && TW.reg())
            {
                TW.run(samples,interval);
            }
            break;
        }
        case 2:
        {
            TestReaderRegistered TR;
            if (TR.init(enable_ds_p2p_ppty) && TR.reg())
            {
                TR.run();
            }
            break;
        }
    }

    RTPSDomain::stopAll();
    cout << "EVERYTHING STOPPED FINE" << endl;

    return 0;
}
