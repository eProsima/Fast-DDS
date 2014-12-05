/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDPSimpleListeners.h
 *
 */

#ifndef EDPSIMPLELISTENER_H_
#define EDPSIMPLELISTENER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include "fastrtps/rtps/reader/ReaderListener.h"

#include "fastrtps/rtps/builtin/data/ReaderProxyData.h"
#include "fastrtps/rtps/builtin/data/WriterProxyData.h"



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
	* @param p
	*/
	EDPSimplePUBListener(EDPSimple* p):mp_SEDP(p){free(aux_msg.buffer);aux_msg.buffer = nullptr;};
	virtual ~EDPSimplePUBListener(){};
	/**
	* @param reader
	* @param change
	*/
	void onNewCacheChangeAdded(RTPSReader* reader,const CacheChange_t* const  change);
	/**
	* @param change
	*/
	bool computeKey(CacheChange_t* change);
	//!
	EDPSimple* mp_SEDP;
	//!
	WriterProxyData m_writerProxyData;
	//!
	CDRMessage_t m_tempMsg;
	//!
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
	EDPSimpleSUBListener(EDPSimple* p):mp_SEDP(p){free(aux_msg.buffer);aux_msg.buffer = nullptr;};
	virtual ~EDPSimpleSUBListener(){};
	/**
	* @param reader
	* @param change
	*/
	void onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change);
	/**
	* @param change
	*/
	bool computeKey(CacheChange_t* change);
	//!
	EDPSimple* mp_SEDP;
	//!
	ReaderProxyData m_readerProxyData;
	//!
	CDRMessage_t m_tempMsg;
	//!
	CDRMessage_t aux_msg;
};

} /* namespace rtps */
}
} /* namespace eprosima */
#endif
#endif /* EDPSIMPLELISTENER_H_ */
