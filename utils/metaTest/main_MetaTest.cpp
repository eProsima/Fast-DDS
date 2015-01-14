/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/



#include <stdio.h>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstdint>
#include <sstream>

#include "fastrtps/utils/RTPSLog.h"

#include "MetaTestPublisher.h"
#include "MetaTestSubscriber.h"


using namespace eprosima;
using namespace std;


int main(int argc, char** argv){
	Log::setVerbosity(VERB_ERROR);
	#if defined(_DEBUG)
	Log::setVerbosity(VERB_INFO);
	Log::logFileName("MetaTest",true);
	#endif




	logUser("Starting");
	int type;
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
	}
	else
	{
		cout << "NEEDS publisher OR subscriber ARGUMENT"<<endl;
		cout << "MetaTest publisher"<<endl;
		cout << "MetaTest subscriber" <<endl;
		return 0;
	}
	switch (type)
	{
	case 1:
	{
		MetaTestPublisher mPub;
		if(mPub.init())
			mPub.run();
		break;
	}
	case 2:
	{
		MetaTestSubscriber mSub;
		if(mSub.init())
			mSub.run();
		break;
	}
	}



	return 0;
}



