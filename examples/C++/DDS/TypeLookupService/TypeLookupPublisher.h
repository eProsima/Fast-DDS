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
 * @file TypeLookupPublisher.h
 *
 */

#ifndef HELLOWORLDPUBLISHER_H_
#define HELLOWORLDPUBLISHER_H_

#include <fastdds/dds/topic/DataWriterListener.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>

#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicDataPtr.h>

class TypeLookupPublisher
{
public:

    TypeLookupPublisher();

    virtual ~TypeLookupPublisher();

    //!Initialize
    bool init();

    //!Publish a sample
    bool publish(
            bool waitForListener = true);

    //!Run for number samples
    void run(
            uint32_t number,
            uint32_t sleep);

private:

    eprosima::fastrtps::types::DynamicData_ptr m_Hello;

    eprosima::fastdds::dds::DomainParticipant* mp_participant;

    eprosima::fastdds::dds::Publisher* mp_publisher;

    eprosima::fastdds::dds::DataWriter* writer_;

    eprosima::fastdds::dds::Topic* topic_;

    bool stop;

    class PubListener : public eprosima::fastdds::dds::DataWriterListener
    {
public:

        PubListener()
            : n_matched(0)
            , firstConnected(false)
        {
        }

        ~PubListener() override
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        void on_offered_incompatible_qos(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::OfferedIncompatibleQosStatus& status) override;

        int n_matched;

        bool firstConnected;
    } m_listener;

    void runThread(
            uint32_t number,
            uint32_t sleep);
};



#endif /* HELLOWORLDPUBLISHER_H_ */
