/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EprosimaClient.h
 *
 */

#ifndef EPROSIMACLIENT_H_
#define EPROSIMACLIENT_H_

#include "ClientServerTypes.h"

#include "fastrtps/fastrtps_fwd.h"
#include "fastrtps/subscriber/SampleInfo.h"

#include "fastrtps/publisher/PublisherListener.h"
#include "fastrtps/subscriber/SubscriberListener.h"

using namespace eprosima;
using namespace fastrtps;

using namespace clientserver;

class EprosimaClient {
public:
	EprosimaClient();
	virtual ~EprosimaClient();
	Publisher* mp_operation_pub;
	Subscriber* mp_result_sub;
	Participant* mp_participant;
	bool init();
	Result::RESULTTYPE calculate(Operation::OPERATIONTYPE type, int32_t num1,int32_t num2,int32_t* result);
	bool isReady();
private:
	Operation m_operation;
	Result m_result;
	SampleInfo_t m_sampleInfo;

	void resetResult();
	ResultDataType* mp_resultdatatype;
	OperationDataType* mp_operationdatatype;
	class OperationListener:public PublisherListener
	{
	public:
		OperationListener(EprosimaClient* up):mp_up(up){}
		~OperationListener(){}
		EprosimaClient* mp_up;
		void onPublicationMatched(Publisher* pub,MatchingInfo info);
	}m_operationsListener;
	class ResultListener:public SubscriberListener
	{
	public:
		ResultListener(EprosimaClient* up):mp_up(up){}
		~ResultListener(){}
		EprosimaClient* mp_up;
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo info);
		void onNewDataMessage(Subscriber* sub);
	}m_resultsListener;
	bool m_isReady;
	int m_operationMatched;
	int m_resultMatched;

};

#endif /* EPROSIMACLIENT_H_ */
