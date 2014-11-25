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

#include "fastrtps/rtps/reader/ReaderListener.h"

#include "fastrtps/rtps/builtin/data/ReaderProxyData.h"
#include "fastrtps/rtps/builtin/data/WriterProxyData.h"



namespace eprosima {
namespace fastrtps{
namespace rtps {

class EDPSimple;
class RTPSReader;
class CacheChange_t;

/**
 * Class EDPSimplePUBReaderListener, used to define the behavior when a new WriterProxyData is received.
 * @ingroup DISCOVERYMODULE
 */
class EDPSimplePUBListener:public ReaderListener{
public:
	EDPSimplePUBListener(EDPSimple* p):mp_SEDP(p){free(aux_msg.buffer);};
	virtual ~EDPSimplePUBListener(){};
	void onNewCacheChangeAdded(RTPSReader* reader, CacheChange_t* change);
	bool computeKey(CacheChange_t* change);
	EDPSimple* mp_SEDP;
	WriterProxyData m_writerProxyData;
	CDRMessage_t m_tempMsg;
	CDRMessage_t aux_msg;
};
/**
 * Class EDPSimpleSUBReaderListener, used to define the behavior when a new ReaderProxyData is received.
 * @ingroup DISCOVERYMODULE
 */
class EDPSimpleSUBListener:public ReaderListener{
public:
	EDPSimpleSUBListener(EDPSimple* p):mp_SEDP(p){free(aux_msg.buffer);};
	virtual ~EDPSimpleSUBListener(){};
	void onNewCacheChangeAdded(RTPSReader* reader, CacheChange_t* change);
	bool computeKey(CacheChange_t* change);
	EDPSimple* mp_SEDP;
	ReaderProxyData m_readerProxyData;
	CDRMessage_t m_tempMsg;
	CDRMessage_t aux_msg;
};

} /* namespace rtps */
}
} /* namespace eprosima */

#endif /* EDPSIMPLELISTENER_H_ */
