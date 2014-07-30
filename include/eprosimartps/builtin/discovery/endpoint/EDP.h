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


/**
 * Class EDP, base class for Endpoint Discovery Protocols. It contains generic methods used by the two EDP implemented (EDPSimple and EDPStatic), as well as abstract methods
 * definitions required by the specific implementations.
 * @ingroup DISCOVERYMODULE
 */
class EDP {
public:
	EDP(PDPSimple* p,ParticipantImpl* part);
	virtual ~EDP();

	/**
	 * Abstract method to initialize the EDP.
	 * @param attributes DiscoveryAttributes structure.
	 * @return True if correct.
	 */
	virtual bool initEDP(BuiltinAttributes& attributes)=0;
	/**
	 * Abstract method that assigns remote endpoints when a new participantProxyData is discovered.
	 * @param pdata
	 */
	virtual void assignRemoteEndpoints(ParticipantProxyData* pdata)=0;

	virtual void removeRemoteEndpoints(ParticipantProxyData* pdata){};

	/**
	 * Abstract method that removes a local Reader from the discovery method
	 * @param R Pointer to the Reader to remove.
	 * @return True if correctly removed.
	 */
	virtual bool removeLocalReader(RTPSReader* R)=0;
	/**
	 * Abstract method that removes a local Writer from the discovery method
	 * @param W Pointer to the Writer to remove.
	 * @return True if correctly removed.
	 */
	virtual bool removeLocalWriter(RTPSWriter*W)=0;

	/**
	 * After a new local ReaderProxyData has been created some processing is needed (depends on the implementation).
	 * @param rdata Pointer to the ReaderProxyData object.
	 * @return True if correct.
	 */
	virtual bool processLocalReaderProxyData(ReaderProxyData* rdata)= 0;
	/**
	 * After a new local WriterProxyData has been created some processing is needed (depends on the implementation).
	 * @param wdata Pointer to the Writer ProxyData object.
	 * @return True if correct.
	 */
	virtual bool processLocalWriterProxyData(WriterProxyData* wdata)= 0;

	/**
	 * Create a new ReaderPD for a local Reader.
	 * @param R Pointer to the RTPSReader.
	 * @return True if correct.
	 */
	bool newLocalReaderProxyData(RTPSReader* R);
	/**
	 * Create a new ReaderPD for a local Writer.
	 * @param W Pointer to the RTPSWriter.
	 * @return True if correct.
	 */
	bool newLocalWriterProxyData(RTPSWriter* W);
	/**
	 * Pair a WriterProxyData against all local Readers.
	 * @param wdata Pointer to the WPD object.
	 */
	void pairWriterProxy(WriterProxyData* wdata);
	/**
	 * Pair a ReaderProxyData against all local Writer.
	 * @param rdata Pointer to the RPD object.
	 */
	void pairReaderProxy(ReaderProxyData* rdata);
	/**
	 * Pair a local Reader against all possible WriterProxyData objects.
	 * @param R Pointer to the reader.
	 */
	void pairReader(RTPSReader* R);
	/**
	 * Pair a local writer against all possible ReaderProxyData objects.
	 * @param W Pointer to the writer.
	 */
	void pairWriter(RTPSWriter* W);
	/**
	 * Check the validity of a matching between a RTPSWriter and a ReaderProxyData object.
	 * @param W Pointer to the writer.
	 * @param rdata Pointer to the ReaderProxyData object.
	 * @return True if the two can be matched.
	 */
	bool validMatching(RTPSWriter* W,ReaderProxyData* rdata);
	/**
	 * Check the validity of a matching between a RTPSReader and a WriterProxyData object.
	 * @param R Pointer to the reader.
	 * @param wdata Pointer to the WriterProxyData object.
	 * @return True if the two can be matched.
	 */
	bool validMatching(RTPSReader* R,WriterProxyData* wdata);
	/**
	 * Remove a WriterProxyDataObject based on its GUID_t.
	 * @param writer Reference to the writer GUID.
	 * @return True if correct.
	 */
	bool removeWriterProxy(const GUID_t& writer);
	/**
	 * Remove a ReaderProxyDataObject based on its GUID_t.
	 * @param reader Reference to the reader GUID.
	 * @return True if correct.
	 */
	bool removeReaderProxy(const GUID_t& reader);
	/**
	 * Unpair a WriterProxyData object from all local readers.
	 * @param wdata Pointer to the WriterProxyData object.
	 * @return True if correct.
	 */
	bool unpairWriterProxy(WriterProxyData* wdata);
	/**
	 * Unpair a ReaderProxyData object from all local writers.
	 * @param rdata Pointer to the ReaderProxyData object.
	 * @return True if correct.
	 */
	bool unpairReaderProxy(ReaderProxyData* rdata);

	/**
	 * Method design to manage an updated ReaderProxyData object. (NOT YET IMPLEMENTED).
	 * @param rdata Pointer to the ReaderProxyData object.
	 * @return True if correct.
	 */
	bool updatedReaderProxy(ReaderProxyData* rdata);
	/**
	 * Method design to manage an updated WriterProxyData object. (NOT YET IMPLEMENTED).
	 * @param wdata Pointer to the WriterProxyData object.
	 * @return True if correct.
	 */
	bool updatedWriterProxy(WriterProxyData* wdata);


	//! Pointer to the PDPSimple object that contains the endpoint discovery protocol.
	PDPSimple* mp_PDP;
	//! Pointer to the participant.
	ParticipantImpl* mp_participant;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* EDP_H_ */
