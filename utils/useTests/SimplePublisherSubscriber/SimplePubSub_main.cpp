/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HelloWorld_main.cpp
 *
 */

#include "eprosimartps/rtps_all.h"
#include "SimplePublisher.h"
#include "SimpleSubscriber.h"
#include "SimplePubSubType.h"


int main(int argc, char** argv)
{
	RTPSLog::setVerbosity(EPROSIMA_WARNING_VERB_LEVEL);
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

	//REGISTER THE TYPE BEING USED
	SimplePubSubType* myType = new SimplePubSubType();
	DomainParticipant::registerType((DDSTopicDataType*)myType);

	switch(type)
	{
	case 1:
	{
		SimplePublisher mypub;
		if(mypub.init())
			mypub.run();

		break;
	}
	case 2:
	{
		SimpleSubscriber mysub;
		if(mysub.init())
			mysub.run();
		break;
	}
	}
	delete(myType);
	DomainParticipant::stopAll();

	return 0;
}
