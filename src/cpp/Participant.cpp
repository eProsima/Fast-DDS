/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * Participant.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *      		grcanosa@gmail.com
 */

#include "eprosimartps/Participant.h"

#include "eprosimartps/StatelessWriter.h"
#include "eprosimartps/StatelessReader.h"
#include "eprosimartps/StatefulWriter.h"
#include "eprosimartps/RTPSReader.h"
#include "eprosimartps/RTPSWriter.h"

#include "eprosimartps/DomainParticipant.h"



namespace eprosima {
namespace rtps {

Participant::Participant() {

	endpointToListenThreadSemaphore = new boost::interprocess::interprocess_semaphore(0);

	// TODO Auto-generated constructor stub
	PROTOCOLVERSION(protocolVersion);
	VENDORID_EPROSIMA(vendorId);
	IdCounter = 0;
	//Create send socket in threadSend
	//threadSend();
	//Initialize threadlisten list to 0;
	threadListenList.clear();
	//cout << "Listen thread with size: " << threadListenList.size() << endl;
	//Initialize default Unicast Multicast locators.
	//TODOG Definir default send and receive locator.
	Locator_t defUni;
	defUni.kind = LOCATOR_KIND_UDPv4;
	for(uint8_t i=0;i<16;i++)
		defUni.address[i]= threadSend.sendLocator.address[i];
	defUni.port = 14445;
	defaultUnicastLocatorList.push_back(defUni);

	// Create Unique GUID
	dds::DomainParticipant *dp;
	dp = dds::DomainParticipant::getInstance();
	uint32_t ID = dp->getNewId();
	int pid;
	#if defined(_WIN32)
		pid = (int)_getpid();
	#else
		pid = (int)getpid();
	#endif
	//cout << "PID: " << pid << " ID:"<< ID << endl;

	guid.guidPrefix.value[0] = threadSend.sendLocator.address[12];
	guid.guidPrefix.value[1] = threadSend.sendLocator.address[13];
	guid.guidPrefix.value[2] = threadSend.sendLocator.address[14];
	guid.guidPrefix.value[3] = threadSend.sendLocator.address[15];
	guid.guidPrefix.value[4] = ((octet*)&pid)[0];
	guid.guidPrefix.value[5] = ((octet*)&pid)[1];
	guid.guidPrefix.value[6] = ((octet*)&pid)[2];
	guid.guidPrefix.value[7] = ((octet*)&pid)[3];
	guid.guidPrefix.value[8] = ((octet*)&ID)[0];
	guid.guidPrefix.value[9] = ((octet*)&ID)[1];
	guid.guidPrefix.value[10] = ((octet*)&ID)[2];
	guid.guidPrefix.value[11] = ((octet*)&ID)[3];
	guid.entityId = ENTITYID_PARTICIPANT;
	std::stringstream ss;
			for(int i =0;i<12;i++)
				ss << (int)guid.guidPrefix.value[i] << ".";
			pInfo("Participant created with guidPrefix: " <<ss.str()<< endl);
	//TODOG CREATE DEFAULT ENDPOINTS. (NOT YET)
}

Participant::Participant(ParticipantParams_t PParam) {

	endpointToListenThreadSemaphore = new boost::interprocess::interprocess_semaphore(0);

	// TODO Auto-generated constructor stub
	PROTOCOLVERSION(protocolVersion);
	VENDORID_EPROSIMA(vendorId);
	IdCounter = 0;
	//Create send socket in threadSend
	threadListenList.clear();
	defaultUnicastLocatorList = PParam.defaultUnicastLocatorList;
	defaultMulticastLocatorList = PParam.defaultMulticastLocatorList;
	Locator_t loc;
	loc.port = PParam.defaultSendPort;
	threadSend.initSend(loc);

	// Create Unique GUID
	dds::DomainParticipant *dp;
	dp = dds::DomainParticipant::getInstance();
	uint32_t ID = dp->getNewId();
	int pid;
	#if defined(_WIN32)
		pid = (int)_getpid();
	#else
		pid = (int)getpid();
	#endif
	//cout << "PID: " << pid << " ID:"<< ID << endl;

	guid.guidPrefix.value[0] = threadSend.sendLocator.address[12];
	guid.guidPrefix.value[1] = threadSend.sendLocator.address[13];
	guid.guidPrefix.value[2] = threadSend.sendLocator.address[14];
	guid.guidPrefix.value[3] = threadSend.sendLocator.address[15];
	guid.guidPrefix.value[4] = ((octet*)&pid)[0];
	guid.guidPrefix.value[5] = ((octet*)&pid)[1];
	guid.guidPrefix.value[6] = ((octet*)&pid)[2];
	guid.guidPrefix.value[7] = ((octet*)&pid)[3];
	guid.guidPrefix.value[8] = ((octet*)&ID)[0];
	guid.guidPrefix.value[9] = ((octet*)&ID)[1];
	guid.guidPrefix.value[10] = ((octet*)&ID)[2];
	guid.guidPrefix.value[11] = ((octet*)&ID)[3];
	guid.entityId = ENTITYID_PARTICIPANT;
	std::stringstream ss;
		for(int i =0;i<12;i++)
			ss << (int)guid.guidPrefix.value[i] << ".";
		pInfo("Participant created with guidPrefix: " <<ss.str()<< endl);
	//TODOG CREATE DEFAULT ENDPOINTS. (NOT YET)
}



Participant::~Participant() {
	// TODO Auto-generated destructor stub

	//Destruct threads:
	std::vector<ThreadListen*>::iterator it;
	for(it=threadListenList.begin();it!=threadListenList.end();it++)
		(*it)->~ThreadListen();

}

bool Participant::createStatelessWriter(StatelessWriter* SWriter,WriterParams_t Wparam) {

	if(SWriter == NULL)
		SWriter = new StatelessWriter();
	SWriter->init(Wparam); //initialize with the correct values
	//Check if locator lists are empty:
	if(SWriter->unicastLocatorList.empty())
		SWriter->unicastLocatorList = defaultUnicastLocatorList;
	if(SWriter->unicastLocatorList.empty())
		SWriter->multicastLocatorList = defaultMulticastLocatorList;
	//Assign participant pointer
	SWriter->participant = this;
	//Assign GUID
	SWriter->guid.guidPrefix = guid.guidPrefix;
	if(SWriter->topicKind == NO_KEY)
		SWriter->guid.entityId.value[3] = 0x03;
	else if(SWriter->topicKind == WITH_KEY)
		SWriter->guid.entityId.value[3] = 0x02;
	IdCounter++;
	octet* c = (octet*)&IdCounter;
	SWriter->guid.entityId.value[2] = c[0];
	SWriter->guid.entityId.value[1] = c[1];
	SWriter->guid.entityId.value[0] = c[2];
	//Look for receiving threads that are already listening to this writer receiving addreesses.
	assignEnpointToListenThreads((Endpoint*)SWriter,'W');
	//Wait until the thread is correctly created

	writerList.push_back((RTPSWriter*)SWriter);
	return true;
}

bool Participant::createStatefulWriter(StatefulWriter* SWriter,WriterParams_t Wparam) {

	if(SWriter == NULL)
		SWriter = new StatefulWriter();
	SWriter->init(Wparam); //initialize with the correct values
	//Check if locator lists are empty:
	if(SWriter->unicastLocatorList.empty())
		SWriter->unicastLocatorList = defaultUnicastLocatorList;
	if(SWriter->unicastLocatorList.empty())
		SWriter->multicastLocatorList = defaultMulticastLocatorList;
	//Assign participant pointer
	SWriter->participant = this;
	//Assign GUID
	SWriter->guid.guidPrefix = guid.guidPrefix;
	if(SWriter->topicKind == NO_KEY)
		SWriter->guid.entityId.value[3] = 0x03;
	else if(SWriter->topicKind == WITH_KEY)
		SWriter->guid.entityId.value[3] = 0x02;
	IdCounter++;
	octet* c = (octet*)&IdCounter;
	SWriter->guid.entityId.value[2] = c[0];
	SWriter->guid.entityId.value[1] = c[1];
	SWriter->guid.entityId.value[0] = c[2];
	//Look for receiving threads that are already listening to this writer receiving addreesses.
	assignEnpointToListenThreads((Endpoint*)SWriter,'W');
	//Wait until the thread is correctly created

	writerList.push_back((RTPSWriter*)SWriter);
	return true;
}




bool Participant::createStatelessReader(StatelessReader* SReader,
		ReaderParams_t RParam) {

	if(SReader == NULL)
		SReader = new StatelessReader();
	SReader->init(RParam);
	//If NO UNICAST
	if(SReader->unicastLocatorList.empty())
		SReader->unicastLocatorList = defaultUnicastLocatorList;
	//IF NO MULTICAST
	if(SReader->multicastLocatorList.empty())
		SReader->multicastLocatorList = defaultMulticastLocatorList;


	//Assign participant pointer
	SReader->participant = this;
	//Assign GUID
	SReader->guid.guidPrefix = guid.guidPrefix;
	if(SReader->topicKind == NO_KEY)
		SReader->guid.entityId.value[3] = 0x04;
	else if(SReader->topicKind == WITH_KEY)
		SReader->guid.entityId.value[3] = 0x07;
	IdCounter++;
	octet* c = (octet*)&IdCounter;
	SReader->guid.entityId.value[2] = c[3];
	SReader->guid.entityId.value[1] = c[2];
	SReader->guid.entityId.value[0] = c[1];
	//Look for receiving threads that are already listening to this writer receiving addreesses.

	assignEnpointToListenThreads((Endpoint*)SReader,'R');

	readerList.push_back((RTPSReader*)SReader);
	return true;



}

inline void addEndpoint(ThreadListen* th,Endpoint* end,char type)
{
	if(type == 'W')
		th->assoc_writers.push_back((RTPSWriter*)end);
	else if(type =='R')
		th->assoc_readers.push_back((RTPSReader*)end);
}


bool Participant::assignEnpointToListenThreads(Endpoint* endpoint, char type) {
	if(type !='R' && type!='W')
		throw ERR_PARTICIPANT_INCORRECT_ENDPOINT_TYPE;

	std::vector<ThreadListen*>::iterator thit;
	std::vector<Locator_t>::iterator locit_th;
	std::vector<Locator_t>::iterator locit_e;
	bool assigned = false;

	for(locit_e = endpoint->unicastLocatorList.begin();locit_e!=endpoint->unicastLocatorList.end();locit_e++)
	{
		assigned = false;
		for(thit=threadListenList.begin();thit!=threadListenList.end();thit++)
		{
			for(locit_th = (*thit)->locList.begin();locit_th != (*thit)->locList.end();locit_th++)
			{
				if((*locit_th).port == (*locit_e).port) //Found a match, assign to this thread
				{
					addEndpoint(*thit,endpoint,type);
					assigned = true;
				}
			}
		}
		if(!assigned) //Create new listen thread
		{
			ThreadListen* thListen = NULL;
			addNewListenThread(*locit_e,&thListen); //Add new listen thread to participant
			endpointToListenThreadSemaphore->wait();
			addEndpoint(thListen,endpoint,type); //add endpoint to that listen thread
			assigned = true;
		}
	}
	for(locit_e = endpoint->multicastLocatorList.begin();locit_e!=endpoint->multicastLocatorList.end();locit_e++)
	{
		assigned = false;
		for(thit=threadListenList.begin();thit!=threadListenList.end();thit++)
		{
			for(locit_th = (*thit)->locList.begin();locit_th != (*thit)->locList.end();locit_th++)
			{
				if((*locit_th).port == (*locit_e).port) //Found a match, assign to this thread
				{
					addEndpoint((*thit),endpoint,type);
					assigned = true;
				}
			}
		}
		if(!assigned) //Create new listen thread
		{
			ThreadListen* thListen = NULL;
			addNewListenThread(*locit_e,&thListen); //Add new listen thread to participant
			endpointToListenThreadSemaphore->wait();
			addEndpoint(thListen,endpoint,type);   //add Endpoint to that listen thread
			assigned = true;
		}
	}
	return true;
}

bool Participant::addNewListenThread(Locator_t loc,ThreadListen** thlisten_in) {
	*thlisten_in = new ThreadListen();
	(*thlisten_in)->locList.push_back(loc);
	(*thlisten_in)->participant = this;
	threadListenList.push_back(*thlisten_in);
	(*thlisten_in)->init_thread();

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
	std::vector<ThreadListen*>::iterator thit;
	for(thit=threadListenList.begin();thit!=threadListenList.end();thit++)
	{
		if(type == 'W')
		{
			for(wit = (*thit)->assoc_writers.begin();wit!=(*thit)->assoc_writers.end();wit++)
			{
				if((*wit)->guid == end->guid)
				{
					(*thit)->assoc_writers.erase(wit);
					if((*thit)->assoc_writers.empty() && (*thit)->assoc_readers.empty())
						threadListenList.erase(thit);

				}
			}
		}
		else if(type == 'R')
		{
			for(rit = (*thit)->assoc_readers.begin();rit!=(*thit)->assoc_readers.end();rit++)
			{
				if((*rit)->guid == end->guid)
				{
					(*thit)->assoc_readers.erase(rit);
					if((*thit)->assoc_readers.empty() && (*thit)->assoc_writers.empty())
						threadListenList.erase(thit);

				}
			}
		}
	}
	return true;
}


} /* namespace rtps */
} /* namespace eprosima */


