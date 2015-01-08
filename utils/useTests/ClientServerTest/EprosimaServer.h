/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EprosimaServer.h
 *
 */

#ifndef EPROSIMASERVER_H_
#define EPROSIMASERVER_H_

#include "ClientServerTypes.h"

#include "fastrtps/fastrtps_fwd.h"
#include "fastrtps/subscriber/SampleInfo.h"

#include "fastrtps/publisher/PublisherListener.h"
#include "fastrtps/subscriber/SubscriberListener.h"

using namespace eprosima;
using namespace fastrtps;

using namespace clientserver;

class EprosimaServer {
	friend class OperationListener;
	friend class ResultListener;
public:
	EprosimaServer();
	virtual ~EprosimaServer();
	bool init();
	void serve();
private:
	Subscriber* mp_operation_sub;
	Publisher* mp_result_pub;
	Participant* mp_participant;
	Result::RESULTTYPE calculate(Operation::OPERATIONTYPE type, int32_t num1,int32_t num2,int32_t* result);
	ResultDataType* mp_resultdatatype;
	OperationDataType* mp_operationdatatype;
	class OperationListener:public SubscriberListener
	{
	public:
		OperationListener(EprosimaServer* up):mp_up(up){}
		~OperationListener(){}
		EprosimaServer* mp_up;
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo info);
		void onNewDataMessage(Subscriber*sub);
		Operation m_operation;
		SampleInfo_t m_sampleInfo;
		Result m_result;
	}m_operationsListener;
	class ResultListener:public PublisherListener
	{
	public:
		ResultListener(EprosimaServer* up):mp_up(up){}
		~ResultListener(){}
		EprosimaServer* mp_up;
		void onPublicationMatched(Publisher* pub,MatchingInfo info);
	}m_resultsListener;
};

#endif /* EPROSIMASERVER_H_ */
