/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EprosimaClient.h
 *
 */

#ifndef EPROSIMACLIENT_H_
#define EPROSIMACLIENT_H_

#include "eprosimartps/rtps_all.h"
#include "ClientServerTypes.h"

using namespace clientserver;

class EprosimaClient {
public:
	EprosimaClient();
	virtual ~EprosimaClient();
	Publisher* mp_operation_pub;
	Subscriber* mp_result_sub;
	RTPSParticipant* mp_RTPSParticipant;
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
		void onPublicationMatched(MatchingInfo info);
	}m_operationsListener;
	class ResultListener:public SubscriberListener
	{
	public:
		ResultListener(EprosimaClient* up):mp_up(up){}
		~ResultListener(){}
		EprosimaClient* mp_up;
		void onSubscriptionMatched(MatchingInfo info);
		void onNewDataMessage();
	}m_resultsListener;
	bool m_isReady;
	int m_operationMatched;
	int m_resultMatched;

};

#endif /* EPROSIMACLIENT_H_ */
