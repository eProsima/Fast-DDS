/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/



#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstdint>
#include <sstream>



#include "fastrtps/fastrtps_all.h"

#include "LatencyTestPublisher.h"
#include "LatencyTestSubscriber.h"

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

int main(int argc, char** argv){
	Log::setVerbosity(VERB_INFO);
	Log::logFileName("LatencyTest.txt",true);
	//cout << "Starting "<< endl;
	logUser("Starting");
	int type;
	int sub_number = 1;
	int n_samples = c_n_samples;
	bool echo = true;
	if(argc > 1)
	{
		if(strcmp(argv[1],"publisher")==0)
			type = 1;
		else if(strcmp(argv[1],"subscriber")==0)
			type = 2;
		else
		{
			cout << "NEEDS publisher OR subscriber as first argument"<<endl;
			return 0;
		}
		if(argc > 2 && type == 1)
		{
			std::istringstream iss( argv[2] );
			if (!(iss >> sub_number))
			{
				cout << "Problem reading subscriber number "<< endl;
			}
			if(argc > 3) //READ SAMPLES NUMBER
			{
				std::istringstream iss2( argv[3] );
				if (!(iss2 >> n_samples))
				{
					cout << "Problem reading subscriber number "<< endl;
					n_samples = c_n_samples;
				}
				else if (n_samples < 10)
				{
					cout << "Samples number must be >= 10"<<endl;
					n_samples = 10;
				}
				else
				{
					cout << "Reading number of samples: "<< n_samples << endl;
				}
			}
		}
		else if(argc > 2 && type == 2)
		{
			if(strcmp(argv[2],"echo")==0)
			{
				cout << "Subscriber will ECHO"<<endl;
				echo = true;
			}
			else if(strcmp(argv[2],"noecho")==0)
			{
				cout << "Subscriber will NOT ECHO"<<endl;
				echo = false;
			}
			else
			{
				cout << "Second argument of subscribers needs to be echo or noecho"<<endl;
				return 0;
			}
			if(argc > 3) //READ SAMPLES NUMBER
			{
				std::istringstream iss( argv[3] );
				if (!(iss >> n_samples))
				{
					cout << "Problem reading subscriber number "<< endl;
					n_samples = c_n_samples;
				}
				else if (n_samples < 10)
				{
					cout << "Samples number must be >= 10"<<endl;
					n_samples = 10;
				}
				else
				{
					cout << "Reading number of samples: "<< n_samples << endl;
				}
			}
		}

	}
	else
	{
		cout << "NEEDS publisher OR subscriber ARGUMENT"<<endl;
		cout << "LatencyTest publisher NUM_SUBSCRIBERS NUM_SAMPLES"<<endl;
		cout << "LatencyTest subscriber echo/noecho NUM_SAMPLES" <<endl;
		return 0;
	}

	switch (type)
	{
	case 1:
	{
		cout << "Performing test with "<< sub_number << " subscribers and "<<n_samples << " samples" <<endl;
		LatencyTestPublisher latencyPub;
		latencyPub.init(sub_number,n_samples);
		latencyPub.run();
		break;
	}
	case 2:
	{
		LatencyTestSubscriber latencySub;
		latencySub.init(echo,n_samples);
		latencySub.run();
		break;
	}
	}

	Domain::stopAll();
	cout << "EVERYTHING STOPPED FINE"<<endl;

	return 0;
}



