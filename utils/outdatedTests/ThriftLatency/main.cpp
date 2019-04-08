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
#include <cstring>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstdint>
#include <sstream>
#include <vector>

#include "ApacheServer.h"
#include "ApacheClient.h"

#if defined(__LITTLE_ENDIAN__)
const Endianness_t DEFAULT_ENDIAN = LITTLEEND;
#elif defined (__BIG_ENDIAN__)
const Endianness_t DEFAULT_ENDIAN = BIGEND;
#endif

using namespace std;

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

void printResultTile()
{
	printf("Samples   ,     Bytes,  Time(us)\n");
	printf("----------,----------,----------\n");
}

void printResult(int samples,int bytes,double res )
{
	printf("%10d,%10d,%10.3f\n",samples, bytes,res);
}


int main(int argc, char** argv){
	//RTPSLog::setVerbosity(EPROSIMA_DEBUGINFO_VERB_LEVEL);
	cout << "Starting "<< endl;
	E_SIDE side;
	int samples = 10000;
	int bytes = 0;
	string ip = "127.0.0.1";
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

		if(argc > 2) {
			std::istringstream iss1( argv[2] );
			if (!(iss1 >> samples))
			{
				cout << "Problem reading samples number,using default (10000) "<< endl;
				samples = 10000;
			}
		}

		if(argc > 3) {
			std::istringstream iss2( argv[3] );
			if (!(iss2 >> bytes))
			{
				cout << "Problem reading bytes number, using all sizes "<< endl;
				bytes = 0;
			}
		}

		if(argc > 4) {
			ip = argv[4];
		}
	}
	else
	{
		cout << "usage: " << argv[0] << " (client/server) [samples] [bytes] [serverIP]"<<endl;
		return 0;
	}


	if(side == SERVER)
	{
		ApacheServer server;
		server.serve();
	}

	if(side == CLIENT)
	{
		ApacheClientTest clienttest;
		printResultTile();
		if(bytes == 0)
		{
			int bytessizes[] = {16,32,64,128,256,512,1024,2048,4096,8192};
			std::vector<int> v_bytes (bytessizes, bytessizes + sizeof(bytessizes) / sizeof(uint32_t) );
			for(std::vector<int>::iterator it = v_bytes.begin();it!=v_bytes.end();++it)
			{
				double result = clienttest.run(ip, samples, *it);
				printResult(samples, *it,result);
			}
		}
		else
		{
			double result = clienttest.run(ip, samples, bytes);
			printResult(samples, bytes,result);
		}
	}
	cout << "EVERYTHING STOPPED FINE"<<endl;

	return 0;
}




