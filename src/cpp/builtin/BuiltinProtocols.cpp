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

#include "eprosimartps/builtin/discovery/participant/PDPSimple.h"
#include "eprosimartps/builtin/discovery/endpoint/EDP.h"

#include "eprosimartps/builtin/liveliness/WLP.h"

#include "eprosimartps/reader/StatelessReader.h"

#include "eprosimartps/dds/DomainParticipant.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/IPFinder.h"

namespace eprosima {
namespace rtps {

BuiltinProtocols::BuiltinProtocols(ParticipantImpl* part):
														mp_participant(part),
														mp_PDP(NULL),
														mp_WLP(NULL),
														m_SPDP_WELL_KNOWN_MULTICAST_PORT(7400),
														m_SPDP_WELL_KNOWN_UNICAST_PORT(7410)
{
	// TODO Auto-generated constructor stub

}

BuiltinProtocols::~BuiltinProtocols() {
	// TODO Auto-generated destructor stub
}


bool BuiltinProtocols::initBuiltinProtocols(const BuiltinAttributes& attributes,uint32_t participantID)
{
	m_attributes = attributes;

	DomainParticipantImpl* dp = DomainParticipantImpl::getInstance();
	m_SPDP_WELL_KNOWN_MULTICAST_PORT = dp->getMulticastPort(m_attributes.domainId);

	m_SPDP_WELL_KNOWN_UNICAST_PORT =  dp->getUnicastPort(m_attributes.domainId,participantID);

	//cout << "PORTS: " << m_SPDP_WELL_KNOWN_MULTICAST_PORT << " "<< m_SPDP_WELL_KNOWN_UNICAST_PORT << endl;

	Locator_t multiLocator;
	multiLocator.kind = LOCATOR_KIND_UDPv4;
	multiLocator.port = m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	multiLocator.set_IP4_address(239,255,0,1);
	m_metatrafficMulticastLocatorList.push_back(multiLocator);
	LocatorList_t locators;
	IPFinder::getIPAddress(&locators);
	for(std::vector<Locator_t>::iterator it=locators.begin();it!=locators.end();++it)
	{
		it->port = m_SPDP_WELL_KNOWN_UNICAST_PORT;
		m_metatrafficUnicastLocatorList.push_back(*it);
	}

	if(m_attributes.use_SIMPLE_ParticipantDiscoveryProtocol)
	{
		mp_PDP = new PDPSimple(this);
		mp_PDP->initPDP(mp_participant,participantID);
	}
	if(m_attributes.use_WriterLivelinessProtocol)
	{
		mp_WLP = new WLP(this->mp_participant);
		mp_WLP->initWL(this);
	}
	if(m_attributes.use_SIMPLE_ParticipantDiscoveryProtocol)
	{
		mp_PDP->announceParticipantState(true);
		mp_PDP->resetParticipantAnnouncement();
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

void BuiltinProtocols::announceParticipantState()
{
	mp_PDP->announceParticipantState(false);
}

void BuiltinProtocols::stopParticipantAnnouncement()
{
	mp_PDP->stopParticipantAnnouncement();
}

void BuiltinProtocols::resetParticipantAnnouncement()
{
	mp_PDP->resetParticipantAnnouncement();
}

} /* namespace rtps */
} /* namespace eprosima */
