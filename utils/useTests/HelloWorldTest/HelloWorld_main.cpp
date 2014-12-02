/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HelloWorld_main.cpp
 *
 */

#include "HelloWorldPublisher.h"
#include "HelloWorldSubscriber.h"

#include "fastrtps/Domain.h"

#include "fastrtps/utils/eClock.h"

int main(int argc, char** argv)
{

	cout << "Starting "<< endl;
	int type = 1;
	if(argc > 1)
	{
		if(strcmp(argv[1],"publisher")==0)
			type = 1;
		else if(strcmp(argv[1],"subscriber")==0)
			type = 2;
	}
	else
	{
		cout << "publisher OR subscriber argument needed"<<endl;
		return 0;
	}


	switch(type)
	{
	case 1:
	{
		HelloWorldPublisher mypub;
		for(int i = 0;i<10;++i)
		{
			if(mypub.publish())
			{
//				int aux;
//				std::cin>>aux;
				eClock::my_sleep(500);
			}
			else
			{
				//cout << "Sleeping till discovery"<<endl;
				eClock::my_sleep(200);
				--i;
			}
		}
		break;
	}
	case 2:
	{
		HelloWorldSubscriber mysub;
		cout << "Enter number to stop: ";
		int aux;
		std::cin>>aux;
		break;
	}
	}

	Domain::stopAll();

	return 0;
}
