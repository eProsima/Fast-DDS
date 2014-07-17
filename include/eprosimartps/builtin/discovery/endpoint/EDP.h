/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDP.h
 *
 */

#ifndef EDP_H_
#define EDP_H_

#include "eprosimartps/dds/attributes/ParticipantAttributes.h"
#include "eprosimartps/common/types/Guid.h"

namespace eprosima {
namespace rtps {

class PDPSimple;
class ParticipantProxyData;
class RTPSWriter;
class RTPSReader;
class ReaderProxyData;
class WriterProxyData;
class ParticipantImpl;

class EDP {
public:
	EDP(PDPSimple* p,ParticipantImpl* part);
	virtual ~EDP();

	/**
	 * Abstract method to initialize the EDP.
	 * @param attributes DiscoveryAttributes structure.
	 * @return True if correct.
	 */
	virtual bool initEDP(DiscoveryAttributes& attributes)=0;

	virtual void assignRemoteEndpoints(ParticipantProxyData* pdata)=0;

	bool newLocalReaderProxyData(RTPSReader*);
	bool newLocalWriterProxyData(RTPSWriter*);

	virtual bool removeLocalReader(RTPSReader*)=0;
	virtual bool removeLocalWriter(RTPSWriter*)=0;

	void pairWriterProxy(WriterProxyData* wdata);
	void pairReaderProxy(ReaderProxyData* rdata);

	bool validMatching(RTPSWriter* W,ReaderProxyData* rdata);
	bool validMatching(RTPSReader* R,WriterProxyData* wdata);

	bool unpairWriterProxy(const GUID_t& writer);
	bool unpairReaderProxy(const GUID_t& reader);

	bool updatedReaderProxy(ReaderProxyData* rdata);
	bool updatedWriterProxy(WriterProxyData* wdata);

	virtual bool processLocalReaderProxyData(ReaderProxyData* rdata)= 0;
	virtual bool processLocalWriterProxyData(WriterProxyData* rdata)= 0;

	PDPSimple* mp_PDP;
	ParticipantImpl* mp_participant;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* EDP_H_ */
