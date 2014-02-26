/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Participant.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "eprosimartps/Participant.h"
#include "eprosimartps/StatelessWriter.h"
#include "eprosimartps/RTPSReader.h"
#include "eprosimartps/RTPSWriter.h"


namespace eprosima {
namespace rtps {

Participant::Participant() {
	// TODO Auto-generated constructor stub
	//So the the service nevers stops until we say so

}

Participant::~Participant() {
	// TODO Auto-generated destructor stub

}

bool Participant::createStatelessWriter(StatelessWriter* SWriter,WriterParams_t Wparam) {
	SWriter->init(Wparam); //initialize with the correct values
	//Look for receiving threads that are already listening to this writer receiving addreesses.
	assignEnpointToListenThreads((Endpoint*)SWriter,'W');

	return true;
}


inline void addEndpoint(ThreadListen*th,Endpoint* end,char type)
{
	if(type == 'W')
		th->assoc_writers.push_back((RTPSWriter*)end);
	else if(type =='R')
		th->assoc_readers.push_back((RTPSReader*)end);
}


bool Participant::assignEnpointToListenThreads(Endpoint* endpoint, char type) {
	if(type !='R' && type!='W')
		throw ERR_PARTICIPANT_INCORRECT_ENDPOINT_TYPE;
	std::vector<ThreadListen>::iterator thit;
	std::vector<Locator_t>::iterator locit_th;
	std::vector<Locator_t>::iterator locit_e;
	bool assigned = false;

	for(locit_e = endpoint->unicastLocatorList.begin();locit_e!=endpoint->unicastLocatorList.end();locit_e++)
	{
		assigned = false;
		for(thit=threadListenList.begin();thit!=threadListenList.end();thit++)
		{
			for(locit_th = thit->locList.begin();locit_th != thit->locList.end();locit_th++)
			{
				if(*locit_th == *locit_e) //Found a match, assign to this thread
				{
					addEndpoint(&(*thit),endpoint,type);
					assigned = true;
				}
			}
		}
		if(!assigned) //Create new listen thread
		{
			ThreadListen* thListen = NULL;
			addNewListenThread(*locit_e,thListen); //Add new listen thread to participant
			addEndpoint(thListen,endpoint,type); //add endpoitn to that listen thread
			assigned = true;
		}
	}
	for(locit_e = endpoint->multicastLocatorList.begin();locit_e!=endpoint->multicastLocatorList.end();locit_e++)
	{
		assigned = false;
		for(thit=threadListenList.begin();thit!=threadListenList.end();thit++)
		{
			for(locit_th = thit->locList.begin();locit_th != thit->locList.end();locit_th++)
			{
				if(*locit_th == *locit_e) //Found a match, assign to this thread
				{
					addEndpoint(&(*thit),endpoint,type);
					assigned = true;
				}
			}
		}
		if(!assigned) //Create new listen thread
		{
			ThreadListen* thListen = NULL;
			addNewListenThread(*locit_e,thListen); //Add new listen thread to participant
			addEndpoint(thListen,endpoint,type);   //add Endpoint to that listen thread
			endpoint->endpointThreadListenList.push_back(thListen);
		}
	}
	return true;
}

bool Participant::addNewListenThread(Locator_t loc,ThreadListen* thlisten) {
	ThreadListen th;
	th.locList.push_back(loc);
	threadListenList.push_back(th);
	thlisten = &(*(threadListenList.end()-1));
	thlisten->init_thread();
	return true;
}

bool Participant::removeEndpoint(Endpoint* end){
	std::vector<RTPSWriter*>::iterator wit;
	std::vector<RTPSReader*>::iterator rit;
	bool found = false;
	char type = 'W';
	for(wit=writerList.begin();wit!=writerList.end();wit++)
	{
		if((*wit)->guid == end->guid) //Found it
		{
			writerList.erase(wit);
			found = true;
			break;
		}
	}
	if(!found)
	{
		for(rit=readerList.begin();rit!=readerList.end();rit++)
		{
			if((*rit)->guid == end->guid) //Found it
			{
				readerList.erase(rit);
				found = true;
				type = 'R';
				break;
			}
		}
	}
	if(!found)
		return false;
	//Remove it from threadListenList
	std::vector<ThreadListen>::iterator thit;
	for(thit=threadListenList.begin();thit!=threadListenList.end();thit++)
	{
		if(type == 'W')
		{
			for(wit = thit->assoc_writers.begin();wit!=thit->assoc_writers.end();wit++)
			{
				if((*wit)->guid == end->guid)
				{
					thit->assoc_writers.erase(wit);
					if(thit->assoc_writers.empty() && thit->assoc_readers.empty())
						threadListenList.erase(thit);

				}
			}
		}
		else if(type == 'R')
		{
			for(rit = thit->assoc_readers.begin();rit!=thit->assoc_readers.end();rit++)
			{
				if((*rit)->guid == end->guid)
				{
					thit->assoc_readers.erase(rit);
					if(thit->assoc_readers.empty() && thit->assoc_writers.empty())
						threadListenList.erase(thit);

				}
			}
		}
	}
	return true;
}


} /* namespace rtps */
} /* namespace eprosima */

