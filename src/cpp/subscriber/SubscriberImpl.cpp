/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SubscriberImpl.cpp
 *
 */

#include "fastrtps/subscriber/SubscriberImpl.h"
#include "fastrtps/subscriber/Subscriber.h"
#include "fastrtps/TopicDataType.h"
#include "fastrtps/subscriber/SubscriberListener.h"
#include "fastrtps/rtps/reader/RTPSReader.h"
#include "fastrtps/rtps/reader/StatefulReader.h"

#include "fastrtps/rtps/RTPSDomain.h"
#include "fastrtps/rtps/participant/RTPSParticipant.h"

#include "fastrtps/utils/RTPSLog.h"

using namespace eprosima::fastrtps::rtps;


namespace eprosima {
namespace fastrtps {

static const char* const CLASS_NAME = "SubscriberImpl";

SubscriberImpl::SubscriberImpl(ParticipantImpl* p,TopicDataType* ptype,
		SubscriberAttributes& att,SubscriberListener* listen):
												mp_participant(p),
												mp_reader(nullptr),
												mp_type(ptype),
												m_att(att),
#pragma warning (disable : 4355 )
												m_history(this,ptype->m_typeSize,att.topic.historyQos,att.topic.resourceLimitsQos),
												mp_listener(listen),
#pragma warning (disable : 4355 )
												m_readerListener(this),
												 mp_userSubscriber(nullptr),
												mp_rtpsParticipant(nullptr)
{

}


SubscriberImpl::~SubscriberImpl()
{
	const char* const METHOD_NAME = "~SubscriberImpl";
	logInfo(SUBSCRIBER,this->getGuid().entityId << " in topic: "<<this->m_att.topic.topicName);
	RTPSDomain::removeRTPSReader(mp_reader);
	delete(this->mp_userSubscriber);
}


void SubscriberImpl::waitForUnreadMessage()
{
	if(m_history.getUnreadCount()==0)
	{
		while(1)
		{
			m_history.waitSemaphore();
			if(m_history.getUnreadCount()>0)
				break;
		}
	}
}



bool SubscriberImpl::readNextData(void* data,SampleInfo_t* info)
{
	return this->m_history.readNextData(data,info);
}

bool SubscriberImpl::takeNextData(void* data,SampleInfo_t* info) {
	return this->m_history.takeNextData(data,info);
}



const GUID_t& SubscriberImpl::getGuid(){
	return mp_reader->getGuid();
}



bool SubscriberImpl::updateAttributes(SubscriberAttributes& att)
{
	const char* const METHOD_NAME = "updateAttributes";
	bool updated = true;
	bool missing = false;
	if(att.unicastLocatorList.size() != this->m_att.unicastLocatorList.size() ||
			att.multicastLocatorList.size() != this->m_att.multicastLocatorList.size())
	{
		logWarning(RTPS_READER,"Locator Lists cannot be changed or updated in this version");
		updated &= false;
	}
	else
	{
		for(LocatorListIterator lit1 = this->m_att.unicastLocatorList.begin();
				lit1!=this->m_att.unicastLocatorList.end();++lit1)
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
				logWarning(RTPS_READER,"Locator: "<< *lit1 << " not present in new list");
				logWarning(RTPS_READER,"Locator Lists cannot be changed or updated in this version");
			}
		}
		for(LocatorListIterator lit1 = this->m_att.multicastLocatorList.begin();
				lit1!=this->m_att.multicastLocatorList.end();++lit1)
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
				logWarning(RTPS_READER,"Locator: "<< *lit1<< " not present in new list");
				logWarning(RTPS_READER,"Locator Lists cannot be changed or updated in this version");
			}
		}
	}

	//TOPIC ATTRIBUTES
	if(this->m_att.topic != att.topic)
	{
		logWarning(RTPS_READER,"Topic Attributes cannot be updated");
		updated &= false;
	}
	//QOS:
	//CHECK IF THE QOS CAN BE SET
	if(!this->m_att.qos.canQosBeUpdated(att.qos))
	{
		updated &=false;
	}
	if(updated)
	{
		this->m_att.expectsInlineQos = att.expectsInlineQos;
		if(this->m_att.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
		{
			//UPDATE TIMES:
			StatefulReader* sfr = (StatefulReader*)mp_reader;
			sfr->updateTimes(att.times);
		}
		this->m_att.qos.setQos(att.qos,false);
		//NOTIFY THE BUILTIN PROTOCOLS THAT THE READER HAS CHANGED
		mp_rtpsParticipant->updateReader(this->mp_reader,m_att.qos);
	}
	return updated;
}

void SubscriberImpl::SubscriberReaderListener::onNewCacheChangeAdded(RTPSReader* reader,const CacheChange_t* const change)
{
	if(mp_subscriberImpl->mp_listener != nullptr)
	{
		//cout << "FIRST BYTE: "<< (int)change->serializedPayload.data[0] << endl;
		mp_subscriberImpl->mp_listener->onNewDataMessage(mp_subscriberImpl->mp_userSubscriber);
	}
}

void SubscriberImpl::SubscriberReaderListener::onReaderMatched(RTPSReader* reader,MatchingInfo info)
{
	if (this->mp_subscriberImpl->mp_listener != nullptr)
	{
		mp_subscriberImpl->mp_listener->onSubscriptionMatched(mp_subscriberImpl->mp_userSubscriber,info);
	}
}

} /* namespace fastrtps */
} /* namespace eprosima */
