/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HelloWorld_main.cpp
 *
 */

#include "HelloWorldPublisher.h"
#include "HelloWorldSubscriber.h"

#include <fastrtps/Domain.h>

#include <fastrtps/utils/eClock.h>
#include <fastrtps/utils/RTPSLog.h>

#include <fastrtps/rtps/rtps_all.h>

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;
int main(int argc, char** argv)
{
	Log::setVerbosity(VERB_ERROR);
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
		if(mypub.init())
		{
			mypub.run(10);
		}
		break;
	}
	case 2:
	{
		HelloWorldSubscriber mysub;
		if(mysub.init())
		{
			mysub.run();
		}
		break;
	}
	}
	Domain::stopAll();
	return 0;
}
