/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file BuiltinProtocols.cpp
 *
 */

#include "fastrtps/rtps/builtin/BuiltinProtocols.h"
#include "fastrtps/rtps/common/Locator.h"

#include "fastrtps/rtps/builtin/discovery/participant/PDPSimple.h"
#include "fastrtps/rtps/builtin/discovery/endpoint/EDP.h"

#include "fastrtps/rtps/builtin/liveliness/WLP.h"

#include "fastrtps/rtps/participant/RTPSParticipantImpl.h"


#include "fastrtps/utils/RTPSLog.h"
#include "fastrtps/utils/IPFinder.h"



using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "BuiltinProtocols";

BuiltinProtocols::BuiltinProtocols():
				mp_participantImpl(nullptr),
				mp_PDP(nullptr),
				mp_WLP(nullptr),
				m_SPDP_WELL_KNOWN_MULTICAST_PORT(7400),
				m_SPDP_WELL_KNOWN_UNICAST_PORT(7410)
{
	// TODO Auto-generated constructor stub
	m_useMandatory = false;
}

BuiltinProtocols::~BuiltinProtocols() {
	// TODO Auto-generated destructor stub
	if(mp_PDP!=nullptr)
		delete(mp_PDP);
	if(mp_WLP!=nullptr)
		delete(mp_WLP);
}


bool BuiltinProtocols::initBuiltinProtocols(RTPSParticipantImpl* p_part, BuiltinAttributes& attributes)
{
	//const char* const METHOD_NAME = "initBuiltinProtocols";
	mp_participantImpl = p_part;
	m_att = attributes;

	m_SPDP_WELL_KNOWN_MULTICAST_PORT = mp_participantImpl->getAttributes().port.getMulticastPort(m_att.domainId);

	m_SPDP_WELL_KNOWN_UNICAST_PORT =
			mp_participantImpl->getAttributes().port.getUnicastPort(m_att.domainId,mp_participantImpl->getAttributes().participantID);

	this->m_mandatoryMulticastLocator.kind = LOCATOR_KIND_UDPv4;
	m_mandatoryMulticastLocator.port = m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	m_mandatoryMulticastLocator.set_IP4_address(239,255,0,1);
	if(m_att.metatrafficMulticastLocatorList.empty())
	{
		m_metatrafficMulticastLocatorList.push_back(m_mandatoryMulticastLocator);
	}
	else
	{
		m_useMandatory = true;
		for(std::vector<Locator_t>::iterator it = m_att.metatrafficMulticastLocatorList.begin();
				it!=m_att.metatrafficMulticastLocatorList.end();++it)
		{
			m_metatrafficMulticastLocatorList.push_back(*it);
		}
	}
	if(m_att.metatrafficUnicastLocatorList.empty())
	{
//		LocatorList_t locators;
//		IPFinder::getIP4Address(&locators);
//		for(std::vector<Locator_t>::iterator it=locators.begin();it!=locators.end();++it)
//		{
		Locator_t loc;
			loc.port = m_SPDP_WELL_KNOWN_UNICAST_PORT;
			loc.kind = LOCATOR_KIND_UDPv4;
			m_metatrafficUnicastLocatorList.push_back(loc);
		//}
	}
	else
	{
		for(std::vector<Locator_t>::iterator it = m_att.metatrafficUnicastLocatorList.begin();
				it!=m_att.metatrafficUnicastLocatorList.end();++it)
		{
			m_metatrafficUnicastLocatorList.push_back(*it);
		}
	}
	if(m_att.use_SIMPLE_RTPSParticipantDiscoveryProtocol)
	{
		mp_PDP = new PDPSimple(this);
		mp_PDP->initPDP(mp_participantImpl);
		if(m_att.use_WriterLivelinessProtocol)
		{
			mp_WLP = new WLP(this);
			mp_WLP->initWL(mp_participantImpl);
		}
		mp_PDP->announceParticipantState(true);
		mp_PDP->resetParticipantAnnouncement();
	}

	return true;
}

bool BuiltinProtocols::updateMetatrafficLocators(LocatorList_t& loclist)
{
	m_metatrafficUnicastLocatorList = loclist;
	return true;
}

bool BuiltinProtocols::addLocalWriter(RTPSWriter* w,fastrtps::TopicAttributes& topicAtt,fastrtps::WriterQos& wqos)
{
	bool ok = false;
	if(mp_PDP!=nullptr)
	{
		ok |= mp_PDP->getEDP()->newLocalWriterProxyData(w,topicAtt,wqos);
	}
	if(mp_WLP !=nullptr)
	{
		ok|= mp_WLP->addLocalWriter(w,wqos);
	}
	return ok;
}

bool BuiltinProtocols::addLocalReader(RTPSReader* R,fastrtps::TopicAttributes& topicAtt, fastrtps::ReaderQos& rqos)
{
	bool ok = false;
	if(mp_PDP!=nullptr)
	{
		ok |= mp_PDP->getEDP()->newLocalReaderProxyData(R,topicAtt, rqos);
	}
	return ok;
}

bool BuiltinProtocols::updateLocalWriter(RTPSWriter* W,WriterQos& wqos)
{
	bool ok = false;
	if(mp_PDP!=nullptr && mp_PDP->getEDP()!=nullptr)
	{
		ok |= mp_PDP->getEDP()->updatedLocalWriter(W,wqos);
	}
	if(mp_WLP!=nullptr)
	{
		ok |= mp_WLP->updateLocalWriter(W,wqos);
	}
	return ok;

	return true;
}

bool BuiltinProtocols::updateLocalReader(RTPSReader* R,ReaderQos& rqos)
{
	bool ok = false;
	if(mp_PDP!=nullptr && mp_PDP->getEDP()!=nullptr)
	{
		ok |= mp_PDP->getEDP()->updatedLocalReader(R,rqos);
	}
	return ok;
}

bool BuiltinProtocols::removeLocalWriter(RTPSWriter* W)
{
	bool ok = false;
	if(mp_WLP !=nullptr)
	{
		ok|= mp_WLP->removeLocalWriter(W);
	}
	if(mp_PDP!=nullptr && mp_PDP->getEDP() != nullptr)
	{
		ok|= mp_PDP->getEDP()->removeLocalWriter(W);
	}
	return ok;
}

bool BuiltinProtocols::removeLocalReader(RTPSReader* R)
{
	bool ok = false;
	if(mp_PDP!=nullptr && mp_PDP->getEDP() != nullptr)
	{
		ok|= mp_PDP->getEDP()->removeLocalReader(R);
	}
	return ok;
}

void BuiltinProtocols::announceRTPSParticipantState()
{
	mp_PDP->announceParticipantState(false);
}

void BuiltinProtocols::stopRTPSParticipantAnnouncement()
{
	mp_PDP->stopParticipantAnnouncement();
}

void BuiltinProtocols::resetRTPSParticipantAnnouncement()
{
	mp_PDP->resetParticipantAnnouncement();
}

}
} /* namespace rtps */
} /* namespace eprosima */
