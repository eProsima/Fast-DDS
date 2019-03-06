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

#include "EprosimaServer.h"
#include "EprosimaClientTest.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using std::cout;
using std::endl;

#if defined(_WIN32)
#define COPYSTR strcpy_s
#else
#define COPYSTR strcpy
#endif

enum E_SIDE
{
	CLIENT,
	SERVER
};


int main(int argc, char** argv){
	cout << "Starting "<< endl;
	E_SIDE side;
	int samples = 10000;
	if(argc > 1)
	{
		if(strcmp(argv[1],"client")==0)
			side = CLIENT;
		else if(strcmp(argv[1],"server")==0)
			side = SERVER;
		else
		{
			cout << "Argument 1 needs to be client OR server"<<endl;
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
		cout << "Client Server Test needs 1 arguments: (client/server)"<<endl;
		return 0;
	}

	if(side == SERVER)
	{
		EprosimaServer server;
		server.init();
		server.serve();
	}
	if(side == CLIENT)
	{
		EprosimaClientTest clienttest;
		double result = clienttest.run(samples);
		if(result > 0)
			cout << "Mean Time of " << samples << " samples: " << result << " us" << endl;
		else
			cout << "Some problem with the test "<<endl;
	}



	cout << "EVERYTHING STOPPED FINE"<<endl;

	return 0;
}




