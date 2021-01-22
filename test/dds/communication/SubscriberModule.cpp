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

#include <asio.hpp>

#include "SubscriberModule.hpp"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>

#include <fstream>
#include <string>

using namespace eprosima::fastdds::dds;

SubscriberModule::~SubscriberModule()
{
    if (nullptr != topic_)
    {
        participant_->delete_topic(topic_);
    }

    if (nullptr != reader_)
    {
        subscriber_->delete_datareader(reader_);
    }

    if (nullptr != subscriber_)
    {
        participant_->delete_subscriber(subscriber_);
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

    //Do not enable entities on creation
    // DomainParticipantFactoryQos factory_qos;
    // factory_qos.entity_factory().autoenable_created_entities = false;
    // DomainParticipantFactory::get_instance()->set_qos(factory_qos);

    DomainParticipantQos participant_qos;
    participant_ =
            DomainParticipantFactory::get_instance()->create_participant(seed % 230, participant_qos, this);

    if (participant_ == nullptr)
    {
        std::cout << "Error creating subscriber participant" << std::endl;
        return false;
    }

    // Construct a FixedSizedType if fixed type is required, defult HelloWro
    if (fixed_type_)
    {
        type_ = new TypeSupport(new FixedSizedType());
    }
    else
    {
        type_ = new TypeSupport(new HelloWorldType());
    }
    type_->register_type(participant_);

    // Generate topic name
    std::ostringstream topic_name;
    topic_name << "HelloWorldTopic_" << ((magic.empty()) ? asio::ip::host_name() : magic) << "_" << seed;

    //CREATE THE SUBSCRIBER
    DataReaderQos rqos;
    rqos.liveliness().lease_duration = 3;
    rqos.liveliness().announcement_period = 1;
    rqos.liveliness().kind = eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS;

    // Defaut mask
    // StatusMask mask = StatusMask::subscription_matched()
            // << StatusMask::data_available()
            // << StatusMask::liveliness_changed();

    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, this);
    if (subscriber_ == nullptr)
    {
        std::cout << "Error creating subscriber" << std::endl;
        return false;
    }

    topic_ = participant_->create_topic(topic_name.str(), type_->get_type_name(), TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        std::cout << "Error creating subscriber topic" << std::endl;
        return false;
    }

    reader_ = subscriber_->create_datareader(topic_, rqos, this);
    if (reader_ == nullptr)
    {
        std::cout << "Error creating subscriber datareader" << std::endl;
        return false;
    }
    std::cout << "Reader created correctly in topic " << topic_->get_name()
            << " with type " << type_->get_type_name() << std::endl;

    std::cout << "Subscriber initialized correctly" << std::endl;

    return true;
}

bool SubscriberModule::run(
        bool notexit)
{
    return run_for(notexit, std::chrono::hours(24));
}

bool SubscriberModule::run_for(
        bool notexit,
        const std::chrono::milliseconds& timeout)
{
    std::cout << "Subscriber running with notexit " << notexit << std::endl;

    bool returned_value = false;

    while (notexit && run_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    if (run_)
    {
        std::cout << "Subscriber running" << std::endl;

        std::unique_lock<std::mutex> lock(mutex_);
        returned_value = cv_.wait_for(lock, timeout, [&]
        {
            if (publishers_ < number_samples_.size())
            {
                // Will fail later.
                std::cout << "Subscriber fails to connect with publishers" << std::endl;
                return true;
            }
            else if (publishers_ > number_samples_.size())
            {
                std::cout << "Subscriber fails to connect, too much publishers" << std::endl;
                return false;
            }

            for (auto& number_samples : number_samples_)
            {
                if (max_number_samples_ > number_samples.second)
                {
                    std::cout << "Subscriber fail, too few samples" << std::endl;
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

    return returned_value;
}

void SubscriberModule::on_participant_discovery(
        DomainParticipant* /*participant*/,
        fastrtps::rtps::ParticipantDiscoveryInfo&& info)
{
    if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
    {
        std::cout << "Subscriber participant " <<         //participant->getGuid() <<
            " discovered participant " << info.info.m_guid << std::endl;
    }
    else if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT)
    {
        std::cout << "Subscriber participant " <<         //participant->getGuid() <<
            " detected changes on participant " << info.info.m_guid << std::endl;
    }
    else if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT)
    {
        std::cout << "Subscriber participant " <<         //participant->getGuid() <<
            " removed participant " << info.info.m_guid << std::endl;
    }
    else if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT)
    {
        std::cout << "Subscriber participant " <<         //participant->getGuid() <<
            " dropped participant " << info.info.m_guid << std::endl;
    }
}

#if HAVE_SECURITY
void SubscriberModule::onParticipantAuthentication(
        DomainParticipant* /*participant*/,
        fastrtps::rtps::ParticipantAuthenticationInfo&& info)
{
    if (eprosima::fastrtps::rtps::ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT == info.status)
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
#endif

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
    std::cout << "Subscriber on_data_available from :" << participant_->guid() << std::endl;

    if (zero_copy_)
    {
        LoanableSequence<FixedSized> l_sample;
        LoanableSequence<SampleInfo> l_info;

        if(ReturnCode_t::RETCODE_OK == reader->take_next_instance(l_sample, l_info))
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
        void* sample;
        SampleInfo info;

        if (ReturnCode_t::RETCODE_OK == reader->take_next_sample(sample, &info))
        {
            std::cout << "Subscriber take next sample from :" << info.sample_identity.writer_guid() << std::endl;

            if (info.instance_state == ALIVE_INSTANCE_STATE)
            {
                std::cout << "Subscriber data received from :" << info.sample_identity.writer_guid() << std::endl;

                if (fixed_type_)
                {
                    FixedSized* data = static_cast<FixedSized*>(sample);
                    std::cout << "Received sample (" << info.sample_identity.writer_guid() << " - " <<
                        info.sample_identity.sequence_number() << "): index(" << data->index() << ")" << std::endl;
                }
                else
                {
                    HelloWorld* data = static_cast<HelloWorld*>(sample);
                    std::cout << "Received sample (" << info.sample_identity.writer_guid() << " - " <<
                        info.sample_identity.sequence_number() << "): index(" << data->index() << "), message("
                                << data->message() << ")" << std::endl;
                }

                if (max_number_samples_ <= ++number_samples_[info.sample_identity.writer_guid()])
                {
                    cv_.notify_all();
                }
            }
        }
        else
        {
            std::cout << "Subscriber error in take :" << std::endl;
        }
    }
}

void SubscriberModule::on_liveliness_changed(
        DataReader* /*reader*/,
        const eprosima::fastdds::dds::LivelinessChangedStatus& status)
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

void SubscriberModule::on_sample_rejected(
    DataReader* /*reader*/,
    const SampleRejectedStatus& /*status*/)
{
    std::cout << "Subscriber on_sample_rejected" << std::endl;
}

void SubscriberModule::on_sample_lost(
    DataReader* /*reader*/,
    const SampleLostStatus& /*status*/)
{
    std::cout << "Subscriber on_sample_lost" << std::endl;
}
