// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
	//Serve indefinitely.
	void serve();
	//Serve for samples operations.
	void serve(uint32_t samples);
private:
	Subscriber* mp_operation_sub;
	Publisher* mp_result_pub;
	Participant* mp_participant;
	Result::RESULTTYPE calculate(Operation::OPERATIONTYPE type, int32_t num1,int32_t num2,int32_t* result);
	ResultDataType* mp_resultdatatype;
	OperationDataType* mp_operationdatatype;
public:
	uint32_t m_n_served;
	class OperationListener:public SubscriberListener
	{
	public:
		OperationListener(EprosimaServer* up):mp_up(up){}
		~OperationListener(){}
		EprosimaServer* mp_up;
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo& info);
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
		void onPublicationMatched(Publisher* pub,MatchingInfo& info);
	}m_resultsListener;
};

#endif /* EPROSIMASERVER_H_ */
