/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDPSimpleListeners.h
 *
 */

#ifndef EDPSIMPLELISTENER_H_
#define EDPSIMPLELISTENER_H_

#include "eprosimartps/pubsub/SubscriberListener.h"

#include "eprosimartps/writer/ReaderProxyData.h"
#include "eprosimartps/reader/WriterProxyData.h"

using namespace eprosima::pubsub;

namespace eprosima {
namespace rtps {

class EDPSimple;
/**
 * Class EDPSimplePUBReaderListener, used to define the behavior when a new WriterProxyData is received.
 * @ingroup DISCOVERYMODULE
 */
class EDPSimplePUBReaderListener:public SubscriberListener{
public:
	EDPSimplePUBReaderListener(EDPSimple* p):mp_SEDP(p){};
	virtual ~EDPSimplePUBReaderListener(){};
	void onNewDataMessage();
	EDPSimple* mp_SEDP;
	WriterProxyData m_writerProxyData;
	CDRMessage_t m_tempMsg;
};
/**
 * Class EDPSimpleSUBReaderListener, used to define the behavior when a new ReaderProxyData is received.
 * @ingroup DISCOVERYMODULE
 */
class EDPSimpleSUBReaderListener:public SubscriberListener{
public:
	EDPSimpleSUBReaderListener(EDPSimple* p):mp_SEDP(p){};
	virtual ~EDPSimpleSUBReaderListener(){};
	void onNewDataMessage();
	EDPSimple* mp_SEDP;
	ReaderProxyData m_readerProxyData;
	CDRMessage_t m_tempMsg;
};

/**
 * Class EDPSimpleListeners that contains two different Listeners for the Publications and Subscriptions Readers.
 * @ingroup DISCOVERYMODULE
 */
class EDPSimpleListeners {
public:
	EDPSimpleListeners(EDPSimple* p):
		m_pubReaderListener(p),
		m_subReaderListener(p){};
	virtual ~EDPSimpleListeners(){};
	EDPSimplePUBReaderListener m_pubReaderListener;
	EDPSimpleSUBReaderListener m_subReaderListener;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* EDPSIMPLELISTENER_H_ */
