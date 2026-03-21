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

#include <array>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fastdds/dds/publisher/DataWriter.hpp>

#include <statistics/fastdds/domain/DomainParticipantStatisticsListener.hpp>
#include <statistics/types/types.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

class DomainParticipantStatisticsListenerTests : public ::testing::Test
{
protected:

    using DataWriter = eprosima::fastdds::dds::DataWriter;

    void SetUp() override
    {
        data_[0].writer_reader_data({});
        data_to_check_[0] = &data_[0].writer_reader_data();

        data_[1].locator2locator_data({});
        data_to_check_[1] = &data_[1].locator2locator_data();

        data_[2].entity_data({});
        data_to_check_[2] = &data_[2].entity_data();

        data_[3].entity_data({});
        data_to_check_[3] = &data_[3].entity_data();

        data_[4].entity2locator_traffic({});
        data_to_check_[4] = &data_[4].entity2locator_traffic();

        data_[5].entity2locator_traffic({});
        data_to_check_[5] = &data_[5].entity2locator_traffic();

        data_[6].entity_count({});
        data_to_check_[6] = &data_[6].entity_count();

        data_[7].entity_count({});
        data_to_check_[7] = &data_[7].entity_count();

        data_[8].entity_count({});
        data_to_check_[8] = &data_[8].entity_count();

        data_[9].entity_count({});
        data_to_check_[9] = &data_[9].entity_count();

        data_[10].entity_count({});
        data_to_check_[10] = &data_[10].entity_count();

        data_[11].entity_count({});
        data_to_check_[11] = &data_[11].entity_count();

        data_[12].entity_count({});
        data_to_check_[12] = &data_[12].entity_count();

        data_[13].entity_count({});
        data_to_check_[13] = &data_[13].entity_count();

        data_[14].discovery_time({});
        data_to_check_[14] = &data_[14].discovery_time();

        data_[15].sample_identity_count({});
        data_to_check_[15] = &data_[15].sample_identity_count();

        data_[16].physical_data({});
        data_to_check_[16] = &data_[16].physical_data();

        for (size_t i = 0; i < kinds_.size(); i++)
        {
            data_[i]._d(kinds_[i]);
        }
    }

    void expect_all_writers(
            int num_times)
    {
        for (size_t i = 0; i < writers_.size(); i++)
        {
            EXPECT_CALL(writers_[i], write(data_to_check_[i])).Times(num_times);
        }
    }

    void expect_no_writes()
    {
        for (auto& writer : writers_)
        {
            EXPECT_CALL(writer, write).Times(0);
        }
    }

    void expect_no_writes_except(
            size_t n)
    {
        for (size_t i = 0; i < writers_.size(); i++)
        {
            if (i == n)
            {
                EXPECT_CALL(writers_[i], write(data_to_check_[i])).Times(1);
            }
            else
            {
                EXPECT_CALL(writers_[i], write).Times(0);
            }
        }
    }

    void write_all_data()
    {
        for (const Data& data : data_)
        {
            listener_.on_statistics_data(data);
        }
    }

    void write_all_data_except(
            uint32_t kind)
    {
        for (const Data& data : data_)
        {
            if (data._d() != kind)
            {
                listener_.on_statistics_data(data);
            }
        }
    }

    void check_and_reset_expectations()
    {
        for (auto& writer : writers_)
        {
            testing::Mock::VerifyAndClearExpectations(&writer);
        }
    }

    std::array<testing::StrictMock<DataWriter>, 17> writers_;
    std::array<void*, 17> data_to_check_;
    std::array<Data, 17> data_;
    std::array<uint32_t, 17> kinds_ =
    {
        EventKind::HISTORY2HISTORY_LATENCY,
        EventKind::NETWORK_LATENCY,
        EventKind::PUBLICATION_THROUGHPUT,
        EventKind::SUBSCRIPTION_THROUGHPUT,
        EventKind::RTPS_SENT,
        EventKind::RTPS_LOST,
        EventKind::RESENT_DATAS,
        EventKind::HEARTBEAT_COUNT,
        EventKind::ACKNACK_COUNT,
        EventKind::NACKFRAG_COUNT,
        EventKind::GAP_COUNT,
        EventKind::DATA_COUNT,
        EventKind::PDP_PACKETS,
        EventKind::EDP_PACKETS,
        EventKind::DISCOVERED_ENTITY,
        EventKind::SAMPLE_DATAS,
        EventKind::PHYSICAL_DATA
    };

    DomainParticipantStatisticsListener listener_;
};

// Check that write calls are not performed when writers are not added to the listener
TEST_F(DomainParticipantStatisticsListenerTests, no_writers)
{
    expect_no_writes();
    write_all_data();
}

// Check that every writer receives one call when all of them have been set
TEST_F(DomainParticipantStatisticsListenerTests, all_writers)
{
    expect_all_writers(1);
    for (size_t i = 0; i < writers_.size(); ++i)
    {
        listener_.set_datawriter(kinds_[i], &writers_[i]);
    }
    write_all_data();
}

// Setting one writer and sending all kinds of data only writes on the writer set
TEST_F(DomainParticipantStatisticsListenerTests, single_writer_all_data)
{
    for (size_t i = 0; i < writers_.size(); ++i)
    {
        expect_no_writes_except(i);
        listener_.set_datawriter(kinds_[i], &writers_[i]);
        write_all_data();
        listener_.set_datawriter(kinds_[i], nullptr);
        check_and_reset_expectations();
    }
}

// Setting one writer and sending the corresponding kind of data only writes on the writer set
TEST_F(DomainParticipantStatisticsListenerTests, single_writer_single_data)
{
    for (size_t i = 0; i < writers_.size(); ++i)
    {
        expect_no_writes_except(i);
        listener_.set_datawriter(kinds_[i], &writers_[i]);
        listener_.on_statistics_data(data_[i]);
        listener_.set_datawriter(kinds_[i], nullptr);
        check_and_reset_expectations();
    }
}

// Setting one writer and sending other kind of data means no writer should be called
TEST_F(DomainParticipantStatisticsListenerTests, single_writer_other_data)
{
    for (size_t i = 0; i < writers_.size(); ++i)
    {
        expect_no_writes();
        listener_.set_datawriter(kinds_[i], &writers_[i]);
        write_all_data_except(kinds_[i]);
        listener_.set_datawriter(kinds_[i], nullptr);
        check_and_reset_expectations();
    }
}

} // dds
} // statistics
} // fastdds
} // eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
