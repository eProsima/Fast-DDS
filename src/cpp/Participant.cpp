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

#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/reader/StatelessReader.h"
#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/reader/RTPSReader.h"
#include "eprosimartps/writer/RTPSWriter.h"

#include "eprosimartps/dds/DomainParticipant.h"



namespace eprosima {
namespace rtps {


Participant::Participant(const ParticipantParams_t& PParam):
				m_defaultUnicastLocatorList(PParam.defaultUnicastLocatorList),
				m_defaultMulticastLocatorList(PParam.defaultMulticastLocatorList),
				m_ThreadSemaphore(new boost::interprocess::interprocess_semaphore(0)),
				IdCounter(0),
				m_SPDP(this),
				m_StaticEDP(this)
{
	Locator_t loc;
	loc.port = PParam.defaultSendPort;
	m_send_thr.initSend(loc);

	m_event_thr.init_thread();

	// Create Unique GUID
	dds::DomainParticipant *dp = dds::DomainParticipant::getInstance();
	uint32_t ID = dp->getNewId();
	int pid;
#if defined(_WIN32)
	pid = (int)_getpid();
#else
	pid = (int)getpid();
#endif
	//cout << "PID: " << pid << " ID:"<< ID << endl;

	m_guid.guidPrefix.value[0] = m_send_thr.m_sendLocator.address[12];
	m_guid.guidPrefix.value[1] = m_send_thr.m_sendLocator.address[13];
	m_guid.guidPrefix.value[2] = m_send_thr.m_sendLocator.address[14];
	m_guid.guidPrefix.value[3] = m_send_thr.m_sendLocator.address[15];
	m_guid.guidPrefix.value[4] = ((octet*)&pid)[0];
	m_guid.guidPrefix.value[5] = ((octet*)&pid)[1];
	m_guid.guidPrefix.value[6] = ((octet*)&pid)[2];
	m_guid.guidPrefix.value[7] = ((octet*)&pid)[3];
	m_guid.guidPrefix.value[8] = ((octet*)&ID)[0];
	m_guid.guidPrefix.value[9] = ((octet*)&ID)[1];
	m_guid.guidPrefix.value[10] = ((octet*)&ID)[2];
	m_guid.guidPrefix.value[11] = ((octet*)&ID)[3];
	m_guid.entityId = ENTITYID_PARTICIPANT;
	std::stringstream ss;
	for(int i =0;i<12;i++)
		ss << (int)m_guid.guidPrefix.value[i] << ".";

	m_participantName = PParam.name;
	pInfo("Participant \"" <<  m_participantName << "\" with guidPrefix: " <<ss.str()<< endl);

	//	cout << "PParam name: "<< PParam.name << endl;
	//	cout << "Participant name: " << m_participantName << endl;
	m_useStaticEDP = PParam.m_useSimpleParticipantDiscovery;
	if(PParam.m_useSimpleParticipantDiscovery)
	{
		m_SPDP.m_useStaticEDP = PParam.m_useStaticEndpointDiscovery;
		m_SPDP.initSPDP(PParam.domainId,ID,PParam.resendSPDPDataPeriod_sec);
		if(PParam.m_useStaticEndpointDiscovery)
		{
			m_StaticEDP.loadStaticEndpointFile(PParam.m_staticEndpointXMLFilename);
		}
		else
		{

		}
	}
}


Participant::~Participant()
{
	pDebugInfo("Participant destructor"<<endl;);
	//Destruct threads:
	for(std::vector<ThreadListen*>::iterator it=m_threadListenList.begin();
			it!=m_threadListenList.end();++it)
		delete(*it);

	for(std::vector<RTPSReader*>::iterator it=m_readerList.begin();
			it!=m_readerList.end();++it)
		delete(*it);

	for(std::vector<RTPSWriter*>::iterator it=m_writerList.begin();
			it!=m_writerList.end();++it)
		delete(*it);

	delete(this->m_ThreadSemaphore);
}

bool Participant::createStatelessWriter(StatelessWriter** SW_out,const WriterParams_t& Wparam,uint32_t payload_size)
{
	pDebugInfo("Creating Stateless Writer"<<endl);
	if(this->m_useStaticEDP && Wparam.userDefinedId == 0)
	{
		pError("Static EDP requires user defined Id"<<endl);
		return false;
	}
	StatelessWriter* SLWriter = new StatelessWriter(&Wparam,payload_size);
	if(this->initWriter((RTPSWriter*)SLWriter))
	{
		*SW_out = SLWriter;
		return true;
	}
	else
		return false;
}

bool Participant::createStatefulWriter(StatefulWriter** SFW_out,const WriterParams_t& Wparam,uint32_t payload_size)
{
	if(this->m_useStaticEDP && Wparam.userDefinedId == 0)
	{
		pError("Static EDP requires user defined Id"<<endl);
		return false;
	}
	StatefulWriter* SFWriter = new StatefulWriter(&Wparam, payload_size);
	if(this->initWriter((RTPSWriter*)SFWriter))
	{
		*SFW_out = SFWriter;
		return true;
	}
	else return false;
}

bool Participant::initWriter(RTPSWriter*W)
{
	pDebugInfo("Finished Writer creation"<<endl);
	//Check if locator lists are empty:
	if(W->unicastLocatorList.empty())
		W->unicastLocatorList = m_defaultUnicastLocatorList;
	if(W->unicastLocatorList.empty())
		W->multicastLocatorList = m_defaultMulticastLocatorList;
	//Assign participant pointer
	W->mp_send_thr = &this->m_send_thr;
	W->mp_event_thr = &this->m_event_thr;
	//Assign GUID
	W->m_guid.guidPrefix = m_guid.guidPrefix;
	W->init_header();

	if(W->topicKind == NO_KEY)
		W->m_guid.entityId.value[3] = 0x03;
	else if(W->topicKind == WITH_KEY)
		W->m_guid.entityId.value[3] = 0x02;
	IdCounter++;
	octet* c = (octet*)&IdCounter;
	W->m_guid.entityId.value[2] = c[0];
	W->m_guid.entityId.value[1] = c[1];
	W->m_guid.entityId.value[0] = c[2];
	//Look for receiving threads that are already listening to this writer receiving addresses.
	if(assignEnpointToListenThreads((Endpoint*)W,'W'))
	{
		//Wait until the thread is correctly created
		m_writerList.push_back(W);
		this->m_StaticEDP.localEndpointMatching((Endpoint*)W,'W');
		return true;
	}
	else
		return false;

}




bool Participant::createStatelessReader(StatelessReader** SR_out,
		const ReaderParams_t& RParam,uint32_t payload_size)
{
	if(this->m_useStaticEDP && RParam.userDefinedId == 0)
	{
		pError("Static EDP requires user defined Id"<<endl);
		return false;
	}
	StatelessReader* SReader = new StatelessReader(&RParam, payload_size);
	if(initReader((RTPSReader*)SReader))
	{
		*SR_out = SReader;
		return true;
	}
	else
		return false;
}

bool Participant::createStatefulReader(StatefulReader** SR_out,
		const ReaderParams_t& RParam,uint32_t payload_size)
{
	pDebugInfo("Creating StatefulReader"<<endl);
	if(this->m_useStaticEDP && RParam.userDefinedId == 0)
	{
		pError("Static EDP requires user defined Id"<<endl);
		return false;
	}
	StatefulReader* SReader = new StatefulReader(&RParam, payload_size);
	if(initReader((RTPSReader*)SReader))
	{
		*SR_out = SReader;
		return true;
	}
	else
		return false;
}



bool Participant::initReader(RTPSReader* p_R)
{
	//If NO UNICAST
	if(p_R->unicastLocatorList.empty())
		p_R->unicastLocatorList = m_defaultUnicastLocatorList;
	//IF NO MULTICAST
	if(p_R->multicastLocatorList.empty())
		p_R->multicastLocatorList = m_defaultMulticastLocatorList;
	//Assignthread pointers
	p_R->mp_send_thr = &this->m_send_thr;
	p_R->mp_event_thr = &this->m_event_thr;
	//Assign GUID
	p_R->m_guid.guidPrefix = m_guid.guidPrefix;
	if(p_R->topicKind == NO_KEY)
		p_R->m_guid.entityId.value[3] = 0x04;
	else if(p_R->topicKind == WITH_KEY)
		p_R->m_guid.entityId.value[3] = 0x07;
	IdCounter++;
	octet* c = (octet*)&IdCounter;
	p_R->m_guid.entityId.value[2] = c[0];
	p_R->m_guid.entityId.value[1] = c[1];
	p_R->m_guid.entityId.value[0] = c[2];

	//Look for receiving threads that are already listening to this writer receiving addresses.

	if(this->assignEnpointToListenThreads((Endpoint*)p_R,'R'))
	{
		m_readerList.push_back(p_R);
		this->m_StaticEDP.localEndpointMatching((Endpoint*)p_R,'R');
		return true;
	}
	else
		return false;


}





inline void addEndpoint(ThreadListen* th,Endpoint* end,char type)
{
	pInfo("Endpoint of type " << type << " added to listen Thread"<<endl);
	if(type == 'W')
		th->m_assoc_writers.push_back((RTPSWriter*)end);
	else if(type =='R')
		th->m_assoc_readers.push_back((RTPSReader*)end);
}


bool Participant::assignEnpointToListenThreads(Endpoint* endpoint, char type) {
	if(type !='R' && type!='W')
	{
		return false;
	}

	std::vector<ThreadListen*>::iterator thit;
	std::vector<Locator_t>::iterator locit_th;
	std::vector<Locator_t>::iterator locit_e;
	bool assigned = false;

	for(locit_e = endpoint->unicastLocatorList.begin();locit_e!=endpoint->unicastLocatorList.end();++locit_e)
	{
		assigned = false;
		for(thit=m_threadListenList.begin();
				thit!=m_threadListenList.end();++thit)
		{
			for(locit_th = (*thit)->m_locList.begin();
					locit_th != (*thit)->m_locList.end();++locit_th)
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
			if(addNewListenThread(*locit_e,&thListen,false))
			{//Add new listen thread to participant
				m_ThreadSemaphore->wait();
				addEndpoint(thListen,endpoint,type); //add endpoint to that listen thread
				assigned = true;
			}
			else
				return false;
		}
	}
	for(locit_e = endpoint->multicastLocatorList.begin();locit_e!=endpoint->multicastLocatorList.end();++locit_e)
	{
		//FIXME: in multicast the IP is important, change this.
		assigned = false;
		for(thit=m_threadListenList.begin();
				thit!=m_threadListenList.end();++thit)
		{
			for(locit_th = (*thit)->m_locList.begin();
					locit_th != (*thit)->m_locList.end();++locit_th)
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
			if(addNewListenThread(*locit_e,&thListen,true))
			{//Add new listen thread to participant
				m_ThreadSemaphore->wait();
				addEndpoint(thListen,endpoint,type);   //add Endpoint to that listen thread
				assigned = true;
			}
			else
				return false;
		}
	}
	return true;
}

bool Participant::addNewListenThread(Locator_t& loc,ThreadListen** thlisten_in,bool isMulticast) {
	*thlisten_in = new ThreadListen();
	(*thlisten_in)->m_locList.push_back(loc);
	(*thlisten_in)->m_isMulticast = isMulticast;
	(*thlisten_in)->m_participant_ptr = this;
	m_threadListenList.push_back(*thlisten_in);
	if((*thlisten_in)->init_thread())
		return true;
	else
		return false;
}

bool Participant::removeEndpoint(Endpoint* p_endpoint){
	bool found = false;
	char type = 'W';
	for(std::vector<RTPSWriter*>::iterator wit=m_writerList.begin();
			wit!=m_writerList.end();++wit)
	{
		if((*wit)->m_guid == p_endpoint->m_guid) //Found it
		{
			m_writerList.erase(wit);
			found = true;
			break;
		}
	}
	if(!found)
	{
		for(std::vector<RTPSReader*>::iterator rit=m_readerList.begin()
				;rit!=m_readerList.end();++rit)
		{
			if((*rit)->m_guid == p_endpoint->m_guid) //Found it
			{
				m_readerList.erase(rit);
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
	for(thit=m_threadListenList.begin();
			thit!=m_threadListenList.end();thit++)
	{
		if(type == 'W')
		{
			for(std::vector<RTPSWriter*>::iterator wit = (*thit)->m_assoc_writers.begin();
					wit!=(*thit)->m_assoc_writers.end();++wit)
			{
				if((*wit)->m_guid == p_endpoint->m_guid)
				{
					(*thit)->m_assoc_writers.erase(wit);
					if((*thit)->m_assoc_writers.empty() && (*thit)->m_assoc_readers.empty())
						m_threadListenList.erase(thit);

				}
			}
		}
		else if(type == 'R')
		{
			for(std::vector<RTPSReader*>::iterator rit = (*thit)->m_assoc_readers.begin();rit!=(*thit)->m_assoc_readers.end();++rit)
			{
				if((*rit)->m_guid == p_endpoint->m_guid)
				{
					(*thit)->m_assoc_readers.erase(rit);
					if((*thit)->m_assoc_readers.empty() && (*thit)->m_assoc_writers.empty())
						m_threadListenList.erase(thit);

				}
			}
		}
	}
	return true;
}


} /* namespace rtps */
} /* namespace eprosima */


