// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <condition_variable>
#include <mutex>
#include <vector>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastrtps/subscriber/SampleInfo.h>
#include "CalculatorPubSubTypes.h"

class CalculatorServer
{
    class Listener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:

        Listener(
                eprosima::fastdds::dds::DataWriter* writer)
            : writer_(writer)
        {
        }

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override
        {
            RequestType request;
            eprosima::fastdds::dds::SampleInfo sample_info;

            reader->take_next_sample(&request, &sample_info);

            if (eprosima::fastdds::dds::InstanceStateKind::ALIVE_INSTANCE_STATE == sample_info.instance_state)
            {
                ReplyType reply;

                if (OperationType::ADDITION == request.operation())
                {
                    reply.z(request.x() + request.y());
                }
                else if (OperationType::SUBTRACTION == request.operation())
                {
                    reply.z(request.x() - request.y());
                }
                else if (OperationType::MULTIPLICATION == request.operation())
                {
                    reply.z(request.x() * request.y());
                }
                else if (OperationType::DIVISION == request.operation())
                {
                    if (0 != request.y())
                    {
                        reply.z(request.x() / request.y());
                    }
                }
                eprosima::fastrtps::rtps::WriteParams write_params;
                write_params.related_sample_identity().writer_guid(sample_info.sample_identity.writer_guid());
                write_params.related_sample_identity().sequence_number(sample_info.sample_identity.sequence_number());
                writer_->write(reinterpret_cast<void*>(&reply), write_params);
            }
        }

    private:

        eprosima::fastdds::dds::DataWriter* writer_ = nullptr;

    };

public:

    bool init()
    {
        participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(0,
                        eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);

        if (nullptr == participant_)
        {
            return false;
        }

        request_type_ = eprosima::fastdds::dds::TypeSupport(new RequestTypePubSubType());
        reply_type_ = eprosima::fastdds::dds::TypeSupport(new ReplyTypePubSubType());

        participant_->register_type(request_type_);
        participant_->register_type(reply_type_);

        publisher_ = participant_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT);

        if (nullptr == publisher_)
        {
            return false;
        }

        subscriber_ = participant_->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);

        if (nullptr == subscriber_)
        {
            return false;
        }

        request_topic_ = participant_->create_topic("CalculatorRequest",
                        request_type_.get_type_name(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);

        if (nullptr == request_topic_)
        {
            return false;
        }

        reply_topic_ = participant_->create_topic("CalculatorReply",
                        reply_type_.get_type_name(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);

        if (nullptr == reply_topic_)
        {
            return false;
        }

        eprosima::fastdds::dds::DataWriterQos writer_qos;
        writer_qos.history().kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
        reply_writer_ = publisher_->create_datawriter(reply_topic_, writer_qos);

        if (nullptr == reply_writer_)
        {
            return false;
        }

        listener_ = { reply_writer_};

        eprosima::fastdds::dds::DataReaderQos reader_qos;
        reader_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        reader_qos.durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
        reader_qos.history().kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
        request_reader_ = subscriber_->create_datareader(request_topic_, reader_qos, &listener_);

        if (nullptr == request_reader_)
        {
            return false;
        }

        return true;
    }

    void deinit()
    {
        if (nullptr != participant_)
        {
            participant_->delete_contained_entities();
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
        }
    }

private:

    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;

    eprosima::fastdds::dds::Publisher* publisher_ = nullptr;

    eprosima::fastdds::dds::Subscriber* subscriber_ = nullptr;

    eprosima::fastdds::dds::Topic* request_topic_ = nullptr;

    eprosima::fastdds::dds::Topic* reply_topic_ = nullptr;

    eprosima::fastdds::dds::DataWriter* reply_writer_ = nullptr;

    eprosima::fastdds::dds::DataReader* request_reader_ = nullptr;

    eprosima::fastdds::dds::TypeSupport request_type_;

    eprosima::fastdds::dds::TypeSupport reply_type_;

    Listener listener_ = {nullptr};
};

int main()
{
    CalculatorServer server;
    server.init();

    std::cout << "Press a key + ENTER to close the server" << std::endl;
    char c;
    std::cin >> c;

    server.deinit();

    return 0;
}
