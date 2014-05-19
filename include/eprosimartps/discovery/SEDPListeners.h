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
 *  Created on: May 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef SEDPLISTENERS_H_
#define SEDPLISTENERS_H_
#include "eprosimartps/dds/SubscriberListener.h"
#include "eprosimartps/discovery/SimpleEDP.h"

#include "eprosimartps/discovery/data/DiscoveredData.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

class SEDPListeners;
class DiscoveredWriterData;
class DiscoveredReaderData;
class DiscoveredTopicData;


class SEDPPubListener: public SubscriberListener {
public:
	SEDPPubListener(SEDPListeners* listeners,SimpleEDP* sedp):mp_listeners(listeners),mp_SEDP(sedp){};
	virtual ~SEDPPubListener();
	void onNewDataMessage();
	bool processParameterList(ParameterList_t param,DiscoveredWriterData* wdata);
	SEDPListeners* mp_listeners;
		SimpleEDP* mp_SEDP;

};

class SEDPSubListener: public SubscriberListener {
public:
	SEDPSubListener(SEDPListeners* listeners,SimpleEDP* sedp):mp_listeners(listeners),mp_SEDP(sedp){};
	virtual ~SEDPSubListener();
	void onNewDataMessage();
	bool processParameterList(ParameterList_t param,DiscoveredReaderData* rdata);
	SEDPListeners* mp_listeners;
	SimpleEDP* mp_SEDP;

};

class SEDPTopListener: public SubscriberListener {
public:
	SEDPTopListener(SEDPListeners* listeners,SimpleEDP* sedp):mp_listeners(listeners),mp_SEDP(sedp){};
	virtual ~SEDPTopListener();
	void onNewDataMessage();
	bool processParameterList(ParameterList_t param,DiscoveredTopicData* tdata);
	SEDPListeners* mp_listeners;
		SimpleEDP* mp_SEDP;

};

class SEDPListeners
{
	SEDPListeners(SimpleEDP* edp):m_PubListener(this,edp),m_SubListener(this,edp),m_TopListener(this,edp){};
	~SEDPListeners(){};
	SEDPPubListener m_PubListener;
	SEDPSubListener m_SubListener;
	SEDPTopListener m_TopListener;
	bool findParticipant(GuidPrefix_t& guidP,DiscoveredParticipantData** pdata);
	bool processParameterList(CacheChange_t* change,DiscoveredData* ddata);
};


} /* namespace rtps */
} /* namespace eprosima */

#endif /* SEDPLISTENERS_H_ */
