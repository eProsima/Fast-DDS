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

#include <gtest/gtest.h>

#include "BlackboxTests.hpp"
#include "../dds-pim/PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include <fastdds/statistics/dds/domain/DomainParticipant.hpp>
#include <fastrtps/transport/test_UDPv4TransportDescriptor.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <statistics/fastdds/domain/DomainParticipantImpl.hpp>
#include <statistics/rtps/StatisticsBase.hpp>
#include <statistics/types/monitorservice_typesPubSubTypes.h>

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::dds;

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class DDSMonitorServiceTest : public testing::TestWithParam<communication_type>

{
public:

    void SetUp() override
    {
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = true;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

    void TearDown() override
    {
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = false;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

};

using MonitorServiceType = eprosima::fastdds::statistics::MonitorServiceStatusDataPubSubType;
using GUIDList = std::vector<GUID_t>;
using StatisticsGUIDList = std::vector<statistics::detail::GUID_s>;

struct SampleValidator;

#ifdef FASTDDS_STATISTICS

void validator_selector(
        statistics::dds::DomainParticipant* participant,
        SampleValidator*& validator,
        const statistics::StatusKind status_kind,
        SampleInfo& info,
        MonitorServiceType::type& data,
        std::list<MonitorServiceType::type>& total_msgs,
        std::atomic<size_t>& processed_count,
        std::condition_variable& cv);

class MonitorServiceParticipant
{

public:

    enum CallbackIndex
    {
        OFFERED_DEADLINE_MISSED_IDX,
        OFFERED_INCOMPATIBLE_QOS_IDX,
        LIVELINESS_LOST_IDX,
        PUBLICATION_MATCHED_IDX,
        REQUESTED_DEADLINE_MISSED_IDX,
        REQUESTED_INCOMPATIBLE_QOS_IDX,
        LIVELINESS_CHANGED_IDX,
        SUBSCRIPTION_MATCHED_IDX,
        SAMPLE_LOST_IDX,
        MAX_SIZE
    };

    MonitorServiceParticipant()
        : type_support_(new HelloWorldPubSubType())
    {

    }

    ~MonitorServiceParticipant() = default;

    void setup(
            const DomainParticipantQos& qos)
    {
        auto participant = DomainParticipantFactory::get_instance()->
                        create_participant((uint32_t)GET_PID() % 230, qos);

        setup(participant);
    }

    void setup(
            std::string profiles_file,
            std::string profile)
    {
        //! Load XML profiles
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_XML_profiles_file(profiles_file);

        eprosima::fastdds::dds::DomainParticipant* participant =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                        create_participant_with_profile((uint32_t)GET_PID() % 230, profile);

        setup(participant);
    }

    void setup()
    {
        auto participant = DomainParticipantFactory::get_instance()->
                        create_participant((uint32_t)GET_PID() % 230, PARTICIPANT_QOS_DEFAULT);

        setup(participant);
    }

    void setup(
            DomainParticipant* participant)
    {
        statistics_part_ = statistics::dds::DomainParticipant::narrow(participant);
        ASSERT_NE(statistics_part_, nullptr);

        type_support_.register_type(participant);

        publisher_ = statistics_part_->create_publisher(PUBLISHER_QOS_DEFAULT);
        ASSERT_NE(publisher_, nullptr);

        subscriber_ = statistics_part_->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
        ASSERT_NE(subscriber_, nullptr);

        create_topic();
    }

    void assert_liveliness()
    {
        for (auto& writer : writers_)
        {
            writer->assert_liveliness();
        }
    }

    const StatisticsGUIDList& get_writer_guids()
    {
        return writer_stat_guids_;
    }

    const StatisticsGUIDList& get_reader_guids()
    {
        return reader_stat_guids_;
    }

    statistics::detail::GUID_s get_participant_guid()
    {
        return statistics::to_statistics_type(statistics_part_->guid());
    }

    ReturnCode_t enable_monitor_service()
    {
        return statistics_part_->enable_monitor_service();
    }

    ReturnCode_t disable_monitor_service()
    {
        return statistics_part_->disable_monitor_service();
    }

    const uint32_t& get_cb_count(
            CallbackIndex cb_idx)
    {
        return listener_.get_cb_count_of(cb_idx);
    }

    void create_topic(
            int topic_number = 0)
    {
        topics_.emplace_back(statistics_part_->create_topic("test_" + std::to_string(topic_number),
                type_support_.get_type_name(), TOPIC_QOS_DEFAULT));
        ASSERT_NE(topics_.back(), nullptr);
    }

    void create_and_add_writer(
            const DataWriterQos& qos = DATAWRITER_QOS_DEFAULT)
    {
        writers_.emplace_back(publisher_->create_datawriter(topics_.back(), qos, &listener_));
        ASSERT_NE(writers_.back(), nullptr);

        writer_stat_guids_.push_back(statistics::to_statistics_type(writers_.back()->guid()));

        std::cout << "Created datawriter " << writers_.back()->guid() << " for topic " <<
            topics_.back()->get_name() << std::endl;
    }

    void create_and_add_reader(
            const DataReaderQos& qos = DATAREADER_QOS_DEFAULT)
    {
        DataReaderQos dr_qos{qos};

        if (enable_datasharing)
        {
            dr_qos.data_sharing().automatic();
        }
        else
        {
            dr_qos.data_sharing().off();
        }

        readers_.emplace_back(subscriber_->create_datareader(topics_.back(), dr_qos, &listener_));
        ASSERT_NE(readers_.back(), nullptr);

        reader_stat_guids_.push_back(statistics::to_statistics_type(readers_.back()->guid()));

        std::cout << "Created datareader " << readers_.back()->guid() << " for topic " <<
            topics_.back()->get_name() << std::endl;
    }

    bool delete_writer()
    {
        if (!writers_.empty())
        {
            return (ReturnCode_t::RETCODE_OK == publisher_->delete_datawriter(writers_.back()));
        }

        return false;
    }

    void send(
            std::list<HelloWorld>& msgs,
            uint32_t milliseconds = 0)
    {
        auto it = msgs.begin();

        bool writing_failed {false};
        while (it != msgs.end() && !writing_failed)
        {
            for (auto& writer : writers_)
            {
                if (writer->write((void*)&(*it)))
                {
                    default_send_print<HelloWorld>(*it);
                    it = msgs.erase(it);
                    if (milliseconds > 0)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
                    }
                }
                else
                {
                    writing_failed = true;
                    break;
                }
            }
        }
    }

    bool write_sample(
            HelloWorld& sample)
    {
        if (!writers_.empty())
        {
            return writers_.back()->write(&sample);
        }

        return false;
    }

    void reset()
    {
        ASSERT_EQ(ReturnCode_t::RETCODE_OK, statistics_part_->delete_contained_entities());
        ASSERT_EQ(ReturnCode_t::RETCODE_OK,
                DomainParticipantFactory::get_instance()->delete_participant(statistics_part_));
    }

protected:

    statistics::dds::DomainParticipant* statistics_part_;

    Publisher* publisher_;
    Subscriber* subscriber_;

    std::vector<Topic*> topics_;

    std::vector<DataWriter*> writers_;
    std::vector<DataReader*> readers_;

    StatisticsGUIDList writer_stat_guids_;
    StatisticsGUIDList reader_stat_guids_;

    TypeSupport type_support_;

    struct MonitorServiceParticipantListener : public DataReaderListener,
        public DataWriterListener
    {
        MonitorServiceParticipantListener()
        {
            cb_counters_.fill(0);
        }

        MonitorServiceParticipantListener(
                const MonitorServiceParticipantListener& other)
        {
            std::unique_lock<std::mutex> lock(other.mtx_);
            this->cb_counters_ = other.cb_counters_;
        }

        MonitorServiceParticipantListener(
                MonitorServiceParticipantListener&& other)
        {
            std::unique_lock<std::mutex> lock(other.mtx_);
            this->cb_counters_ = std::move(other.cb_counters_);
        }

        virtual ~MonitorServiceParticipantListener()
        {
        }

        void on_offered_deadline_missed (
                DataWriter* writer,
                const OfferedDeadlineMissedStatus& status) override
        {
            {
                std::unique_lock<std::mutex> lock(mtx_);
                ++cb_counters_[OFFERED_DEADLINE_MISSED_IDX];
            }

            std::cout << "on_offered_deadline_missed() " << writer->guid() << " total_count " << status.total_count <<
                std::endl;
        }

        void on_offered_incompatible_qos (
                DataWriter* writer,
                const OfferedIncompatibleQosStatus& status) override
        {
            {
                std::unique_lock<std::mutex> lock(mtx_);
                ++cb_counters_[OFFERED_INCOMPATIBLE_QOS_IDX];
            }

            std::cout << "on_offered_incompatible_qos " << writer->guid() << " total_count " << status.total_count <<
                std::endl;
        }

        void on_liveliness_lost (
                DataWriter* writer,
                const LivelinessLostStatus& status) override
        {
            {
                std::unique_lock<std::mutex> lock(mtx_);
                ++cb_counters_[LIVELINESS_LOST_IDX];
            }

            std::cout << "on_liveliness_lost " << writer->guid() << " total_count " << status.total_count << std::endl;
        }

        void on_publication_matched (
                DataWriter* writer,
                const PublicationMatchedStatus& status) override
        {
            {
                std::unique_lock<std::mutex> lock(mtx_);
                ++cb_counters_[PUBLICATION_MATCHED_IDX];
            }

            std::cout << "on_publication_matched " << writer->guid() << " total_count " << status.total_count <<
                std::endl;
        }

        void on_requested_deadline_missed (
                DataReader* reader,
                const RequestedDeadlineMissedStatus& status) override
        {
            {
                std::unique_lock<std::mutex> lock(mtx_);
                ++cb_counters_[REQUESTED_DEADLINE_MISSED_IDX];
            }

            std::cout << "on_requested_deadline_missed" << reader->guid() << " total_count " << status.total_count <<
                std::endl;
        }

        void on_requested_incompatible_qos (
                DataReader* reader,
                const RequestedIncompatibleQosStatus& status) override
        {
            {
                std::unique_lock<std::mutex> lock(mtx_);
                ++cb_counters_[REQUESTED_INCOMPATIBLE_QOS_IDX];
            }

            std::cout << "on_requested_incompatible_qos" << reader->guid() << " total_count " << status.total_count <<
                std::endl;
        }

        void on_liveliness_changed (
                DataReader* reader,
                const LivelinessChangedStatus& status) override
        {
            {
                std::unique_lock<std::mutex> lock(mtx_);
                ++cb_counters_[LIVELINESS_CHANGED_IDX];
            }

            std::cout << "on_liveliness_changed " << reader->guid() << " not_alive_count " << status.not_alive_count <<
                std::endl;
        }

        void on_subscription_matched (
                DataReader* reader,
                const SubscriptionMatchedStatus& status) override
        {
            {
                std::unique_lock<std::mutex> lock(mtx_);
                ++cb_counters_[SUBSCRIPTION_MATCHED_IDX];
            }

            std::cout << "on_subscription_matched " << reader->guid() << " total_count " << status.total_count <<
                std::endl;
        }

        void on_sample_lost (
                DataReader* reader,
                const SampleLostStatus& status) override
        {
            {
                std::unique_lock<std::mutex> lock(mtx_);
                ++cb_counters_[SAMPLE_LOST_IDX];
            }

            std::cout << "on_sample_lost " << reader->guid() << " total_count " << status.total_count << std::endl;
        }

        const uint32_t& get_cb_count_of(
                CallbackIndex cb_idx)
        {
            std::unique_lock<std::mutex> lock(mtx_);
            return cb_counters_[cb_idx];
        }

    private:

        mutable std::mutex mtx_;
        std::array<uint32_t, CallbackIndex::MAX_SIZE> cb_counters_;

    }
    listener_;
};

struct SampleValidator
{
    //! Avoid decalaring it as virtual
    //! in order to correctly apply static_cast
    void validate(
            SampleInfo&,
            MonitorServiceType::type&,
            std::list<MonitorServiceType::type>&,
            std::atomic<size_t>&,
            std::condition_variable&,
            statistics::dds::DomainParticipant*&)
    {
    }

};

class MonitorServiceConsumer : protected PubSubReader<MonitorServiceType>
{

public:

    MonitorServiceConsumer()
        : PubSubReader<MonitorServiceType>("fastdds_monitor_service_status", true, true)
        , sample_validator_( new SampleValidator())
    {
        statistics_part_ = statistics::dds::DomainParticipant::narrow(get_participant());
    }

    virtual ~MonitorServiceConsumer()
    {
    }

    void init_monitor_service_reader()
    {
        reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
        durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS);
        history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS);
        history_depth(1);
        init();
    }

    SequenceNumber_t start_reception(
            const std::list<MonitorServiceType::type>& msgs)
    {
        return startReception(msgs);
    }

    template<class _Rep,
            class _Period>
    size_t block_for_all(
            const std::chrono::duration<_Rep, _Period>& time)
    {
        return PubSubReader<MonitorServiceType>::block_for_all(time);
    }

    void stop()
    {
        destroy();
    }

protected:

    void receive_one(
            eprosima::fastdds::dds::DataReader* datareader,
            bool& returnedValue) override
    {
        returnedValue = false;
        type data;
        eprosima::fastdds::dds::SampleInfo info;

        ReturnCode_t success = datareader->take_next_sample((void*)&data, &info);

        if (ReturnCode_t::RETCODE_OK == success)
        {
            returnedValue = true;

            std::unique_lock<std::mutex> lock(mutex_);

            // Check order of changes
            LastSeqInfo seq_info{ info.instance_handle, info.sample_identity.writer_guid() };
            ASSERT_LT(last_seq[seq_info], info.sample_identity.sequence_number());
            last_seq[seq_info] = info.sample_identity.sequence_number();

            validator_selector(statistics_part_, sample_validator_,
                    data.status_kind(), info, data, total_msgs_, current_processed_count_, cv_);
        }
    }

    SampleValidator* sample_validator_;

    statistics::dds::DomainParticipant* statistics_part_;
};

struct ProxySampleValidator : public SampleValidator
{
    void validate(
            SampleInfo& info,
            MonitorServiceType::type& data,
            std::list<MonitorServiceType::type>& total_msgs,
            std::atomic<size_t>& processed_count,
            std::condition_variable& cv,
            statistics::dds::DomainParticipant* participant = nullptr)
    {
        if (info.valid_data
                && info.instance_state == eprosima::fastdds::dds::ALIVE_INSTANCE_STATE)
        {
            auto it = std::find_if(total_msgs.begin(), total_msgs.end(),
                            [&](const MonitorServiceType::type& elem)
                            {
                                return (data.status_kind() == elem.status_kind()) &&
                                data.local_entity() == elem.local_entity();
                            });

            ASSERT_NE(it, total_msgs.end());

            std::cout << "Received Proxy on local_entity "
                      << statistics::to_fastdds_type(data.local_entity()) << std::endl;

            //! Analyze the discriminant
            auto serialized_proxy(std::move(data.value().entity_proxy()));

            CDRMessage_t serialized_msg;
            serialized_msg.init(serialized_proxy.data(), (uint32_t) data.value().getCdrSerializedSize(data.value()));

            GUID_t guid = statistics::to_fastdds_type(data.local_entity());

            bool valid_entity = true;

            if (guid.entityId == c_EntityId_RTPSParticipant)
            {
                RTPSParticipantAllocationAttributes att;
                ParticipantProxyData pdata(att);

                serialized_msg.init(serialized_proxy.data(),
                        (uint32_t) data.value().getCdrSerializedSize(data.value()));
                ASSERT_EQ(participant->fill_discovery_data_from_cdr_message(pdata, data), ReturnCode_t::RETCODE_ERROR);
            }
            else
            {
                valid_entity = false;
            }

            if (valid_entity)
            {
                total_msgs.erase(it);
                ++processed_count;
                cv.notify_one();
            }
        }
    }

};

struct ConnectionListSampleValidator : public SampleValidator
{
    void validate(
            SampleInfo& info,
            MonitorServiceType::type& data,
            std::list<MonitorServiceType::type>& total_msgs,
            std::atomic<size_t>& processed_count,
            std::condition_variable& cv)
    {
        if (info.valid_data
                && info.instance_state == eprosima::fastdds::dds::ALIVE_INSTANCE_STATE)
        {
            std::cout << "Received ConnectionList on local_entity "
                      << statistics::to_fastdds_type(data.local_entity()) << std::endl;

            for (auto& connection : data.value().connection_list())
            {
                std::cout << "Received Connection: \n\tMode: " << connection.mode() <<
                    "\n\t Guid " << statistics::to_fastdds_type(connection.guid()) << "\n\t"
                          << "Announced Locators: ";
                for (auto& locator : connection.announced_locators())
                {
                    std::cout << "\n\t " << statistics::to_fastdds_type(locator) << std::endl;
                }
                std::cout << std::endl;
            }

            auto it = std::find_if(total_msgs.begin(), total_msgs.end(),
                            [&](const MonitorServiceType::type& elem)
                            {
                                if ((data.status_kind() == elem.status_kind()) &&
                                (data.local_entity() == elem.local_entity()))
                                {
                                    //! Check for connections
                                    bool expected_locators_found = true;
                                    for (auto& incoming_connection : data.value().connection_list())
                                    {
                                        //! Find the first Connection that matches with the incoming, with same
                                        //  mode and same locators
                                        auto conn_it = std::find_if(elem.value().connection_list().begin(),
                                        elem.value().connection_list().end(),
                                        [&](const statistics::Connection&  connection)
                                        {
                                            if (connection.mode() == incoming_connection.mode())
                                            {
                                                //! check locators
                                                bool same_locators = true;
                                                for (size_t i = 0; i < connection.announced_locators().size(); i++)
                                                {
                                                    bool locator_found = false;
                                                    for (size_t j = 0;
                                                    j < incoming_connection.announced_locators().size(); j++)
                                                    {
                                                        //! provided comparison operator
                                                        if (connection.announced_locators()[i] ==
                                                        incoming_connection.announced_locators()[j])
                                                        {
                                                            locator_found = true;
                                                        }
                                                    }

                                                    if (!locator_found)
                                                    {
                                                        same_locators = false;
                                                        break;
                                                    }
                                                }

                                                return same_locators;
                                            }
                                            else
                                            {
                                                return false;
                                            }
                                        });

                                        //! Check return connection is the last (valid connection NOT found)
                                        if (conn_it == elem.value().connection_list().end())
                                        {
                                            expected_locators_found = false;
                                            break;
                                        }
                                    }

                                    return expected_locators_found;
                                }
                                else
                                {
                                    return false;
                                }

                                return true;
                            });

            ASSERT_NE(it, total_msgs.end());

            total_msgs.erase(it);
            ++processed_count;
            cv.notify_one();
        }
    }

};

struct IncompatibleQoSSampleValidator : public SampleValidator
{
    void validate(
            SampleInfo& info,
            MonitorServiceType::type& data,
            std::list<MonitorServiceType::type>& total_msgs,
            std::atomic<size_t>& processed_count,
            std::condition_variable& cv)
    {
        if (info.valid_data
                && info.instance_state == eprosima::fastdds::dds::ALIVE_INSTANCE_STATE)
        {
            auto it = std::find_if(total_msgs.begin(), total_msgs.end(),
                            [&](const MonitorServiceType::type& elem)
                            {
                                return (data.status_kind() == elem.status_kind()) &&
                                (data.local_entity() == elem.local_entity()) &&
                                (data.value().incompatible_qos_status().last_policy_id()
                                == elem.value().incompatible_qos_status().last_policy_id());
                            });

            ASSERT_NE(it, total_msgs.end());

            std::cout << "Received QoS Incompatibility on local_entity "
                      << statistics::to_fastdds_type(data.local_entity())
                      << "\n\tLast policy id: " << data.value().incompatible_qos_status().last_policy_id()
                      << std::endl;

            total_msgs.erase(it);
            ++processed_count;
            cv.notify_one();
        }
    }

};

struct LivelinessLostSampleValidator : public SampleValidator
{
    void validate(
            SampleInfo& info,
            MonitorServiceType::type& data,
            std::list<MonitorServiceType::type>& total_msgs,
            std::atomic<size_t>& processed_count,
            std::condition_variable& cv)
    {
        if (info.valid_data
                && info.instance_state == eprosima::fastdds::dds::ALIVE_INSTANCE_STATE)
        {
            auto it = std::find_if(total_msgs.begin(), total_msgs.end(),
                            [&](const MonitorServiceType::type& elem)
                            {
                                return (data.status_kind() == elem.status_kind()) &&
                                (data.local_entity() == elem.local_entity()) &&
                                (data.value().liveliness_lost_status().total_count()
                                == elem.value().liveliness_lost_status().total_count());
                            });

            ASSERT_NE(it, total_msgs.end());

            std::cout << "Received QoS Incompatibility on local_entity "
                      << statistics::to_fastdds_type(data.local_entity())
                      << "\n\tLiveliness Lost Count: " << data.value().liveliness_lost_status().total_count()
                      << std::endl;

            total_msgs.erase(it);
            ++processed_count;
            cv.notify_one();
        }
    }

};

struct LivelinessChangedSampleValidator : public SampleValidator
{
    void validate(
            SampleInfo& info,
            MonitorServiceType::type& data,
            std::list<MonitorServiceType::type>& total_msgs,
            std::atomic<size_t>& processed_count,
            std::condition_variable& cv)
    {
        if (info.valid_data
                && info.instance_state == eprosima::fastdds::dds::ALIVE_INSTANCE_STATE)
        {
            auto it = std::find_if(total_msgs.begin(), total_msgs.end(),
                            [&](const MonitorServiceType::type& elem)
                            {
                                return (data.status_kind() == elem.status_kind()) &&
                                (data.local_entity() == elem.local_entity()) &&
                                (data.value().liveliness_changed_status().not_alive_count()
                                == elem.value().liveliness_changed_status().not_alive_count());
                            });

            ASSERT_NE(it, total_msgs.end());

            std::cout << "Received QoS Incompatibility on local_entity "
                      << statistics::to_fastdds_type(data.local_entity())
                      << "\n\tNot Alive Count: " << data.value().liveliness_changed_status().not_alive_count()
                      << std::endl;

            total_msgs.erase(it);
            ++processed_count;
            cv.notify_one();
        }
    }

};

struct DeadlineMissedSampleValidator : public SampleValidator
{
    void validate(
            SampleInfo& info,
            MonitorServiceType::type& data,
            std::list<MonitorServiceType::type>& total_msgs,
            std::atomic<size_t>& processed_count,
            std::condition_variable& cv)
    {
        if (info.valid_data
                && info.instance_state == eprosima::fastdds::dds::ALIVE_INSTANCE_STATE)
        {
            auto it = std::find_if(total_msgs.begin(), total_msgs.end(),
                            [&](const MonitorServiceType::type& elem)
                            {
                                return (data.status_kind() == elem.status_kind()) &&
                                (data.local_entity() == elem.local_entity()) &&
                                (data.value().deadline_missed_status().total_count()
                                == elem.value().deadline_missed_status().total_count());
                            });

            ASSERT_NE(it, total_msgs.end());

            std::cout << "Received Deadline Missed on local_entity "
                      << statistics::to_fastdds_type(data.local_entity())
                      << "\n\tTotal Count: " << data.value().deadline_missed_status().total_count()
                      << std::endl;

            total_msgs.erase(it);
            ++processed_count;
            cv.notify_one();
        }
    }

};

struct SampleLostSampleValidator : public SampleValidator
{
    void validate(
            SampleInfo& info,
            MonitorServiceType::type& data,
            std::list<MonitorServiceType::type>& total_msgs,
            std::atomic<size_t>& processed_count,
            std::condition_variable& cv)
    {
        if (info.valid_data
                && info.instance_state == eprosima::fastdds::dds::ALIVE_INSTANCE_STATE)
        {
            auto it = std::find_if(total_msgs.begin(), total_msgs.end(),
                            [&](const MonitorServiceType::type& elem)
                            {
                                return (data.status_kind() == elem.status_kind()) &&
                                (data.local_entity() == elem.local_entity()) &&
                                (data.value().sample_lost_status().total_count() >= 1);
                            });

            ASSERT_NE(it, total_msgs.end());

            std::cout << "Received QoS Incompatibility on local_entity "
                      << statistics::to_fastdds_type(data.local_entity())
                      << "\n\tLast policy id: " << data.value().sample_lost_status().total_count()
                      << std::endl;

            total_msgs.erase(it);
            ++processed_count;
            cv.notify_one();
        }
    }

};

void validator_selector(
        statistics::dds::DomainParticipant* participant,
        SampleValidator*& validator,
        const statistics::StatusKind status_kind,
        SampleInfo& info,
        MonitorServiceType::type& data,
        std::list<MonitorServiceType::type>& total_msgs,
        std::atomic<size_t>& processed_count,
        std::condition_variable& cv)
{
    switch (status_kind)
    {
        case statistics::StatusKind::PROXY:
        {
            auto sample_validator = static_cast<ProxySampleValidator*>(validator);
            sample_validator->validate(info, data, total_msgs, processed_count, cv, participant);
            break;
        }
        case statistics::StatusKind::CONNECTION_LIST:
        {
            auto sample_validator = static_cast<ConnectionListSampleValidator*>(validator);
            sample_validator->validate(info, data, total_msgs, processed_count, cv);
            break;
        }
        case statistics::StatusKind::INCOMPATIBLE_QOS:
        {
            auto sample_validator = static_cast<IncompatibleQoSSampleValidator*>(validator);
            sample_validator->validate(info, data, total_msgs, processed_count, cv);
            break;
        }
        case statistics::StatusKind::INCONSISTENT_TOPIC:
        {
            break;
        }
        case statistics::StatusKind::LIVELINESS_LOST:
        {
            auto sample_validator = static_cast<LivelinessLostSampleValidator*>(validator);
            sample_validator->validate(info, data, total_msgs, processed_count, cv);
            break;
        }
        case statistics::StatusKind::LIVELINESS_CHANGED:
        {
            auto sample_validator = static_cast<LivelinessChangedSampleValidator*>(validator);
            sample_validator->validate(info, data, total_msgs, processed_count, cv);
            break;
        }
        case statistics::StatusKind::DEADLINE_MISSED:
        {
            auto sample_validator = static_cast<DeadlineMissedSampleValidator*>(validator);
            sample_validator->validate(info, data, total_msgs, processed_count, cv);
            break;
        }
        case statistics::StatusKind::SAMPLE_LOST:
        {
            auto sample_validator = static_cast<SampleLostSampleValidator*>(validator);
            sample_validator->validate(info, data, total_msgs, processed_count, cv);
            break;
        }
        default:
            break;
    }
}

#endif //FASTDDS_STATISTICS

/*
 * Abbreviations
 * +--------+----------------------------+
 * | Abbr   |  Description               |
 * +--------+----------------------------+
 * | MS     | Monitor Service            |
 * +--------+----------------------------+
 * | MSC    | Monitor Service Consumer   |
 * +--------+----------------------------+
 * | MSP    | Monitor Service Participant|
 * +--------+----------------------------+
 * | MSP    | Monitor Service Topic      |
 * +--------+----------------------------+
 */

/**
 * Refers to DDS-MS-API-01 from the test plan.
 *
 * Check enable() disable() operations
 */
TEST(DDSMonitorServiceTest, monitor_service_enable_disable_api)
{
#ifdef FASTDDS_STATISTICS
    //! Setup
    MonitorServiceParticipant MSP;

    //! Procedure
    MSP.setup();

    //! Assertions
    ASSERT_EQ(ReturnCode_t::RETCODE_NOT_ENABLED, MSP.disable_monitor_service());
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, MSP.enable_monitor_service());
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, MSP.disable_monitor_service());
    ASSERT_EQ(ReturnCode_t::RETCODE_NOT_ENABLED, MSP.disable_monitor_service());
#endif //FASTDDS_STATISTICS
}

/**
 * Refers to DDS-MS-API-02 from the test plan.
 *
 * Checks fastdds.enable_monitor_service property and fastdds.statistics with the MONITOR_SERVICE_TOPIC
 */
TEST(DDSMonitorServiceTest, monitor_service_property)
{
#ifdef FASTDDS_STATISTICS
    //! Setup
    std::string xml_file = "MonitorServiceDomainParticipant_profile.xml";
    std::pair<std::string, std::string> participant_profile_names = {
        "monitor_service_property_participant", "monitor_service_statistics_property_participant" };

    MonitorServiceParticipant MSP;

    //! Procedure
    MSP.setup(xml_file, participant_profile_names.first);
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, MSP.disable_monitor_service());

    MSP.reset();
    MSP.setup(xml_file, participant_profile_names.second);

    //! Assertions
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, MSP.disable_monitor_service());
#endif //FASTDDS_STATISTICS
}

/**
 * Refers to DDS-MS-API-03 from the test plan.
 *
 * Checks that appending MONITOR_SERVICE_TOPIC reserved name
 * in FASTDDS_STATISTICS enviroment variable properly initializes
 * the service.
 */
TEST(DDSMonitorServiceTest, monitor_service_environment_variable)
{
#ifdef FASTDDS_STATISTICS
    //! Set environment variable and create participant using Qos set by code
    const char* value = "NETWORK_LATENCY_TOPIC;MONITOR_SERVICE_TOPIC";

    #ifdef _WIN32
    ASSERT_EQ(0, _putenv_s(eprosima::fastdds::statistics::dds::FASTDDS_STATISTICS_ENVIRONMENT_VARIABLE, value));
    #else
    ASSERT_EQ(0, setenv(eprosima::fastdds::statistics::dds::FASTDDS_STATISTICS_ENVIRONMENT_VARIABLE, value, 1));
    #endif // ifdef _WIN32

    //! Setup
    MonitorServiceParticipant MSP;

    //! Procedure
    MSP.setup();

    //! Assertions
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, MSP.disable_monitor_service());
#endif //FASTDDS_STATISTICS
}

/**
 * Refers to DDS-MS-API-04 from the test plan.
 *
 * Appending the fastdds.enable_monitor_service to the DomainParticipant
 * properties in the C++ API correctly creates a MSP.
 */
TEST(DDSMonitorServiceTest, monitor_service_properties_cpp_api)
{
#ifdef FASTDDS_STATISTICS
    //! Setup
    MonitorServiceParticipant MSP;

    //! Procedure
    DomainParticipantQos pqos;
    pqos.properties().properties().push_back({"fastdds.enable_monitor_service", ""});
    MSP.setup(pqos);

    //! Assertions
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, MSP.disable_monitor_service());
#endif //FASTDDS_STATISTICS
}

/**
 * Refers to DDS-MS-SIMPLE-01 from the test plan.
 *
 * A MSC correctly shall receive the corresponding proxy update in the MST after creating
 * an endpoint in a MSP
 */
TEST(DDSMonitorServiceTest, monitor_service_simple_proxy)
{
#ifdef FASTDDS_STATISTICS

    //! Setup
    MonitorServiceParticipant MSP;
    MonitorServiceConsumer MSC;

    MSP.setup();

    //! Procedure
    MSC.init_monitor_service_reader();
    MSP.enable_monitor_service();

    std::list<MonitorServiceType::type> expected_msgs;

    MonitorServiceType::type participant_proxy_msg, writer_proxy_msg;

    participant_proxy_msg.status_kind(eprosima::fastdds::statistics::PROXY);
    participant_proxy_msg.local_entity(MSP.get_participant_guid());

    expected_msgs.push_back(participant_proxy_msg);

    MSP.create_and_add_writer();

    writer_proxy_msg.status_kind(eprosima::fastdds::statistics::PROXY);
    StatisticsGUIDList guids = MSP.get_writer_guids();

    ASSERT_EQ(guids.size(), 1);
    writer_proxy_msg.local_entity(guids.back());

    expected_msgs.push_back(writer_proxy_msg);

    MSC.start_reception(expected_msgs);

    //! Assertions
    //! The assertion checking whether the on_data_availble() was called is assumed in the following one
    ASSERT_EQ(MSC.block_for_all(std::chrono::seconds(3)), expected_msgs.size());
#endif //FASTDDS_STATISTICS
}

/**
 * Refers to DDS-MS-SIMPLE-02 from the test plan.
 *
 * A MSC correctly shall receive the corresponding connection list update in the MST
 * after creating two MSP with a pair of matched endpoints.
 *
 * TODO: Extend the connection list test cases that checks for intraprocess and datasharing
 * connection modes
 *
 */
TEST_P(DDSMonitorServiceTest, monitor_service_simple_connection_list)
{
#ifdef FASTDDS_STATISTICS
    //! Setup
    MonitorServiceParticipant MSP1, MSP2;
    MonitorServiceConsumer MSC;

    //! Procedure
    MSC.init_monitor_service_reader();

    std::string xml_profile = "MonitorServiceConnectionList_profile.xml";
    std::pair<std::string, std::string> participant_profiles =
    {"monitor_service_connections_list_participant_1", "monitor_service_connections_list_participant_2"};

    MSP1.setup(xml_profile, participant_profiles.first);
    MSP2.setup(xml_profile, participant_profiles.second);

    MSP1.enable_monitor_service();
    MSP2.enable_monitor_service();

    std::list<MonitorServiceType::type> expected_msgs;

    MonitorServiceType::type participant_connection_msg, endpoint_connections_msg;

    participant_connection_msg.status_kind(eprosima::fastdds::statistics::CONNECTION_LIST);
    participant_connection_msg.local_entity(MSP1.get_participant_guid());

    std::vector<statistics::detail::Locator_s> locators;
    statistics::detail::Locator_s loc;

    //! For the participant, assert the unicast locators
    constexpr const char* LOCAL_ADDRESS = "127.0.0.1";
    constexpr const char* MULTICAST_ADDRESS = "239.255.0.1";
    Locator_t metatraffic_unicast_local_addr_locator(LOCATOR_KIND_UDPv4, 7399);
    IPLocator::setIPv4(metatraffic_unicast_local_addr_locator, LOCAL_ADDRESS);
    Locator_t default_unicast_local_addr_locator(LOCATOR_KIND_UDPv4, 2020);
    IPLocator::setIPv4(default_unicast_local_addr_locator, LOCAL_ADDRESS);
    Locator_t metatraffic_multicast_addr_locator(LOCATOR_KIND_UDPv4, 7400);
    IPLocator::setIPv4(metatraffic_multicast_addr_locator, MULTICAST_ADDRESS);

    locators.push_back(statistics::to_statistics_type(metatraffic_unicast_local_addr_locator));
    locators.push_back(statistics::to_statistics_type(default_unicast_local_addr_locator));
    locators.push_back(statistics::to_statistics_type(metatraffic_multicast_addr_locator));

    std::vector<statistics::Connection> connection_list;
    statistics::Connection conn;
    conn.guid() = MSP2.get_participant_guid();
    conn.mode() = statistics::ConnectionMode::TRANSPORT;
    conn.announced_locators(locators);
    connection_list.push_back(conn);

    participant_connection_msg.value().connection_list(connection_list);

    expected_msgs.push_back(participant_connection_msg);

    participant_connection_msg.status_kind(eprosima::fastdds::statistics::CONNECTION_LIST);
    participant_connection_msg.local_entity(MSP2.get_participant_guid());

    ASSERT_FALSE(participant_connection_msg.value().connection_list().empty());
    ASSERT_EQ(participant_connection_msg.value().connection_list().back().announced_locators().size(), 3u);

    participant_connection_msg.value().connection_list().back().guid() = MSP1.get_participant_guid();
    participant_connection_msg.value().connection_list().back().mode() = statistics::ConnectionMode::TRANSPORT;
    participant_connection_msg.value().connection_list().back().announced_locators()[0].port() = 7398;//unicast
    participant_connection_msg.value().connection_list().back().announced_locators()[1].port() = 2019;//unicast

    expected_msgs.push_back(participant_connection_msg);

    MSP1.create_and_add_writer();
    MSP2.create_and_add_reader();

    StatisticsGUIDList w_guids, r_guids;
    endpoint_connections_msg.status_kind(eprosima::fastdds::statistics::CONNECTION_LIST);
    w_guids = MSP1.get_writer_guids();
    ASSERT_EQ(w_guids.size(), 1);
    r_guids = MSP2.get_reader_guids();
    ASSERT_EQ(r_guids.size(), 1);

    endpoint_connections_msg.local_entity(w_guids.back());
    endpoint_connections_msg.value().connection_list(connection_list);
    endpoint_connections_msg.value().connection_list().back().guid() = r_guids.back();
    endpoint_connections_msg.value().connection_list().back().mode() = statistics::ConnectionMode::TRANSPORT;
    endpoint_connections_msg.value().connection_list().back().announced_locators()[0].port() = 2020;//unicast
    endpoint_connections_msg.value().connection_list().back().announced_locators()[1].port() = 2022;//multicast

    expected_msgs.push_back(endpoint_connections_msg);

    endpoint_connections_msg.local_entity(r_guids.back());
    endpoint_connections_msg.value().connection_list().back().guid() = w_guids.back();
    endpoint_connections_msg.value().connection_list().back().mode() = statistics::ConnectionMode::TRANSPORT;
    endpoint_connections_msg.value().connection_list().back().announced_locators()[0].port() = 2019;//unicast
    endpoint_connections_msg.value().connection_list().back().announced_locators()[1].port() = 2021;//multicast

    expected_msgs.push_back(endpoint_connections_msg);

    MSC.start_reception(expected_msgs);

    //! Assertions
    //! The assertion checking whether the on_data_availble() was called is assumed in the following one
    ASSERT_EQ(MSC.block_for_all(std::chrono::seconds(3)), expected_msgs.size());
#endif //FASTDDS_STATISTICS
}

/**
 * Refers to DDS-MS-SIMPLE-03 from the test plan.
 *
 * MSC correctly receives a QoS incompatibility after adding a pair of reader/writer
 */
TEST(DDSMonitorServiceTest, monitor_service_simple_qos_incompatibility_status)
{
#ifdef FASTDDS_STATISTICS
    //! Setup
    MonitorServiceParticipant MSP;
    MonitorServiceConsumer MSC;

    //! Procedure
    MSC.init_monitor_service_reader();

    MSP.setup();

    MSP.enable_monitor_service();

    DataReaderQos dr_qos;
    DataWriterQos dw_qos;

    dr_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    dw_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;

    MSP.create_and_add_writer(dw_qos);
    MSP.create_and_add_reader(dr_qos);

    std::list<MonitorServiceType::type> expected_msgs;
    MonitorServiceType::type endpoint_qos_msg;
    StatisticsGUIDList w_guids, r_guids;

    endpoint_qos_msg.status_kind(eprosima::fastdds::statistics::INCOMPATIBLE_QOS);
    w_guids = MSP.get_writer_guids();
    ASSERT_EQ(w_guids.size(), 1);
    endpoint_qos_msg.local_entity(w_guids.back());

    statistics::IncompatibleQoSStatus_s incompatible_qos;
    statistics::QosPolicyCount_s policy;
    policy.policy_id(RELIABILITY_QOS_POLICY_ID);
    incompatible_qos.policies().push_back(policy);
    endpoint_qos_msg.value().incompatible_qos_status(incompatible_qos);

    expected_msgs.push_back(endpoint_qos_msg);

    endpoint_qos_msg.status_kind(eprosima::fastdds::statistics::INCOMPATIBLE_QOS);
    r_guids = MSP.get_reader_guids();
    ASSERT_EQ(r_guids.size(), 1);
    endpoint_qos_msg.local_entity(r_guids.back());

    expected_msgs.push_back(endpoint_qos_msg);

    MSC.start_reception(expected_msgs);
    //! Assertions
    //! The assertion checking whether the on_data_availble() was called is assumed in the following one
    ASSERT_EQ(MSC.block_for_all(std::chrono::seconds(3)), expected_msgs.size());
#endif //FASTDDS_STATISTICS
}

/**
 * Refers to DDS-MS-SIMPLE-04 from the test plan.
 *
 * To implement when InconsistentTopciStatus is fully supported

   TEST(DDSMonitorServiceTest, monitor_service_simple_inconsistent_topic)
   {
   }
 */

/**
 * Refers to DDS-MS-SIMPLE-05 from the test plan.
 *
 * The lease duration of a writer created in the MSP expires and the liveliness lost status
 * is correctly notified to the MSC
 */
TEST(DDSMonitorServiceTest, monitor_service_simple_liveliness_lost_status)
{
#ifdef FASTDDS_STATISTICS
    //! Setup
    MonitorServiceParticipant MSP;
    MonitorServiceConsumer MSC;

    //! Procedure
    MSC.init_monitor_service_reader();

    MSP.setup();

    MSP.enable_monitor_service();

    DataWriterQos dw_qos;

    dw_qos.liveliness().kind = eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS;
    dw_qos.liveliness().lease_duration = eprosima::fastrtps::Time_t{1, 0};

    MSP.create_and_add_writer(dw_qos);
    MSP.assert_liveliness();

    std::list<MonitorServiceType::type> expected_msgs;
    MonitorServiceType::type endpoint_liveliness_msg;
    StatisticsGUIDList w_guids;

    endpoint_liveliness_msg.status_kind(eprosima::fastdds::statistics::LIVELINESS_LOST);
    w_guids = MSP.get_writer_guids();
    ASSERT_EQ(w_guids.size(), 1);
    endpoint_liveliness_msg.local_entity(w_guids.back());

    statistics::LivelinessLostStatus_s liv_lost_status;
    liv_lost_status.total_count() = 1;
    endpoint_liveliness_msg.value().liveliness_lost_status(liv_lost_status);

    expected_msgs.push_back(endpoint_liveliness_msg);

    MSC.start_reception(expected_msgs);

    //! Assertions
    //! The assertion checking whether the on_data_availble() was called is assumed in the following one
    ASSERT_EQ(MSC.block_for_all(std::chrono::seconds(3)), expected_msgs.size());
    ASSERT_TRUE(MSP.get_cb_count(MonitorServiceParticipant::LIVELINESS_LOST_IDX) > 0);
#endif //FASTDDS_STATISTICS
}

/**
 * Refers to DDS-MS-SIMPLE-06 from the test plan.
 *
 * In a MSP, the liveliness of a reader changes and the status is correctly notified to the
 * MSC.
 */
TEST(DDSMonitorServiceTest, monitor_service_simple_liveliness_changed_status)
{
#ifdef FASTDDS_STATISTICS
    //! Setup
    MonitorServiceParticipant MSP;
    MonitorServiceConsumer MSC;

    //! Procedure
    MSC.init_monitor_service_reader();

    MSP.setup();

    MSP.enable_monitor_service();

    DataReaderQos dr_qos;

    dr_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    dr_qos.liveliness().kind = eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS;
    dr_qos.liveliness().lease_duration = eprosima::fastrtps::Time_t{1, 0};

    DataWriterQos dw_qos;

    dw_qos.liveliness().kind = eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS;
    dw_qos.liveliness().lease_duration = eprosima::fastrtps::Time_t{1, 0};

    MSP.create_and_add_reader(dr_qos);
    MSP.create_and_add_writer(dw_qos);

    MSP.assert_liveliness();

    std::list<MonitorServiceType::type> expected_msgs;
    MonitorServiceType::type endpoint_liveliness_msg;
    StatisticsGUIDList r_guids;

    endpoint_liveliness_msg.status_kind(eprosima::fastdds::statistics::LIVELINESS_CHANGED);
    r_guids = MSP.get_reader_guids();
    ASSERT_EQ(r_guids.size(), 1);
    endpoint_liveliness_msg.local_entity(r_guids.back());

    statistics::LivelinessChangedStatus_s liv_changed_status;
    liv_changed_status.not_alive_count() = 1;
    endpoint_liveliness_msg.value().liveliness_changed_status(liv_changed_status);

    expected_msgs.push_back(endpoint_liveliness_msg);

    MSC.start_reception(expected_msgs);

    //! Assertions
    //! The assertion checking whether the on_data_availble() was called is assumed in the following one
    ASSERT_EQ(MSC.block_for_all(std::chrono::seconds(5)), expected_msgs.size());
    ASSERT_TRUE(MSP.get_cb_count(MonitorServiceParticipant::LIVELINESS_CHANGED_IDX) > 0);
#endif //FASTDDS_STATISTICS
}

/**
 * Refers to DDS-MS-SIMPLE-07 from the test plan.
 *
 * The deadline is forced to be missed in an entity of the MSP and the status is correctly
 * notified to the MSC.
 */
TEST(DDSMonitorServiceTest, monitor_service_simple_deadline_missed_status)
{
#ifdef FASTDDS_STATISTICS
    //! Setup
    MonitorServiceParticipant MSP;
    MonitorServiceConsumer MSC;

    //! Procedure
    MSC.init_monitor_service_reader();

    MSP.setup();
    MSP.enable_monitor_service();

    DataReaderQos dr_qos;
    DataWriterQos dw_qos;

    dr_qos.deadline().period = eprosima::fastrtps::Time_t{1, 0};
    dw_qos.deadline().period = eprosima::fastrtps::Time_t{1, 0};

    MSP.create_and_add_reader(dr_qos);
    MSP.create_and_add_writer(dw_qos);

    std::list<MonitorServiceType::type> expected_msgs;
    MonitorServiceType::type endpoint_deadline_msg;
    StatisticsGUIDList r_guids, w_guids;

    endpoint_deadline_msg.status_kind(eprosima::fastdds::statistics::DEADLINE_MISSED);
    r_guids = MSP.get_reader_guids();
    ASSERT_EQ(r_guids.size(), 1);
    endpoint_deadline_msg.local_entity(r_guids.back());

    statistics::DeadlineMissedStatus_s deadline_missed_status;
    deadline_missed_status.total_count() = 1;
    endpoint_deadline_msg.value().deadline_missed_status(deadline_missed_status);

    expected_msgs.push_back(endpoint_deadline_msg);

    w_guids = MSP.get_writer_guids();
    ASSERT_EQ(w_guids.size(), 1);
    endpoint_deadline_msg.local_entity(w_guids.back());

    expected_msgs.push_back(endpoint_deadline_msg);

    MSC.start_reception(expected_msgs);

    HelloWorld hello;
    ASSERT_TRUE(MSP.write_sample(hello));


    //! Assertions
    //! The assertion checking whether the on_data_availble() was called is assumed in the following one
    ASSERT_EQ(MSC.block_for_all(std::chrono::seconds(3)), expected_msgs.size());
    ASSERT_TRUE(MSP.get_cb_count(MonitorServiceParticipant::OFFERED_DEADLINE_MISSED_IDX) > 0);
    ASSERT_TRUE(MSP.get_cb_count(MonitorServiceParticipant::REQUESTED_DEADLINE_MISSED_IDX) > 0);
#endif //FASTDDS_STATISTICS
}

/**
 * Refers to DDS-MS-SIMPLE-08 from the test plan.
 *
 * Making use of the test transport, force loosing a sample in the MSP so that the lost
 * sample status is correctly notified to the MSC.
 */
TEST(DDSMonitorServiceTest, monitor_service_simple_sample_lost_status)
{
#ifdef FASTDDS_STATISTICS
    //! Setup
    MonitorServiceParticipant MSP1, MSP2;
    MonitorServiceConsumer MSC;

    //! Procedure
    DomainParticipantQos dqos;

    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->drop_data_messages_filter_ = [](eprosima::fastrtps::rtps::CDRMessage_t& msg)-> bool
            {
                uint32_t old_pos = msg.pos;

                EntityId_t readerID, writerID;
                SequenceNumber_t sn;

                msg.pos += 2; // flags
                msg.pos += 2; // octets to inline qos
                CDRMessage::readEntityId(&msg, &readerID);
                CDRMessage::readEntityId(&msg, &writerID);
                CDRMessage::readSequenceNumber(&msg, &sn);

                // restore buffer pos
                msg.pos = old_pos;

                // generate losses
                if ((writerID.value[3] & 0xC0) == 0 // only user endpoints
                        && (sn == SequenceNumber_t{0, 2} ||
                        sn == SequenceNumber_t(0, 3) ||
                        sn == SequenceNumber_t(0, 4) ||
                        sn == SequenceNumber_t(0, 6) ||
                        sn == SequenceNumber_t(0, 8) ||
                        sn == SequenceNumber_t(0, 10) ||
                        sn == SequenceNumber_t(0, 11) ||
                        sn == SequenceNumber_t(0, 13)))
                {
                    return true;
                }

                return false;
            };

    dqos.transport().use_builtin_transports = false;
    dqos.transport().user_transports.push_back(testTransport);

    MSC.init_monitor_service_reader();

    MSP1.setup(dqos);
    MSP1.enable_monitor_service();

    MSP2.setup();
    MSP2.enable_monitor_service();

    DataReaderQos dr_qos;
    DataWriterQos dw_qos;

    MSP1.create_and_add_writer(dw_qos);
    MSP2.create_and_add_reader(dr_qos);

    std::list<MonitorServiceType::type> expected_msgs;
    MonitorServiceType::type endpoint_sample_lost_msg;
    StatisticsGUIDList r_guids;

    endpoint_sample_lost_msg.status_kind(eprosima::fastdds::statistics::SAMPLE_LOST);
    r_guids = MSP2.get_reader_guids();
    ASSERT_EQ(r_guids.size(), 1);
    endpoint_sample_lost_msg.local_entity(r_guids.back());

    expected_msgs.push_back(endpoint_sample_lost_msg);

    MSC.start_reception(expected_msgs);

    auto data = default_helloworld_data_generator();
    MSP1.send(data, 50);


    //! Assertions
    //! The assertion checking whether the on_data_availble() was called is assumed in the following one
    ASSERT_EQ(MSC.block_for_all(std::chrono::seconds(5)), expected_msgs.size());
    ASSERT_TRUE(MSP2.get_cb_count(MonitorServiceParticipant::SAMPLE_LOST_IDX) > 0);
#endif //FASTDDS_STATISTICS
}

/**
 * Refers to DDS-MS-SIMPLE-09 from the test plan.
 *
 * Removing a previously created endpoint in the MSP makes MSC to receive the corre-
 * sponding instance disposals.
 */
TEST(DDSMonitorServiceTest, monitor_service_simple_instance_disposals)
{
#ifdef FASTDDS_STATISTICS
    //! Setup
    MonitorServiceParticipant MSP;
    MonitorServiceConsumer MSC;

    MSP.setup();

    //! Procedure
    MSC.init_monitor_service_reader();
    MSP.enable_monitor_service();

    MSP.create_and_add_writer();

    std::list<MonitorServiceType::type> expected_msgs;

    MonitorServiceType::type writer_proxy_msg;
    expected_msgs.push_back(writer_proxy_msg);

    MSC.start_reception(expected_msgs);

    MSP.delete_writer();

    //! Assertions
    //! The assertion checking whether the on_data_availble() was called is assumed in the following one
    ASSERT_EQ(MSC.block_for_all(std::chrono::seconds(3)), expected_msgs.size());

#endif //FASTDDS_STATISTICS
}

/**
 * Refers to DDS-MS-SIMPLE-10 from the test plan.
 *
 * MSC late joins a MSP with an already enabled MS.
 */
TEST(DDSMonitorServiceTest, monitor_service_simple_late_joiner)
{
#ifdef FASTDDS_STATISTICS
    //! Setup
    MonitorServiceParticipant MSP;
    MonitorServiceConsumer MSC;

    MSP.setup();

    //! Procedure
    MSP.enable_monitor_service();

    std::list<MonitorServiceType::type> expected_msgs;

    MonitorServiceType::type participant_proxy_msg, entity_proxy_msg;

    participant_proxy_msg.status_kind(eprosima::fastdds::statistics::PROXY);
    participant_proxy_msg.local_entity(MSP.get_participant_guid());

    expected_msgs.push_back(participant_proxy_msg);

    MSP.create_and_add_writer();
    MSP.create_and_add_reader();

    entity_proxy_msg.status_kind(eprosima::fastdds::statistics::PROXY);
    StatisticsGUIDList w_guids = MSP.get_writer_guids();

    ASSERT_EQ(w_guids.size(), 1);
    entity_proxy_msg.local_entity(w_guids.back());

    expected_msgs.push_back(entity_proxy_msg);

    entity_proxy_msg.status_kind(eprosima::fastdds::statistics::PROXY);
    StatisticsGUIDList r_guids = MSP.get_reader_guids();

    ASSERT_EQ(r_guids.size(), 1);
    entity_proxy_msg.local_entity(r_guids.back());

    expected_msgs.push_back(entity_proxy_msg);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    MSC.init_monitor_service_reader();
    MSC.start_reception(expected_msgs);

    //! Assertions
    //! The assertion checking whether the on_data_availble() was called is assumed in the following one
    ASSERT_EQ(MSC.block_for_all(std::chrono::seconds(3)), expected_msgs.size());

#endif //FASTDDS_STATISTICS
}

/**
 * Refers to DDS-MS-SIMPLE-11 from the test plan.
 *
 * Enabling the MS, disabling it, making some updates and re-enabling it shall correctly
 * behave.
 */
TEST(DDSMonitorServiceTest, monitor_service_simple_enable_disable_enable)
{
#ifdef FASTDDS_STATISTICS
    //! Setup
    MonitorServiceParticipant MSP;
    MonitorServiceConsumer MSC;

    //! Procedure
    MSP.setup();

    MSC.init_monitor_service_reader();
    MSP.enable_monitor_service();

    MSP.create_and_add_writer();
    MSP.disable_monitor_service();

    MSP.create_and_add_writer();
    MSP.create_and_add_reader();

    std::list<MonitorServiceType::type> expected_msgs;

    MonitorServiceType::type participant_proxy_msg, entity_proxy_msg;

    participant_proxy_msg.status_kind(eprosima::fastdds::statistics::PROXY);
    participant_proxy_msg.local_entity(MSP.get_participant_guid());

    expected_msgs.push_back(participant_proxy_msg);

    entity_proxy_msg.status_kind(eprosima::fastdds::statistics::PROXY);
    StatisticsGUIDList w_guids = MSP.get_writer_guids();
    StatisticsGUIDList r_guids = MSP.get_reader_guids();

    ASSERT_EQ(w_guids.size(), 2);
    entity_proxy_msg.local_entity(w_guids.front());
    expected_msgs.push_back(entity_proxy_msg);
    entity_proxy_msg.local_entity(w_guids.back());
    expected_msgs.push_back(entity_proxy_msg);

    ASSERT_EQ(r_guids.size(), 1);
    entity_proxy_msg.local_entity(r_guids.back());

    expected_msgs.push_back(entity_proxy_msg);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    MSP.enable_monitor_service();

    MSC.start_reception(expected_msgs);

    //! Assertions
    //! The assertion checking whether the on_data_availble() was called is assumed in the following one
    ASSERT_EQ(MSC.block_for_all(std::chrono::seconds(3)), expected_msgs.size());
#endif //FASTDDS_STATISTICS
}

/**
 * Refers to DDS-MS-ADV-01 from the test plan.
 *
 * A MSC shall correctly receive the corresponding proxies from different MSPs.
 */
TEST(DDSMonitorServiceTest, monitor_service_advanced_proxy)
{
#ifdef FASTDDS_STATISTICS
    //! Setup
    size_t n_participants = 3;
    std::vector<MonitorServiceParticipant> MSPs;
    MSPs.reserve(n_participants);

    MonitorServiceConsumer MSC;

    //! Procedure
    MSC.init_monitor_service_reader();

    std::list<MonitorServiceType::type> expected_msgs;

    for (size_t i = 0; i < n_participants; i++)
    {
        MonitorServiceParticipant MSP;

        MSP.setup();
        MSP.enable_monitor_service();

        if (i != 0)
        {
            MSP.create_topic((int)i);
        }

        MSP.create_and_add_writer();
        MSP.create_and_add_reader();

        MonitorServiceType::type participant_proxy_msg, entity_proxy_msg;

        participant_proxy_msg.status_kind(eprosima::fastdds::statistics::PROXY);
        participant_proxy_msg.local_entity(MSP.get_participant_guid());

        expected_msgs.push_back(participant_proxy_msg);

        entity_proxy_msg.status_kind(eprosima::fastdds::statistics::PROXY);
        StatisticsGUIDList w_guids = MSP.get_writer_guids();
        StatisticsGUIDList r_guids = MSP.get_reader_guids();

        ASSERT_EQ(w_guids.size(), 1);
        entity_proxy_msg.local_entity(w_guids.back());
        expected_msgs.push_back(entity_proxy_msg);

        ASSERT_EQ(r_guids.size(), 1);
        entity_proxy_msg.local_entity(r_guids.back());

        expected_msgs.push_back(entity_proxy_msg);
        MSPs.emplace_back(std::move(MSP));
    }

    MSC.start_reception(expected_msgs);

    //! Assertions
    //! The assertion checking whether the on_data_availble() was called is assumed in the following one
    ASSERT_EQ(MSC.block_for_all(std::chrono::seconds(10)), expected_msgs.size());
#endif //FASTDDS_STATISTICS
}

/**
 * Refers to DDS-MS-ADV-02 from the test plan.
 *
 * A MSC shall correctly receive the corresponding instance disposals after deleting one
 * of the MSP.
 */
TEST(DDSMonitorServiceTest, monitor_service_advanced_instance_disposals)
{
#ifdef FASTDDS_STATISTICS
    //! Setup
    size_t n_participants = 3;
    std::vector<MonitorServiceParticipant> MSPs;
    MSPs.reserve(n_participants);

    MonitorServiceConsumer MSC;

    //! Procedure
    MSC.init_monitor_service_reader();

    for (size_t i = 0; i < n_participants; i++)
    {
        MonitorServiceParticipant MSP;

        MSP.setup();
        MSP.enable_monitor_service();

        if (i != 0)
        {
            MSP.create_topic((int)i);
        }

        MSP.create_and_add_writer();
        MSP.create_and_add_reader();

        MSPs.emplace_back(std::move(MSP));
    }

    MSPs.back().reset();
    MSPs.back().reset();

    //! Expect instance disposals from proxy and connection list of
    //! 4 instances = 2 participants proxies + 2 participant connection lists
    //! 4 instances = 2 readers proxies + 2 reader connection lists
    //! 4 instances = 2 writer proxies + 2 writer connection lists
    //! 12 = 2 statuses * 3 entities * 3 participants removed
    std::list<MonitorServiceType::type> expected_msgs{2 * 3 * 3, MonitorServiceType::type()};
    MSC.start_reception(expected_msgs);


    //! Assertions
    //! The assertion checking whether the on_data_availble() was called is assumed in the following one
    ASSERT_EQ(MSC.block_for_all(std::chrono::seconds(3)), expected_msgs.size());
#endif //FASTDDS_STATISTICS
}

/**
 * Refers to DDS-MS-ADV-03 from the test plan.
 *
 * A MSC shall correctly receive the updates after late joining.
 */
TEST(DDSMonitorServiceTest, monitor_service_advanced_single_late_joiner)
{
#ifdef FASTDDS_STATISTICS
    //! Setup
    size_t n_participants = 3;
    std::vector<MonitorServiceParticipant> MSPs;
    MSPs.reserve(n_participants);
    MonitorServiceConsumer MSC;

    //! Procedure
    std::list<MonitorServiceType::type> expected_msgs;

    for (size_t i = 0; i < n_participants; i++)
    {
        MonitorServiceParticipant MSP;

        MSP.setup();
        MSP.enable_monitor_service();

        DataReaderQos dr_qos;
        DataWriterQos dw_qos;

        dr_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        dw_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;

        if (i != 0)
        {
            MSP.create_topic((int)i);
        }

        MSP.create_and_add_writer(dw_qos);
        MSP.create_and_add_reader(dr_qos);


        MonitorServiceType::type endpoint_qos_msg;
        StatisticsGUIDList w_guids, r_guids;

        endpoint_qos_msg.status_kind(eprosima::fastdds::statistics::INCOMPATIBLE_QOS);
        w_guids = MSP.get_writer_guids();
        ASSERT_EQ(w_guids.size(), 1);
        endpoint_qos_msg.local_entity(w_guids.back());

        statistics::IncompatibleQoSStatus_s incompatible_qos;
        statistics::QosPolicyCount_s policy;
        policy.policy_id(RELIABILITY_QOS_POLICY_ID);
        incompatible_qos.policies().push_back(policy);
        endpoint_qos_msg.value().incompatible_qos_status(incompatible_qos);

        expected_msgs.push_back(endpoint_qos_msg);

        endpoint_qos_msg.status_kind(eprosima::fastdds::statistics::INCOMPATIBLE_QOS);
        r_guids = MSP.get_reader_guids();
        ASSERT_EQ(r_guids.size(), 1);
        endpoint_qos_msg.local_entity(r_guids.back());

        expected_msgs.push_back(endpoint_qos_msg);

        MSPs.emplace_back(std::move(MSP));
    }

    std::this_thread::sleep_for(std::chrono::seconds(10));
    MSC.init_monitor_service_reader();
    MSC.start_reception(expected_msgs);

    //! Assertions
    //! The assertion checking whether the on_data_availble() was called is assumed in the following one
    ASSERT_EQ(MSC.block_for_all(std::chrono::seconds(10)), expected_msgs.size());
#endif //FASTDDS_STATISTICS
}

/**
 * Refers to DDS-MS-ADV-04 from the test plan.
 *
 * Multiple MSC shall correctly receive the updates after late joining.
 */
TEST(DDSMonitorServiceTest, monitor_service_advanced_multiple_late_joiners)
{
#ifdef FASTDDS_STATISTICS
    //! Setup
    MonitorServiceParticipant MSP;
    size_t n_participants = 3;

    //! Procedure
    std::list<MonitorServiceType::type> expected_msgs;

    MSP.setup();
    MSP.enable_monitor_service();

    DataReaderQos dr_qos;
    DataWriterQos dw_qos;

    dr_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    dw_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;

    MSP.create_topic(0);

    MSP.create_and_add_writer(dw_qos);
    MSP.create_and_add_reader(dr_qos);

    MonitorServiceType::type endpoint_qos_msg;
    StatisticsGUIDList w_guids, r_guids;

    endpoint_qos_msg.status_kind(eprosima::fastdds::statistics::INCOMPATIBLE_QOS);
    w_guids = MSP.get_writer_guids();
    ASSERT_EQ(w_guids.size(), 1);
    endpoint_qos_msg.local_entity(w_guids.back());

    statistics::IncompatibleQoSStatus_s incompatible_qos;
    statistics::QosPolicyCount_s policy;
    policy.policy_id(RELIABILITY_QOS_POLICY_ID);
    incompatible_qos.policies().push_back(policy);
    endpoint_qos_msg.value().incompatible_qos_status(incompatible_qos);

    expected_msgs.push_back(endpoint_qos_msg);

    endpoint_qos_msg.status_kind(eprosima::fastdds::statistics::INCOMPATIBLE_QOS);
    r_guids = MSP.get_reader_guids();
    ASSERT_EQ(r_guids.size(), 1);
    endpoint_qos_msg.local_entity(r_guids.back());

    expected_msgs.push_back(endpoint_qos_msg);

    std::this_thread::sleep_for(std::chrono::seconds(10));

    for (size_t i = 0; i < n_participants; i++)
    {
        MonitorServiceConsumer MSC;

        MSC.init_monitor_service_reader();
        MSC.start_reception(expected_msgs);

        //! Assertions
        //! The assertion checking whether the on_data_availble() was called is assumed in the following one
        ASSERT_EQ(MSC.block_for_all(std::chrono::seconds(10)), expected_msgs.size());
    }
#endif //FASTDDS_STATISTICS
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(DDSMonitorServiceTest,
        DDSMonitorServiceTest,
        testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const testing::TestParamInfo<DDSMonitorServiceTest::ParamType>& info)
        {
            switch (info.param)
            {
                case INTRAPROCESS:
                    return "Intraprocess";
                    break;
                case DATASHARING:
                    return "Datasharing";
                    break;
                case TRANSPORT:
                default:
                    return "Transport";
            }
        });
