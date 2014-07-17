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

	void pairWriterProxy(WriterProxyData* wdata);
	void pairReaderProxy(ReaderProxyData* rdata);

	bool validMatching(RTPSWriter* W,ReaderProxyData* rdata);
	bool validMatching(RTPSReader* R,WriterProxyData* wdata);

	bool unpairWriterProxy(WriterProxyData* wdata);
	bool unpairReaderProxy(ReaderProxyData* rdata);

	virtual bool processLocalReaderProxyData(ReaderProxyData* rdata)= 0;
	virtual bool processLocalWriterProxyData(WriterProxyData* rdata)= 0;



private:


	PDPSimple* mp_PDP;
	ParticipantImpl* mp_participant;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* EDP_H_ */
