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

#include "eprosimartps/pubsub/attributes/RTPSParticipantAttributes.h"
#include "eprosimartps/common/types/Guid.h"

namespace eprosima {
namespace rtps {

class PDPSimple;
class RTPSParticipantProxyData;
class RTPSWriter;
class RTPSReader;
class ReaderProxyData;
class WriterProxyData;
class RTPSParticipantImpl;


/**
 * Class EDP, base class for Endpoint Discovery Protocols. It contains generic methods used by the two EDP implemented (EDPSimple and EDPStatic), as well as abstract methods
 * definitions required by the specific implementations.
 * @ingroup DISCOVERYMODULE
 */
class EDP {
public:
	EDP(PDPSimple* p,RTPSParticipantImpl* part);
	virtual ~EDP();

	/**
	 * Abstract method to initialize the EDP.
	 * @param attributes DiscoveryAttributes structure.
	 * @return True if correct.
	 */
	virtual bool initEDP(BuiltinAttributes& attributes)=0;
	/**
	 * Abstract method that assigns remote endpoints when a new RTPSParticipantProxyData is discovered.
	 * @param pdata
	 */
	virtual void assignRemoteEndpoints(RTPSParticipantProxyData* pdata)=0;

	virtual void removeRemoteEndpoints(RTPSParticipantProxyData* pdata){};

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
	 * A previously created Reader has been updated
	 * @param R Pointer to the reader;
	 * @return True if correctly updated
	 */
	bool updatedLocalReader(RTPSReader* R);
	/**
	 * A previously created Writer has been updated
	 * @param W Pointer to the Writer
	 * @return True if correctly updated
	 */
	bool updatedLocalWriter(RTPSWriter* W);


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
	 * Try to pair/unpair a local Reader against all possible writerProxy Data.
	 * @param R Pointer to the Reader
	 * @return True
	 */
	bool pairingReader(RTPSReader* R);
	/**l
	 * Try to pair/unpair a local Writer against all possible readerProxy Data.
	 * @param W Pointer to the Writer
	 * @return True
	 */
	bool pairingWriter(RTPSWriter* W);
	/**
	 * Try to pair/unpair ReaderProxyData.
	 * @param rdata Pointer to the ReaderProxyData object.
	 * @return True.
	 */
	bool pairingReaderProxy(ReaderProxyData* rdata);
	/**
	 * Try to pair/unpair WriterProxyData.
	 * @param wdata Pointer to the WriterProxyData.
	 * @return True.
	 */
	bool pairingWriterProxy(WriterProxyData* wdata);

	//! Pointer to the PDPSimple object that contains the endpoint discovery protocol.
	PDPSimple* mp_PDP;
	//! Pointer to the RTPSParticipant.
	RTPSParticipantImpl* mp_RTPSParticipant;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* EDP_H_ */
