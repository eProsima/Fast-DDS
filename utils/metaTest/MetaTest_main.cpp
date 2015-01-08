/*************************************************************************
 * Copyright (c) 2015 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file MetaTest_main.cpp
 *
 */

#include "MetaTestPublisher.h"
#include "MetaTestSubscriber.h"

#include "fastrtps/Domain.h"

#include "fastrtps/utils/eClock.h"
#include "fastrtps/utils/RTPSLog.h"

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;
int main(int argc, char** argv)
{
	Log::setVerbosity(VERB_INFO);
	int type = 0;
	if(argc > 1)
	{
		if(strcmp(argv[1],"publisher")==0)
			type = 1;
		else if(strcmp(argv[1],"subscriber")==0)
			type = 2;
	}
	else
	{
		cout << "\"publisher\" OR \"subscriber\" argument needed."<<endl;
		return 0;
	}


	switch(type)
	{
	case 1:
	{
		MetaTestPublisher metaPub;
		if(metaPub.init())
			metaPub.run();
		break;
	}
	case 2:
	{
		MetaTestSubscriber metaSub;
		if(metaSub.init())
			metaSub.run();

		break;
	}
	}

	Domain::stopAll();

	return 0;
}
