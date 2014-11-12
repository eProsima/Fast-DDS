/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Publisher.cpp
 *
 */

#include "eprosimartps/pubsub/Publisher.h"
#include "eprosimartps/pubsub/TopicDataType.h"
#include "eprosimartps/pubsub/PublisherListener.h"
#include "eprosimartps/writer/RTPSWriter.h"
#include "eprosimartps/writer/ReaderLocator.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/writer/ReaderProxy.h"

#include "eprosimartps/utils/RTPSLog.h"


#include "eprosimartps/Participant.h"

namespace eprosima {
namespace pubsub {

PublisherImpl::PublisherImpl(ParticipantImpl* p,RTPSWriter* Win,TopicDataType*pdatatype,PublisherAttributes& att):
								mp_Writer(Win),
								mp_type(pdatatype),
								m_attributes(att),
								mp_participant(p)
{

}

PublisherImpl::~PublisherImpl() {

	pDebugInfo("Publisher destructor"<<endl;);
}

bool PublisherImpl::write(void* Data) {

	pInfo("Writing new data"<<endl)
							return mp_Writer->add_new_change(ALIVE,Data);
}

bool PublisherImpl::dispose(void* Data) {

	pInfo("Disposing of Data"<<endl)
							return mp_Writer->add_new_change(NOT_ALIVE_DISPOSED,Data);
}


bool PublisherImpl::unregister(void* Data) {
	//Convert data to serialized Payload
	pInfo("Unregistering of Data"<<endl)
							return mp_Writer->add_new_change(NOT_ALIVE_UNREGISTERED,Data);
}

bool PublisherImpl::dispose_and_unregister(void* Data) {
	//Convert data to serialized Payload
	pInfo("Disposing and Unregistering Data"<<endl)
							return mp_Writer->add_new_change(NOT_ALIVE_DISPOSED_UNREGISTERED,Data);
}


bool PublisherImpl::removeMinSeqChange()
{
	return mp_Writer->removeMinSeqCacheChange();
}

bool PublisherImpl::removeAllChange(size_t* removed)
{
	return mp_Writer->removeAllCacheChange(removed);
}

size_t PublisherImpl::getHistoryElementsNumber()
{
	return mp_Writer->getHistoryCacheSize();
}

size_t PublisherImpl::getMatchedSubscribers()
{
	return mp_Writer->getMatchedSubscribers();
}

bool PublisherImpl::assignListener(PublisherListener* listen_in)
{
	mp_Writer->setListener(listen_in);
	return true;
}

const GUID_t& PublisherImpl::getGuid()
{
	return mp_Writer->getGuid();
}

bool PublisherImpl::updateAttributes(PublisherAttributes& att)
{
	bool updated = true;
	bool missing = false;
	if(this->mp_Writer->getStateType() == STATEFUL)
	{
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
					pWarning("Locator Lists cannot be changed or updated in this version"<<endl);
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
					pWarning("Locator Lists cannot be changed or updated in this version"<<endl);
				}
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
	if(!this->mp_Writer->canQosBeUpdated(att.qos))
	{
		updated &=false;
	}
	if(updated)
	{
		if(this->mp_Writer->getStateType() == STATEFUL)
		{
			//UPDATE TIMES:
			StatefulWriter* sfw = (StatefulWriter*)mp_Writer;
			sfw->updateTimes(att.times);
		}
		this->mp_Writer->setQos(att.qos,false);
		this->m_attributes = att;
		//NOTIFY THE BUILTIN PROTOCOLS THAT THE READER HAS CHANGED
		mp_participant->getBuiltinProtocols()->updateLocalWriter(this->mp_Writer);
	}


	return updated;
}


} /* namespace pubsub */
} /* namespace eprosima */


