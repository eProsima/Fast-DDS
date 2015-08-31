/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDPSimpleListeners.h
 *
 */

#ifndef EDPSIMPLELISTENER_H_
#define EDPSIMPLELISTENER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include "../../../reader/ReaderListener.h"

#include "../../data/ReaderProxyData.h"
#include "../../data/WriterProxyData.h"



namespace eprosima {
namespace fastrtps{
namespace rtps {

class EDPSimple;
class RTPSReader;
struct CacheChange_t;

/**
 * Class EDPSimplePUBReaderListener, used to define the behavior when a new WriterProxyData is received.
 *@ingroup DISCOVERY_MODULE
 */
class EDPSimplePUBListener:public ReaderListener{
public:
	/**
	Constructor
	* @param p Pointer to the EDPSimple associated with this listener.
	*/
	EDPSimplePUBListener(EDPSimple* p):mp_SEDP(p){free(aux_msg.buffer);aux_msg.buffer = nullptr;};
	virtual ~EDPSimplePUBListener(){};
	/**
	* Virtual method, 
	* @param reader
	* @param change
	*/
	void onNewCacheChangeAdded(RTPSReader* reader,const CacheChange_t* const  change);
	/**
	* Compute the Key from a CacheChange_t
	* @param change Pointer to the change.
	*/
	bool computeKey(CacheChange_t* change);
	//!Pointer to the EDPSimple
	EDPSimple* mp_SEDP;
	//!WriterProxyData where to store the information
	WriterProxyData m_writerProxyData;
	//!Temporal message to deserialize the information.
	CDRMessage_t m_tempMsg;
	//!Auxiliay message.
	CDRMessage_t aux_msg;
};
/**
 * Class EDPSimpleSUBReaderListener, used to define the behavior when a new ReaderProxyData is received.
 *@ingroup DISCOVERY_MODULE
 */
class EDPSimpleSUBListener:public ReaderListener{
public:
	/**
	* @param p
	*/
	EDPSimpleSUBListener(EDPSimple* p):mp_SEDP(p){}

	virtual ~EDPSimpleSUBListener(){}
	/**
	* @param reader
	* @param change
	*/
	void onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change);
	/**
	* @param change
	*/
	bool computeKey(CacheChange_t* change);
	//!Pointer to the EDPSimple
	EDPSimple* mp_SEDP;
	//!ReaderProxyData object to store the recevied information.
	ReaderProxyData m_readerProxyData;
	//!Temporal message to deserialize the information.
	CDRMessage_t m_tempMsg;
	//!Auxiliay message.
	CDRMessage_t aux_msg;
};

} /* namespace rtps */
}
} /* namespace eprosima */
#endif
#endif /* EDPSIMPLELISTENER_H_ */
