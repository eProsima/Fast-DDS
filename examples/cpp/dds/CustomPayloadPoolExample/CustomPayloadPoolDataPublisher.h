// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file CustomPayloadPoolDataPublisher.h
 *
 */

#ifndef CUSTOM_PAYLOAD_POOL_DATA_PUBLISHER_H_
#define CUSTOM_PAYLOAD_POOL_DATA_PUBLISHER_H_

#include "CustomPayloadPoolDataPubSubTypes.h"
#include "CustomPayloadPool.hpp"

#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>

class CustomPayloadPoolDataPublisher : public eprosima::fastdds::dds::DataWriterListener
{
public:

    CustomPayloadPoolDataPublisher(
            std::shared_ptr<CustomPayloadPool> payload_pool);

    ~CustomPayloadPoolDataPublisher();

    //!Initialize
    bool init();

    //!Publish a sample
    bool publish(
            bool wait_for_listener = true);

    //!Run for number samples
    void run(
            uint32_t number,
            uint32_t sleep);

private:

    CustomPayloadPoolData hello_;

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Publisher* publisher_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataWriter* writer_;

    bool stop_;

    void on_publication_matched(
            eprosima::fastdds::dds::DataWriter* writer,
            const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

    int matched_;

    bool first_connected_;

    void run_thread(
            uint32_t number,
            uint32_t sleep);

    eprosima::fastdds::dds::TypeSupport type_;

    std::shared_ptr<CustomPayloadPool> payload_pool_;
};



#endif /* CUSTOM_PAYLOAD_POOL_DATA_PUBLISHER_H_ */
