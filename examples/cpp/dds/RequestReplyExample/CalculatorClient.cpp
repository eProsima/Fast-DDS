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

class CalculatorClient
{
    class Listener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override
        {
            ReplyType reply;
            eprosima::fastdds::dds::SampleInfo sample_info;

            reader->take_next_sample(&reply, &sample_info);

            if (eprosima::fastdds::dds::InstanceStateKind::ALIVE_INSTANCE_STATE == sample_info.instance_state)
            {
                if (sample_info.related_sample_identity == write_params.sample_identity())
                {
                    {
                        std::unique_lock<std::mutex> lock(reception_mutex);
                        received_reply = true;
                        z = reply.z();
                    }
                    reception_cv.notify_one();
                }
            }
        }

        eprosima::fastrtps::rtps::WriteParams write_params;

        // Structures for waiting reply
        std::mutex reception_mutex;
        std::condition_variable reception_cv;
        bool received_reply = false;
        int64_t z = 0;

    }
    listener_;

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

        request_writer_ = publisher_->create_datawriter(request_topic_, eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT);

        if (nullptr == request_writer_)
        {
            return false;
        }

        eprosima::fastdds::dds::DataReaderQos reader_qos;
        reader_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        reader_qos.history().kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
        reply_reader_ = subscriber_->create_datareader(reply_topic_, reader_qos, &listener_);

        if (nullptr == reply_reader_)
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

    int64_t request(
            OperationType operation,
            int32_t x,
            int32_t y)
    {
        int64_t z = 0;
        RequestType request;
        request.operation(operation);
        request.x(x);
        request.y(y);

        if (request_writer_->write(static_cast<void*>(&request), listener_.write_params))
        {
            std::unique_lock<std::mutex> lock(listener_.reception_mutex);
            listener_.reception_cv.wait(lock, [&]()
                    {
                        return listener_.received_reply;
                    });
            z = listener_.z;
        }
        else
        {
            std::cerr << "Error writing the request" << std::endl;
        }

        return z;
    }

private:

    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;

    eprosima::fastdds::dds::Publisher* publisher_ = nullptr;

    eprosima::fastdds::dds::Subscriber* subscriber_ = nullptr;

    eprosima::fastdds::dds::Topic* request_topic_ = nullptr;

    eprosima::fastdds::dds::Topic* reply_topic_ = nullptr;

    eprosima::fastdds::dds::DataWriter* request_writer_ = nullptr;

    eprosima::fastdds::dds::DataReader* reply_reader_ = nullptr;

    eprosima::fastdds::dds::TypeSupport request_type_;

    eprosima::fastdds::dds::TypeSupport reply_type_;
};

void print_help()
{
    std::cout << "Usage: CalculatorClient <x> <+|-|x|/> <y>" << std::endl;
}

int main(
        int argc,
        char** argv)
{
    argc -= (argc > 0);
    argv += (argc > 0); // skip program name argv[0] if present

    if (3 != argc)
    {
        print_help();
        return -1;
    }

    int32_t x;
    int32_t y;
    OperationType operation;
    char* endptr = 0;

    x = strtol(argv[0], &endptr, 10);
    if (endptr == argv[0] || *endptr != 0)
    {
        std::cerr << "Error reading numeric argument x." << std::endl;
        print_help();
        return -1;
    }

    if (0 == strcmp(argv[1], "+"))
    {
        operation = OperationType::ADDITION;
    }
    else if (0 == strcmp(argv[1], "-"))
    {
        operation = OperationType::SUBTRACTION;
    }
    else if (0 == strcmp(argv[1], "x"))
    {
        operation = OperationType::MULTIPLICATION;
    }
    else if (0 == strcmp(argv[1], "/"))
    {
        operation = OperationType::DIVISION;
    }
    else
    {
        std::cerr << "Error reading operation argument. Valid values: <+|-|x|/>" << std::endl;
        print_help();
        return -1;
    }

    y = strtol(argv[2], &endptr, 10);
    if (endptr == argv[2] || *endptr != 0)
    {
        std::cerr << "Error reading numeric argument y." << std::endl;
        print_help();
        return -1;
    }

    CalculatorClient client;
    client.init();
    int64_t z = client.request(operation, x, y);

    std::cout << "Result: " << argv[0] << " " << argv[1] << " " << argv[2] << " = " << z << std::endl;

    client.deinit();

    return 0;
}
