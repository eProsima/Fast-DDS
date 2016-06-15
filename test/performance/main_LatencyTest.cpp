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

#include "LatencyTestPublisher.h"
#include "LatencyTestSubscriber.h"

#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstdint>
#include <sstream>

#include <fastrtps/fastrtps_all.h>

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable:4512)
#endif

#include <boost/program_options.hpp>

using namespace eprosima;
using namespace rtps;
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

const int c_n_samples = 10000;

int main(int argc, char** argv)
{
    const char* const LATENCY_TEST_USAGE = "Usage: LatencyTest <publisher|subscriber>\n";
    const char* const LATENCY_TEST_USAGE_SUBSCRIBER = "Usage: LatencyTest subscriber\n";

	Log::setVerbosity(VERB_ERROR);

    boost::program_options::options_description p_optionals("Publisher options");
    p_optionals.add_options()
        ("subscribers,n", boost::program_options::value<int>()->default_value(1), "number of subscribers")
        ;

    boost::program_options::options_description s_optionals("Subscriber options");
    s_optionals.add_options()
        ("echo,e", boost::program_options::value<string>()->default_value("true"), "echo mode (\"true\"/\"false\")")
        ;

    boost::program_options::options_description g_optionals("General options");
    g_optionals.add_options()
        ("help,h", "produce help message")
        ("reliability,r", boost::program_options::value<string>()->default_value("besteffort"), "set reliability (\"reliable\"/\"besteffort\")")
		("samples,s", boost::program_options::value<int>()->default_value(c_n_samples), "number of samples")
		("seed", boost::program_options::value<uint32_t>()->default_value(80), "seed to calculate domain and topic, to isolate test")
        ;

    boost::program_options::options_description visible_optionals("Allowed options");
    visible_optionals.add(g_optionals).add(p_optionals).add(s_optionals);

    boost::program_options::options_description visible_p_optionals("Allowed options");
    visible_p_optionals.add(g_optionals).add(p_optionals);

    boost::program_options::options_description visible_s_optionals("Allowed options");
    visible_s_optionals.add(g_optionals).add(s_optionals);

    boost::program_options::options_description all_optionals("Allowed options");
	all_optionals.add_options()
		("hostname", "use hostname in topic name")
		("export_csv", "export csv")
        ;
    all_optionals.add(g_optionals).add(p_optionals).add(s_optionals);


	int type;
	int sub_number = 1;
	int n_samples = c_n_samples;
	bool echo = true;
    bool reliable = false;
	uint32_t seed = 80;
	bool hostname = false;
	bool export_csv = false;

	if (argc > 1)
	{
		if (strcmp(argv[1], "publisher") == 0)
			type = 1;
		else if (strcmp(argv[1], "subscriber") == 0)
			type = 2;
		else
		{
			cout << LATENCY_TEST_USAGE << visible_optionals << endl;
			return -1;
		}

		boost::program_options::variables_map vm;
		try
		{
			boost::program_options::store(boost::program_options::parse_command_line(argc - 1, argv + 1, all_optionals), vm);
			boost::program_options::notify(vm);
		}
		catch (std::exception ex)
		{
			cout << "Error: " << ex.what() << std::endl;
			cout << LATENCY_TEST_USAGE << visible_optionals << endl;
			return -1;
		}

		if (type == 1)
		{
			sub_number = vm["subscribers"].as<int>();
		}
		else
		{
			string s_echo = vm["echo"].as<std::string>();

			if (s_echo.compare("true") == 0)
			{
				echo = true;
			}
			else if (s_echo.compare("false") == 0)
			{
				echo = false;
			}
			else
			{
				cout << "Bad argument for option --echo. Valid values: \"true\"/\"false\".\n" << endl;
				cout << LATENCY_TEST_USAGE_SUBSCRIBER << visible_s_optionals << endl;
				return -1;
			}
		}

		if (vm.count("help"))
		{
			cout << LATENCY_TEST_USAGE << visible_optionals << endl;
			return 0;
		}

		std::string reliability = vm["reliability"].as<std::string>();

		if (reliability.compare("reliable") == 0)
		{
			reliable = true;
		}
		else if (reliability.compare("besteffort") == 0)
		{
			reliable = false;
		}
		else
		{
			cout << "Bad argument for option --reliability. Valid values: \"reliable\"/\"besteffort\".\n" << endl;
			cout << LATENCY_TEST_USAGE << visible_optionals << endl;
			return -1;
		}

		n_samples = vm["samples"].as<int>();
		seed = vm["seed"].as<uint32_t>();

		if (vm.count("hostname"))
			hostname = true;
		if (vm.count("export_csv"))
			export_csv = true;
	}
	else
	{
        cout << LATENCY_TEST_USAGE << visible_optionals << endl;
		return -1;
	}

	std::cout << "Starting" << std::endl;

    switch (type)
    {
        case 1:
            {
                cout << "Performing test with "<< sub_number << " subscribers and "<<n_samples << " samples" <<endl;
                LatencyTestPublisher latencyPub;
                latencyPub.init(sub_number,n_samples, reliable, seed, hostname, export_csv);
                latencyPub.run();
                break;
            }
        case 2:
            {
                LatencyTestSubscriber latencySub;
				latencySub.init(echo, n_samples, reliable, seed, hostname);
                latencySub.run();
                break;
            }
    }

	eClock::my_sleep(1000);
	
	cout << "EVERYTHING STOPPED FINE"<<endl;

	return 0;
}

#if defined(_MSC_VER)
#pragma warning (pop)
#endif
