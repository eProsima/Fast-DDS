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

/**
 * @file main_ClientServerTest.cpp
 *
 */



#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstdint>
#include <sstream>

#include "fastrtps/rtps_all.h"

#include "eprosima/EprosimaServer.h"
#include "eprosima/EprosimaClientTest.h"

#include "apache/ApacheServer.h"
#include "apache/ApacheClientTest.h"

#if defined(__LITTLE_ENDIAN__)
const Endianness_t DEFAULT_ENDIAN = LITTLEEND;
#elif defined (__BIG_ENDIAN__)
const Endianness_t DEFAULT_ENDIAN = BIGEND;
#endif

using namespace eprosima;
using namespace dds;
using namespace rtps;
using namespace std;

#if defined(_WIN32)
#define COPYSTR strcpy_s
#else
#define COPYSTR strcpy
#endif

enum E_VENDOR
{
	EPROSIMA,
	APACHE
};
enum E_SIDE
{
	CLIENT,
	SERVER
};


int main(int argc, char** argv){
	RTPSLog::setVerbosity(EPROSIMA_DEBUGINFO_VERB_LEVEL);
	cout << "Starting "<< endl;
	pInfo("Starting"<<endl)
	E_VENDOR vendor;
	E_SIDE side;
	int samples = 10000;
	if(argc > 2)
	{
		if(strcmp(argv[1],"eprosima")==0)
			vendor = EPROSIMA;
		else if(strcmp(argv[1],"apache")==0)
			vendor = APACHE;
		else
		{
			cout << "Argument 1 needs to be eprosima OR apache"<<endl;
			return 0;
		}
		if(strcmp(argv[2],"client")==0)
			side = CLIENT;
		else if(strcmp(argv[2],"server")==0)
			side = SERVER;
		else
		{
			cout << "Argument 2 needs to be client OR server"<<endl;
			return 0;
		}
		if(argc > 3)
		{
			std::istringstream iss( argv[3] );
			if (!(iss >> samples))
			{
				cout << "Problem reading samples number,using default 10000 samples "<< endl;
				samples = 10000;
			}
		}
	}
	else
	{
		cout << "Client Server Test needs 2 arguments: (eprosima/apache) (client/server)"<<endl;
		return 0;
	}

	if(vendor == EPROSIMA && side == SERVER)
	{
		EprosimaServer server;
		server.init();
		server.serve();
	}
	if(vendor == APACHE && side == SERVER)
	{
		ApacheServer server;
		server.serve();
	}
	if(vendor == EPROSIMA && side == CLIENT)
	{
		EprosimaClientTest clienttest;
		double result = clienttest.run(samples);
		if(result > 0)
			cout << "Mean Time of " << samples << " samples: " << result << " us" << endl;
		else
			cout << "Some problem with the test "<<endl;
	}
	if(vendor == APACHE && side == CLIENT)
	{
		ApacheClientTest clienttest;
		double result = clienttest.run(samples);
		if(result > 0)
			cout << "Mean Time of " << samples << " samples: " << result << " us" << endl;
		else
			cout << "Some problem with the test "<<endl;
	}


	DomainRTPSParticipant::stopAll();
	cout << "EVERYTHING STOPPED FINE"<<endl;

	return 0;
}




