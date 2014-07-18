/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDPSimpleListener.h
 *
 */

#ifndef EDPSIMPLELISTENER_H_
#define EDPSIMPLELISTENER_H_

#include "eprosimartps/dds/SubscriberListener.h"

#include "eprosimartps/writer/ReaderProxyData.h"
#include "eprosimartps/reader/WriterProxyData.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

class EDPSimple;
/**
 * SEDP Publications Listener, used to define the behavior when a new DWriterData is received.
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
 * SEDP Subscription Listener, used to define the behavior when a new DReaderData is received.
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
 * Class SEDPListeners that contains two different Listeners for the Publications and Subscriptions Readers.
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
