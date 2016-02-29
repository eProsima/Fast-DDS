/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#include "ThroughputTypes.h"
#include "ThroughputPublisher.h"
#include "ThroughputSubscriber.h"

#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstdint>

#include <fastrtps/utils/RTPSLog.h>
#include <fastrtps/Domain.h>

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable:4512)
#endif

#include <boost/program_options.hpp>

using namespace eprosima;
using namespace fastrtps;


using namespace std;


#if defined(__LITTLE_ENDIAN__)
const Endianness_t DEFAULT_ENDIAN = LITTLEEND;
#elif defined (__BIG_ENDIAN__)
const Endianness_t DEFAULT_ENDIAN = BIGEND;
#endif

#if defined(_WIN32)
#define COPYSTR strcpy_s
#else
#define COPYSTR strcpy
#endif


int main(int argc, char** argv){

	Log::setVerbosity(VERB_ERROR);

    boost::program_options::options_description p_optionals("Publisher optional options");
    p_optionals.add_options()
        ("help,h", "produce help message")
        ("reliability,r", boost::program_options::value<string>()->default_value("besteffort"), "set reliability (\"reliable\"/\"besteffort\")")
        ("time,t", boost::program_options::value<uint32_t>()->default_value(5), "time of the test in seconds")
        ("recovery_time", boost::program_options::value<uint32_t>()->default_value(5), "how long to sleep after writing a demand in milliseconds")
        ("demand,d", boost::program_options::value<int>()->default_value(0), "number of sample sent in block")
		("msg_size,s", boost::program_options::value<int>()->default_value(0), "size of the message")
		("pid,p", boost::program_options::value<uint32_t>()->default_value(80), "pid of parent executable")
		("file,f", boost::program_options::value<std::string>(), "file to read the payload demands from")
        ;

    boost::program_options::options_description s_optionals("Subscriber optional options");
    s_optionals.add_options()
        ("help,h", "produce help message")
        ("reliability,r", boost::program_options::value<string>()->default_value("besteffort"), "set reliability (\"reliable\"/\"besteffort\")")
        ("pid,p", boost::program_options::value<uint32_t>()->default_value(80), "pid of parent executable")
        ;

	int type;
	uint32_t test_time_sec = 5, recovery_time_ms = 5;
	int demand = 0;
	int msg_size = 0;
    bool reliable = false;
    uint32_t pid = 80;
	std::string file_name = "";

	if(argc > 1)
	{
		if(strcmp(argv[1],"publisher")==0)
        {
			type = 1;
        }
		else if(strcmp(argv[1],"subscriber")==0)
        {
			type = 2;
        }
		else
		{
			cout << "Usage: ThroughputTest <publisher|subscriber>"<< endl;
            cout << p_optionals << endl;
            cout << s_optionals << endl;
            cout << "Note:\n\tIf no demand or msg_size is provided the .cvs file is used"<<endl;
			return -1;
		}

        boost::program_options::variables_map vm;

        if(type == 1)
        {
            boost::program_options::store(boost::program_options::parse_command_line(argc - 1, argv + 1, p_optionals), vm);
            boost::program_options::notify(vm);

            if(vm.count("help"))
            {
                cout << "Usage: ThroughputTest publisher"<< endl;
                cout << p_optionals << endl;
                return 0;
            }

			if (vm.count("file"))
			{
				file_name = vm["file"].as<std::string>();
			}

            test_time_sec = vm["time"].as<uint32_t>();
            recovery_time_ms = vm["recovery_time"].as<uint32_t>();
            demand = vm["demand"].as<int>();
            msg_size = vm["msg_size"].as<int>();
        }
        else
        {
            boost::program_options::store(boost::program_options::parse_command_line(argc - 1, argv + 1, s_optionals), vm);
            boost::program_options::notify(vm);

            if(vm.count("help"))
            {
                cout << "Usage: ThroughputTest subscriber"<< endl;
                cout << s_optionals << endl;
                return 0;
            }
        }

        std::string reliability = vm["reliability"].as<std::string>();

        if(reliability.compare("reliable") == 0)
        {
            reliable = true;
        }
        else if(reliability.compare("besteffort") == 0)
        {
            reliable = false;
        }
        else
        {
            cout << "Bad argument for option --reliability. Valid values: \"reliable\"/\"besteffort\".\n" << endl;
            cout << "Usage: ThroughputTest <publisher|subscriber>"<< endl;
            cout << p_optionals << endl;
            cout << s_optionals << endl;
            return -1;
        }

        pid = vm["pid"].as<uint32_t>();
	}
	else
	{
        cout << "Usage: ThroughputTest <publisher|subscriber>"<< endl;
        cout << p_optionals << endl;
        cout << s_optionals << endl;
		cout << "Note:\n\tIf no demand or msg_size is provided the .cvs file is used"<<endl;
        return -1;
	}

	cout << "Starting Throughput Test"<< endl;

	switch (type)
	{
	case 1:
	{
		ThroughputPublisher tpub(reliable, pid);
		tpub.file_name = file_name;
		tpub.run(test_time_sec, recovery_time_ms, demand, msg_size);
		break;
	}
	case 2:
	{
		ThroughputSubscriber tsub(reliable, pid);
		tsub.run();
		break;
	}
	}

	Domain::stopAll();


	return 0;
}

#if defined(_MSC_VER)
#pragma warning (pop)
#endif