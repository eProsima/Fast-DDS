/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file BuiltinProtocols.h
 *
 */

#ifndef BUILTINPROTOCOLS_H_
#define BUILTINPROTOCOLS_H_

namespace eprosima {
namespace rtps {

class PDPSimple;
class WriterLiveliness;
class ParticipantImpl;

class BuiltinProtocols {
public:
	BuiltinProtocols(ParticipantImpl* p_part);
	virtual ~BuiltinProtocols();

	bool initBuiltinProtocols(const BuiltinAttributes& attributes, uint32_t participantID);
	bool updateMetatrafficLocators();

	BuiltinAttributes m_attributes;
	ParticipantImpl* mp_participant;
	PDPSimple* mp_PDP;
	WriterLiveliness* mp_WL;
	uint32_t m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	uint32_t m_SPDP_WELL_KNOWN_UNICAST_PORT;
	LocatorList_t m_metatrafficMulticastLocatorList;
	LocatorList_t m_metatrafficUnicastLocatorList;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* BUILTINPROTOCOLS_H_ */
