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
 * @file TestPublisher.cpp
 *
 */

#include "TestPublisher.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/transport/TCPv6TransportDescriptor.h>
#include <fastrtps/transport/UDPv6TransportDescriptor.h>
#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>
#include <fastrtps/utils/IPLocator.h>
#include <gtest/gtest.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

TestPublisher::TestPublisher()
    : m_iSamples(-1)
    , m_sentSamples(0)
	, m_iWaitTime(1000)
    , m_bInitialized(false)
	, mp_participant(nullptr)
	, mp_publisher(nullptr)
	, m_pubListener(this)
{
}

bool TestPublisher::init(const std::string& topicName, int domain, eprosima::fastrtps::TopicDataType* type,
		const eprosima::fastrtps::types::TypeObject* type_object,
		const eprosima::fastrtps::types::TypeIdentifier* type_identifier,
		const eprosima::fastrtps::types::TypeInformation* type_info,
		const std::string& name,
        const eprosima::fastrtps::DataRepresentationQosPolicy* dataRepresentationQos,
		const eprosima::fastrtps::TypeConsistencyEnforcementQosPolicy* typeConsistencyQos)
{
    m_Name = name;
    m_Type = type;

    ParticipantAttributes PParam;
    PParam.rtps.builtin.domainId = domain;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.builtin.leaseDuration_announcementperiod = Duration_t(1, 0);
    PParam.rtps.setName(m_Name.c_str());

    mp_participant = Domain::createParticipant(PParam);

    if (mp_participant == nullptr)
    {
        return false;
    }

	// CREATE THE PUBLISHER
	PublisherAttributes Wparam;
	Wparam.topic.topicKind = m_Type->m_isGetKeyDefined ? WITH_KEY : NO_KEY;
    Wparam.topic.topicDataType = m_Type->getName();

    //REGISTER THE TYPE
    Domain::registerType(mp_participant, m_Type);

	Wparam.topic.topicName = topicName;
    if (type_object != nullptr)
    {
        Wparam.topic.type = *type_object;
        Wparam.qos.type = *type_object;
    }
    if (type_identifier != nullptr)
    {
        Wparam.topic.type_id = *type_identifier;
        Wparam.qos.type_id = *type_identifier;
    }
    if (type_info != nullptr)
    {
        Wparam.topic.type_information = *type_info;
        Wparam.qos.type_information = *type_info;
    }

    if (typeConsistencyQos != nullptr)
    {
        Wparam.topic.typeConsistencyQos = *typeConsistencyQos;
        Wparam.qos.m_typeConsistency = *typeConsistencyQos;
    }
    if (dataRepresentationQos != nullptr)
    {
        Wparam.topic.dataRepresentationQos = *dataRepresentationQos;
        Wparam.qos.m_dataRepresentation = *dataRepresentationQos;
    }
    // Wparam.topic.dataRepresentationQos = XCDR_DATA_REPRESENTATION
    // Wparam.topic.dataRepresentationQos = XML_DATA_REPRESENTATION
    // Wparam.topic.dataRepresentationQos = XCDR2_DATA_REPRESENTATION

    mp_publisher = Domain::createPublisher(mp_participant,Wparam,(PublisherListener*)&m_pubListener);
    if (mp_publisher == nullptr)
    {
        return false;
    }

    m_Data = m_Type->createData();

    m_bInitialized = true;

    return true;
}

TestPublisher::~TestPublisher()
{
    m_Type->deleteData(m_Data);
    Domain::removeParticipant(mp_participant);
}

void TestPublisher::waitDiscovery(bool expectMatch, int maxWait)
{
    std::unique_lock<std::mutex> lock(m_mDiscovery);

    if(m_pubListener.n_matched == 0)
        m_cvDiscovery.wait_for(lock, std::chrono::seconds(maxWait));

    if (expectMatch)
    {
        ASSERT_GE(m_pubListener.n_matched, 1);
    }
    else
    {
        ASSERT_EQ(m_pubListener.n_matched, 0);
    }
}

void TestPublisher::matched()
{
    std::unique_lock<std::mutex> lock(m_mDiscovery);
    ++m_pubListener.n_matched;
    if(m_pubListener.n_matched >= 1)
        m_cvDiscovery.notify_one();
}

TestPublisher::PubListener::PubListener(TestPublisher* parent)
    : mParent(parent)
	, n_matched(0)
{
}

void TestPublisher::PubListener::onPublicationMatched(Publisher* /*pub*/,MatchingInfo& info)
{
    if(info.status == MATCHED_MATCHING)
    {
        std::cout << mParent->m_Name << " matched." << std::endl;
        mParent->matched();
    }
    else
    {
        std::cout << mParent->m_Name << " unmatched."<<std::endl;
    }
}

void TestPublisher::runThread()
{
	int iPrevCount = 0;
	std::cout << m_Name << " running..." << std::endl;
    while (!publish() && iPrevCount < m_iSamples)
    {
		eClock::my_sleep(m_iWaitTime);
        ++iPrevCount;
	}
}

void TestPublisher::run()
{
    std::thread thread(&TestPublisher::runThread, this);
    thread.join();
}

bool TestPublisher::publish()
{
    if (m_pubListener.n_matched > 0)
    {
        if (mp_publisher->write(m_Data))
        {
            ++m_sentSamples;
            //std::cout << m_Name << " sent a total of " << m_sentSamples << " samples." << std::endl;
            return true;
        }
        //else
        //{
        //    std::cout << m_Name << " failed to send " << (m_sentSamples + 1) << " sample." << std::endl;
        //}
    }
    return false;
}
