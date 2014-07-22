/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDPStatic.h
 *
 */

#ifndef EDPSTATIC_H_
#define EDPSTATIC_H_


#include "eprosimartps/builtin/discovery/endpoint/EDP.h"
#include "eprosimartps/builtin/discovery/endpoint/EDPStaticXML.h"

namespace eprosima {
namespace rtps {

class EDPStaticProperty
{
public:
	EDPStaticProperty():userId(0){};
	~EDPStaticProperty(){};
	std::string str1;
	std::string type;
	std::string status;
	std::string idstr;
	std::string userIDstr;
	uint16_t userId;
	EntityId_t entityId;
	static std::pair<std::string,std::string> toProperty(std::string type,std::string status,uint16_t id,const EntityId_t& ent);
	bool fromProperty(std::pair<std::string,std::string> property);
};


class EDPStatic : public EDP {
public:
	EDPStatic(PDPSimple* p,ParticipantImpl* part);
	virtual ~EDPStatic();
	/**
	 * Abstract method to initialize the EDP.
	 * @param attributes DiscoveryAttributes structure.
	 * @return True if correct.
	 */
	bool initEDP(BuiltinAttributes& attributes);
	/**
	 * Abstract method that assigns remote endpoints when a new participantProxyData is discovered.
	 * @param pdata
	 */
	void assignRemoteEndpoints(ParticipantProxyData* pdata);
	/**
	 * Abstract method that removes a local Reader from the discovery method
	 * @param R Pointer to the Reader to remove.
	 * @return True if correctly removed.
	 */
	bool removeLocalReader(RTPSReader* R);
	/**
	 * Abstract method that removes a local Writer from the discovery method
	 * @param W Pointer to the Writer to remove.
	 * @return True if correctly removed.
	 */
	bool removeLocalWriter(RTPSWriter*W);

	/**
	 * After a new local ReaderProxyData has been created some processing is needed (depends on the implementation).
	 * @param rdata Pointer to the ReaderProxyData object.
	 * @return True if correct.
	 */
	bool processLocalReaderProxyData(ReaderProxyData* rdata);
	/**
	 * After a new local WriterProxyData has been created some processing is needed (depends on the implementation).
	 * @param wdata Pointer to the Writer ProxyData object.
	 * @return True if correct.
	 */
	bool processLocalWriterProxyData(WriterProxyData* wdata);


	bool newRemoteWriter(ParticipantProxyData*pdata,uint16_t userId,EntityId_t& entId);
	bool newRemoteReader(ParticipantProxyData*pdata,uint16_t userId,EntityId_t& entId);

private:
	EDPStaticXML m_edpXML;
	BuiltinAttributes m_attributes;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* EDPSTATIC_H_ */
