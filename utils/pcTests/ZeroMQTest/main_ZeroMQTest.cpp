/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file main_ClientServerTest.cpp
 *
 */


//#include <stdio.h>
//#include <unistd.h>
//#include <string.h>
//#include <assert.h>
#include <iostream>

#include "ZeroMQPublisher.h"
#include "ZeroMQSubscriber.h"

using namespace std;

int main (int argc, char** argv)
{
	int type = 0;
	std::string ipstr;
	int nsamples;
	if(argc > 3)
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
		std::istringstream ip(argv[2]);
		ipstr = ip.str();
		std::istringstream nsampl( argv[3] );
		if(!(nsampl>>nsamples))
		{
			cout << "Problem reading samples number"<<endl;
		}
	}
	else
	{
		cout << "Application needs at least 3 arguments: publisher/subscriber SUB_IP/PUB_IP N_SAMPLES"<<endl;
		return 0;
	}
	if(type == 1)
	{
		ZeroMQPublisher zmqpub;
		zmqpub.init(ipstr,nsamples);
		zmqpub.run();
	}
	else if(type ==2)
	{
		ZeroMQSubscriber zmqsub;
		zmqsub.init(ipstr,nsamples);
		zmqsub.run();
	}
	else
		return 0;

}

