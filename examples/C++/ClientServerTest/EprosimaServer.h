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
	eprosima::fastrtps::Subscriber* mp_operation_sub;
	eprosima::fastrtps::Publisher* mp_result_pub;
	eprosima::fastrtps::Participant* mp_participant;
	clientserver::Result::RESULTTYPE calculate(clientserver::Operation::OPERATIONTYPE type, int32_t num1,int32_t num2,int32_t* result);
	clientserver::ResultDataType* mp_resultdatatype;
	clientserver::OperationDataType* mp_operationdatatype;
public:
	uint32_t m_n_served;
	class OperationListener:public eprosima::fastrtps::SubscriberListener
	{
	public:
		OperationListener(EprosimaServer* up):mp_up(up){}
		~OperationListener(){}
		EprosimaServer* mp_up;
        void onNewDataMessage(eprosima::fastrtps::Subscriber*sub);
        clientserver::Operation m_operation;
		eprosima::fastrtps::SampleInfo_t m_sampleInfo;
		clientserver::Result m_result;
	}m_operationsListener;

    class ResultListener: public eprosima::fastrtps::PublisherListener
	{
	public:
        ResultListener(EprosimaServer* up):mp_up(up){}
		~ResultListener(){}
		EprosimaServer* mp_up;
	}m_resultsListener;
};

#endif /* EPROSIMASERVER_H_ */
