/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file BuiltinProtocols.cpp
 *
 */

#include "eprosimartps/builtin/BuiltinProtocols.h"
#include "eprosimartps/common/types/Locator.h"

#include "eprosimartps/builtin/discovery/RTPSParticipant/PDPSimple.h"
#include "eprosimartps/builtin/discovery/endpoint/EDP.h"

#include "eprosimartps/builtin/liveliness/WLP.h"

#include "eprosimartps/reader/StatelessReader.h"

#include "eprosimartps/pubsub/RTPSDomain.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/IPFinder.h"

namespace eprosima {
namespace rtps {

BuiltinProtocols::BuiltinProtocols(RTPSParticipantImpl* part):
														mp_RTPSParticipant(part),
														mp_PDP(NULL),
														mp_WLP(NULL),
														m_SPDP_WELL_KNOWN_MULTICAST_PORT(7400),
														m_SPDP_WELL_KNOWN_UNICAST_PORT(7410)
{
	// TODO Auto-generated constructor stub
	m_useMandatory = false;
}

BuiltinProtocols::~BuiltinProtocols() {
	// TODO Auto-generated destructor stub
}


bool BuiltinProtocols::initBuiltinProtocols(const BuiltinAttributes& attributes,uint32_t RTPSParticipantID)
{
	m_attributes = attributes;

	RTPSDomainImpl* dp = RTPSDomainImpl::getInstance();
	m_SPDP_WELL_KNOWN_MULTICAST_PORT = dp->getMulticastPort(m_attributes.domainId);

	m_SPDP_WELL_KNOWN_UNICAST_PORT =  dp->getUnicastPort(m_attributes.domainId,RTPSParticipantID);

	//cout << "PORTS: " << m_SPDP_WELL_KNOWN_MULTICAST_PORT << " "<< m_SPDP_WELL_KNOWN_UNICAST_PORT << endl;

	this->m_mandatoryMulticastLocator.kind = LOCATOR_KIND_UDPv4;
	m_mandatoryMulticastLocator.port = m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	m_mandatoryMulticastLocator.set_IP4_address(239,255,0,1);
	if(m_attributes.metatrafficMulticastLocatorList.empty())
	{
		m_metatrafficMulticastLocatorList.push_back(m_mandatoryMulticastLocator);
	}
	else
	{
		m_useMandatory = true;
		for(std::vector<Locator_t>::iterator it = m_attributes.metatrafficMulticastLocatorList.begin();
			it!=m_attributes.metatrafficMulticastLocatorList.end();++it)
		{
			m_metatrafficMulticastLocatorList.push_back(*it);
		}
	}
	if(m_attributes.metatrafficUnicastLocatorList.empty())
	{
		LocatorList_t locators;
		IPFinder::getIPAddress(&locators);
		for(std::vector<Locator_t>::iterator it=locators.begin();it!=locators.end();++it)
		{
			it->port = m_SPDP_WELL_KNOWN_UNICAST_PORT;
			m_metatrafficUnicastLocatorList.push_back(*it);
		}
	}
	else
	{
		for(std::vector<Locator_t>::iterator it = m_attributes.metatrafficUnicastLocatorList.begin();
		it!=m_attributes.metatrafficUnicastLocatorList.end();++it)
		{
			m_metatrafficUnicastLocatorList.push_back(*it);
		}
	}
	if(m_attributes.use_SIMPLE_RTPSParticipantDiscoveryProtocol)
	{
		mp_PDP = new PDPSimple(this);
		mp_PDP->initPDP(mp_RTPSParticipant,RTPSParticipantID);
	}
	if(m_attributes.use_WriterLivelinessProtocol)
	{
		mp_WLP = new WLP(this->mp_RTPSParticipant);
		mp_WLP->initWL(this);
	}
	if(m_attributes.use_SIMPLE_RTPSParticipantDiscoveryProtocol)
	{
		mp_PDP->announceRTPSParticipantState(true);
		mp_PDP->resetRTPSParticipantAnnouncement();
	}


	return false;
}

bool BuiltinProtocols::updateMetatrafficLocators(LocatorList_t& loclist)
{
	m_metatrafficUnicastLocatorList = loclist;
	return true;
}

bool BuiltinProtocols::addLocalWriter(RTPSWriter* W)
{
	bool ok = false;
	if(mp_PDP!=NULL)
	{
		ok |= mp_PDP->getEDP()->newLocalWriterProxyData(W);
	}
	if(mp_WLP !=NULL)
	{
		ok|= mp_WLP->addLocalWriter(W);
	}
	return ok;
}

bool BuiltinProtocols::addLocalReader(RTPSReader* R)
{
	bool ok = false;
	if(mp_PDP!=NULL)
	{
		ok |= mp_PDP->getEDP()->newLocalReaderProxyData(R);
	}
	return ok;
}

bool BuiltinProtocols::updateLocalWriter(RTPSWriter* W)
{
	bool ok = false;
	if(mp_PDP!=NULL)
	{
		ok |= mp_PDP->getEDP()->updatedLocalWriter(W);
	}
	if(mp_WLP!=NULL)
	{
		ok |= mp_WLP->updateLocalWriter(W);
	}
	return ok;

	return true;
}

bool BuiltinProtocols::updateLocalReader(RTPSReader* R)
{
	bool ok = false;
	if(mp_PDP!=NULL)
	{
		ok |= mp_PDP->getEDP()->updatedLocalReader(R);
	}
	return ok;
}

bool BuiltinProtocols::removeLocalWriter(RTPSWriter* W)
{
	bool ok = false;
	if(mp_PDP!=NULL)
	{
		ok|= mp_PDP->getEDP()->removeLocalWriter(W);
	}
	if(mp_WLP !=NULL)
	{
		ok|= mp_WLP->removeLocalWriter(W);
	}
	return ok;
}

bool BuiltinProtocols::removeLocalReader(RTPSReader* R)
{
	bool ok = false;
	if(mp_PDP!=NULL)
	{
		ok|= mp_PDP->getEDP()->removeLocalReader(R);
	}
	return ok;
}

void BuiltinProtocols::announceRTPSParticipantState()
{
	mp_PDP->announceRTPSParticipantState(false);
}

void BuiltinProtocols::stopRTPSParticipantAnnouncement()
{
	mp_PDP->stopRTPSParticipantAnnouncement();
}

void BuiltinProtocols::resetRTPSParticipantAnnouncement()
{
	mp_PDP->resetRTPSParticipantAnnouncement();
}

} /* namespace rtps */
} /* namespace eprosima */
