/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Subscriber.cpp
 *
 */

#include "eprosimartps/dds/Subscriber.h"
#include "eprosimartps/reader/RTPSReader.h"
#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/reader/WriterProxy.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/dds/DDSTopicDataType.h"

#include "eprosimartps/dds/SubscriberListener.h"

#include "eprosimartps/Participant.h"



namespace eprosima {
namespace dds {



SubscriberImpl::SubscriberImpl(ParticipantImpl* p,RTPSReader* Rin,DDSTopicDataType* ptype):
												mp_Reader(Rin),
												mp_type(ptype),
												mp_participant(p)
{

}


SubscriberImpl::~SubscriberImpl()
{
	pDebugInfo("Subscriber destructor"<<endl;);
}


void SubscriberImpl::waitForUnreadMessage()
{
	if(!mp_Reader->isUnreadCacheChange())
	{
		while(1)
		{
			mp_Reader->m_semaphore.wait();
			if(mp_Reader->isUnreadCacheChange())
				break;
		}
	}
}

bool SubscriberImpl::assignListener(SubscriberListener* p_listener)
{
	mp_Reader->mp_listener = p_listener;
	return true;
}

bool SubscriberImpl::isHistoryFull()
{
	return mp_Reader->isHistoryFull();
}


size_t SubscriberImpl::getHistoryElementsNumber()
{
	return mp_Reader->getHistoryCacheSize();
}

size_t SubscriberImpl::getMatchedPublishers()
{return mp_Reader->getMatchedPublishers();};


bool SubscriberImpl::readNextData(void* data,SampleInfo_t* info)
{
	return this->mp_Reader->readNextCacheChange(data,info);
}

bool SubscriberImpl::takeNextData(void* data,SampleInfo_t* info) {
	return this->mp_Reader->takeNextCacheChange(data,info);
}



const GUID_t& SubscriberImpl::getGuid(){
	return mp_Reader->getGuid();
}



bool SubscriberImpl::updateAttributes(SubscriberAttributes& att)
{
	bool updated = true;
	bool missing = false;
	if(att.unicastLocatorList.size() != this->m_attributes.unicastLocatorList.size() ||
			att.multicastLocatorList.size() != this->m_attributes.multicastLocatorList.size())
	{
		pWarning("Locator Lists cannot be changed or updated in this version"<<endl);
		updated &= false;
	}
	else
	{
		for(LocatorListIterator lit1 = this->m_attributes.unicastLocatorList.begin();
				lit1!=this->m_attributes.unicastLocatorList.end();++lit1)
		{
			missing = true;
			for(LocatorListIterator lit2 = att.unicastLocatorList.begin();
					lit2!= att.unicastLocatorList.end();++lit2)
			{
				if(*lit1 == *lit2)
				{
					missing = false;
					break;
				}
			}
			if(missing)
			{
				pWarning("Locator: "<< *lit1 << " not present in new list"<<endl);
			}
		}
		for(LocatorListIterator lit1 = this->m_attributes.multicastLocatorList.begin();
				lit1!=this->m_attributes.multicastLocatorList.end();++lit1)
		{
			missing = true;
			for(LocatorListIterator lit2 = att.multicastLocatorList.begin();
					lit2!= att.multicastLocatorList.end();++lit2)
			{
				if(*lit1 == *lit2)
				{
					missing = false;
					break;
				}
			}
			if(missing)
			{
				pWarning("Locator: "<< *lit1<< " not present in new list"<<endl);
			}
		}
	}

	//TOPIC ATTRIBUTES
	if(this->m_attributes.topic != att.topic)
	{
		pWarning("Topic Attributes cannot be updated"<<endl;);
		updated &= false;
	}
	//QOS:
	//CHECK IF THE QOS CAN BE SET
	if(!this->mp_Reader->canQosBeUpdated(att.qos))
	{
		updated &=false;
	}
	if(updated)
	{
		this->mp_Reader->setExpectsInlineQos(att.expectsInlineQos);
		if(this->mp_Reader->getStateType() == STATEFUL)
		{
			//UPDATE TIMES:
			StatefulReader* sfr = (StatefulReader*)mp_Reader;
			sfr->updateTimes(att.times);
		}
		this->mp_Reader->setQos(att.qos,false);
		this->m_attributes = att;
		//NOTIFY THE BUILTIN PROTOCOLS THAT THE READER HAS CHANGED
		mp_participant->getBuiltinProtocols()->updateLocalReader(this->mp_Reader);
	}


	return updated;
}


} /* namespace dds */
} /* namespace eprosima */


