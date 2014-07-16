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

namespace eprosima {
namespace rtps {

BuiltinProtocols::BuiltinProtocols(ParticipantImpl* part):
		mp_participant(part),
		mp_PDP(NULL),
		mp_WL(NULL),
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

	if(m_attributes.discovery.use_SIMPLE_ParticipantDiscoveryProtocol)
	{
		mp_PDP = new PDPSimple(this);
		mp_PDP->initPDP(mp_participant,participantID);
	}



	return false;
}

bool BuiltinProtocols::updateMetatrafficLocators()
{
	m_metatrafficUnicastLocatorList = mp_PDP->mp_SPDPReader->unicastLocatorList;
	return true;
}


} /* namespace rtps */
} /* namespace eprosima */
