/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SEDPListeners.h
 *
*/

#ifndef SEDPLISTENERS_H_
#define SEDPLISTENERS_H_

#include "eprosimartps/dds/SubscriberListener.h"

//#include "eprosimartps/qos/QosList.h"
//#include "eprosimartps/discovery/data/DiscoveredData.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

class SEDPListeners;
class DiscoveredWriterData;
class DiscoveredReaderData;
class SimpleEDP;

/**
 * SEDP Publications Listener, used to define the behavior when a new DWriterData is received.
 */
class SEDPPubListener: public SubscriberListener {
public:
	SEDPPubListener(SEDPListeners* listeners,SimpleEDP* sedp):mp_listeners(listeners),mp_SEDP(sedp){};
	virtual ~SEDPPubListener(){};
	void onNewDataMessage();
	SEDPListeners* mp_listeners;
	SimpleEDP* mp_SEDP;

};
/**
 * SEDP Subscription Listener, used to define the behavior when a new DReaderData is received.
 */
class SEDPSubListener: public SubscriberListener {
public:
	SEDPSubListener(SEDPListeners* listeners,SimpleEDP* sedp):mp_listeners(listeners),mp_SEDP(sedp){};
	virtual ~SEDPSubListener(){};
	void onNewDataMessage();
	SEDPListeners* mp_listeners;
	SimpleEDP* mp_SEDP;

};

/**
 * Class SEDPListeners that contains two different Listeners for the Publications and Subscriptions Readers.
 */
class SEDPListeners
{
public:
	SEDPListeners(SimpleEDP* edp):m_PubListener(this,edp),m_SubListener(this,edp){};
	virtual ~SEDPListeners(){};
	SEDPPubListener m_PubListener;
	SEDPSubListener m_SubListener;

};


} /* namespace rtps */
} /* namespace eprosima */

#endif /* SEDPLISTENERS_H_ */
