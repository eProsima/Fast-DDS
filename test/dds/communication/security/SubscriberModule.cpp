// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SubscriberModule.cpp
 *
 */

#include "SubscriberModule.hpp"

#include <chrono>
#include <fstream>
#include <string>
#include <thread>

#include <asio.hpp>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

SubscriberModule::~SubscriberModule()
{
    if (nullptr != reader_)
    {
        subscriber_->delete_datareader(reader_);
    }

    if (nullptr != subscriber_)
    {
        participant_->delete_subscriber(subscriber_);
    }

    if (nullptr != topic_)
    {
        participant_->delete_topic(topic_);
    }

    if (nullptr != participant_)
    {
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

bool SubscriberModule::init(
        uint32_t seed,
        const std::string& magic)
{
    std::cout << "Initializing Subscriber" << std::endl;

    StatusMask mask = StatusMask::subscription_matched()
            << StatusMask::data_available()
            << StatusMask::liveliness_changed();

    participant_ =
            DomainParticipantFactory::get_instance()->create_participant(seed % 230, PARTICIPANT_QOS_DEFAULT, this,
                    mask);

    if (participant_ == nullptr)
    {
        std::cout << "Error creating subscriber participant" << std::endl;
        return false;
    }

    // Construct a FixedSizedType if fixed type is required, defult HelloWro
    if (fixed_type_)
    {
        type_.reset(new FixedSizedPubSubType());
    }
    else
    {
        type_.reset(new HelloWorldPubSubType());
    }
    type_.register_type(participant_);

    // Generate topic name
    std::ostringstream topic_name;
    topic_name << "HelloWorldTopic_" << ((magic.empty()) ? asio::ip::host_name() : magic) << "_" << seed;

    //CREATE THE SUBSCRIBER
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {
        std::cout << "Error creating subscriber" << std::endl;
        return false;
    }

    //CREATE THE TOPIC
    topic_ = participant_->create_topic(topic_name.str(), type_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        std::cout << "Error creating subscriber topic" << std::endl;
        return false;
    }

    //CREATE THE DATAREADER
    DataReaderQos rqos = subscriber_->get_default_datareader_qos();
    rqos.liveliness().lease_duration = 3;
    rqos.liveliness().announcement_period = 1;
    rqos.liveliness().kind = AUTOMATIC_LIVELINESS_QOS;

    reader_ = subscriber_->create_datareader(topic_, rqos);
    if (reader_ == nullptr)
    {
        std::cout << "Error creating subscriber datareader" << std::endl;
        return false;
    }
    std::cout << "Reader created correctly in topic " << topic_->get_name()
              << " with type " << type_.get_type_name() << std::endl;

    std::cout << "Subscriber initialized correctly" << std::endl;

    return true;
}

bool SubscriberModule::run(
        bool notexit,
        const uint32_t rescan_interval)
{
    return run_for(notexit, rescan_interval, std::chrono::hours(24));
}

bool SubscriberModule::run_for(
        bool notexit,
        const uint32_t rescan_interval,
        const std::chrono::milliseconds& timeout)
{
    bool returned_value = false;

    std::thread net_rescan_thread([this, rescan_interval]()
            {
                if (rescan_interval > 0)
                {
                    auto interval = std::chrono::seconds(rescan_interval);
                    while (run_)
                    {
                        std::this_thread::sleep_for(interval);
                        if (run_)
                        {
                            participant_->set_qos(participant_->get_qos());
                        }
                    }
                }
            });

    while (notexit && run_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    if (run_)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        returned_value = cv_.wait_for(lock, timeout, [&]
                        {
                            if (publishers_ < number_samples_.size())
                            {
                                // Will fail later.
                                return true;
                            }
                            else if (publishers_ > number_samples_.size())
                            {
                                return false;
                            }

                            for (auto& number_samples : number_samples_)
                            {
                                if (max_number_samples_ > number_samples.second)
                                {
                                    return false;
                                }
                            }

                            return true;
                        });
    }
    else
    {
        returned_value = true;
    }


    if (publishers_ < number_samples_.size())
    {
        std::cout << "ERROR: detected more than " << publishers_ << " publishers" << std::endl;
        returned_value = false;
    }

    run_ = false;
    net_rescan_thread.join();

    return returned_value;
}

void SubscriberModule::on_participant_discovery(
        DomainParticipant* /*participant*/,
        ParticipantDiscoveryStatus status,
        const ParticipantBuiltinTopicData& info,
        bool& /*should_be_ignored*/)
{
    if (status == ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT)
    {
        std::cout << "Subscriber participant " <<         //participant->getGuid() <<
            " discovered participant " << info.guid << std::endl;
    }
    else if (status == ParticipantDiscoveryStatus::CHANGED_QOS_PARTICIPANT)
    {
        std::cout << "Subscriber participant " <<         //participant->getGuid() <<
            " detected changes on participant " << info.guid << std::endl;
    }
    else if (status == ParticipantDiscoveryStatus::REMOVED_PARTICIPANT)
    {
        std::cout << "Subscriber participant " <<         //participant->getGuid() <<
            " removed participant " << info.guid << std::endl;
    }
    else if (status == ParticipantDiscoveryStatus::DROPPED_PARTICIPANT)
    {
        std::cout << "Subscriber participant " <<         //participant->getGuid() <<
            " dropped participant " << info.guid << std::endl;
    }
}

#if HAVE_SECURITY
void SubscriberModule::onParticipantAuthentication(
        DomainParticipant* /*participant*/,
        ParticipantAuthenticationInfo&& info)
{
    if (ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT == info.status)
    {
        std::cout << "Subscriber participant " <<         //participant->getGuid() <<
            " authorized participant " << info.guid << std::endl;
    }
    else
    {
        std::cout << "Subscriber participant " <<         //participant->getGuid() <<
            " unauthorized participant " << info.guid << std::endl;
    }
}

#endif // if HAVE_SECURITY

void SubscriberModule::on_subscription_matched(
        DataReader* /*reader*/,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "Subscriber matched with publisher " << info.last_publication_handle << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "Subscriber unmatched with publisher " << info.last_publication_handle << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void SubscriberModule::on_data_available(
        DataReader* reader)
{
    if (die_on_data_received_)
    {
        std::abort();
    }
    std::cout << "Subscriber on_data_available from :" << participant_->guid() << std::endl;

    if (zero_copy_)
    {
        LoanableSequence<FixedSized> l_sample;
        LoanableSequence<SampleInfo> l_info;

        if (RETCODE_OK == reader->take_next_instance(l_sample, l_info))
        {
            SampleInfo info = l_info[0];

            if (info.valid_data && info.instance_state == ALIVE_INSTANCE_STATE)
            {
                FixedSized& data = l_sample[0];

                std::cout << "Received sample (" << info.sample_identity.writer_guid() << " - " <<
                    info.sample_identity.sequence_number() << "): index(" << data.index() << ")" << std::endl;


                if (max_number_samples_ <= ++number_samples_[info.sample_identity.writer_guid()])
                {
                    cv_.notify_all();
                }
            }
        }
        reader->return_loan(l_sample, l_info);
    }
    else
    {
        SampleInfo info;

        if (fixed_type_)
        {
            FixedSized sample;
            if (reader->take_next_sample((void*)&sample, &info) == RETCODE_OK)
            {
                if (info.instance_state == ALIVE_INSTANCE_STATE)
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    std::cout << "Received sample (" << info.sample_identity.writer_guid() << " - " <<
                        info.sample_identity.sequence_number() << "): index(" << sample.index() << ")" << std::endl;
                    if (max_number_samples_ <= ++number_samples_[info.sample_identity.writer_guid()])
                    {
                        cv_.notify_all();
                    }
                }
            }
        }
        else
        {
            HelloWorld sample;
            if (reader->take_next_sample((void*)&sample, &info) == RETCODE_OK)
            {
                if (info.instance_state == ALIVE_INSTANCE_STATE)
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    std::cout << "Received sample (" << info.sample_identity.writer_guid() << " - " <<
                        info.sample_identity.sequence_number() << "): index(" << sample.index() << "), message("
                              << sample.message() << ")" << std::endl;
                    if (max_number_samples_ <= ++number_samples_[info.sample_identity.writer_guid()])
                    {
                        cv_.notify_all();
                    }
                }
            }
        }
    }
}

void SubscriberModule::on_liveliness_changed(
        DataReader* /*reader*/,
        const LivelinessChangedStatus& status)
{
    if (status.alive_count_change == 1)
    {
        std::cout << "Subscriber recovered liveliness" << std::endl;
    }
    else if (status.not_alive_count_change == 1)
    {
        std::cout << "Subscriber lost liveliness" << std::endl;
        run_ = false;
    }
}
