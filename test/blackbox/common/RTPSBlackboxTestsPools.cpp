// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <unordered_map>
#include <vector>

#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <gtest/gtest.h>

#include "BlackboxTests.hpp"
#include "RTPSWithRegistrationReader.hpp"
#include "RTPSWithRegistrationWriter.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS
};

class RTPSCustomPools : public testing::TestWithParam<communication_type>
{
public:

    void SetUp() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
                eprosima::fastdds::rtps::RTPSDomain::set_library_settings(library_settings);
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

    void TearDown() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF;
                eprosima::fastdds::rtps::RTPSDomain::set_library_settings(library_settings);
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

};

class PoolForTest : public IPayloadPool
{
public:

    PoolForTest(
            uint32_t payload_size,
            uint32_t num_endpoints,
            uint32_t num_samples)
        : payload_size_(payload_size)
    {
        for (uint32_t i = 0; i < num_samples * num_endpoints; ++i)
        {
            octet* payload = (octet*)calloc(payload_size_, sizeof(octet));

            all_payloads_.emplace(payload, 0u);
            free_payloads_.push_back(payload);
        }
    }

    ~PoolForTest()
    {
        for (auto it : all_payloads_)
        {
            free(it.first);
        }
    }

    bool get_payload(
            uint32_t size,
            SerializedPayload_t& payload) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return do_get_payload(size, payload);
    }

    bool get_payload(
            const SerializedPayload_t& data,
            SerializedPayload_t& payload) override
    {
        std::lock_guard<std::mutex> lock(mutex_);

        octet* payload_buff = data.data;

        if (data.payload_owner == this)
        {
            uint32_t& refs = all_payloads_[payload_buff];
            EXPECT_LT(0u, refs);
            ++refs;
            ++num_reserves_;
            ++num_references_;

            payload.data = payload_buff;
            payload.length = data.length;
            payload.max_size = data.max_size;
            payload.payload_owner = this;
            return true;
        }

        if (!do_get_payload(data.max_size, payload))
        {
            return false;
        }

        ++num_copies_;
        payload.copy(&data, true);

        return true;
    }

    bool release_payload(
            SerializedPayload_t& payload) override
    {
        std::lock_guard<std::mutex> lock(mutex_);

        EXPECT_EQ(this, payload.payload_owner);

        octet* payload_buff = payload.data;
        uint32_t& refs = all_payloads_[payload_buff];

        EXPECT_GT(refs, 0u);

        ++num_releases_;
        if (0 == --refs)
        {
            free_payloads_.push_back(payload_buff);
        }

        payload.data = nullptr;
        payload.max_size = 0;
        payload.length = 0;
        payload.payload_owner = nullptr;

        return true;
    }

    size_t num_reserves() const
    {
        return num_reserves_;
    }

    size_t num_releases() const
    {
        return num_releases_;
    }

    size_t num_references() const
    {
        return num_references_;
    }

    size_t num_copies() const
    {
        return num_copies_;
    }

private:

    bool do_get_payload(
            uint32_t size,
            SerializedPayload_t& payload)
    {
        if (free_payloads_.empty())
        {
            return false;
        }

        EXPECT_LE(size, payload_size_);

        octet* payload_buff = free_payloads_.back();
        uint32_t& refs = all_payloads_[payload_buff];
        EXPECT_EQ(0u, refs);

        free_payloads_.pop_back();
        ++refs;
        ++num_reserves_;

        payload.data = payload_buff;
        payload.max_size = payload_size_;
        payload.length = 0;
        payload.pos = 0;
        payload.payload_owner = this;

        return true;
    }

    uint32_t payload_size_;

    size_t num_reserves_ = 0;
    size_t num_releases_ = 0;
    size_t num_references_ = 0;
    size_t num_copies_ = 0;

    std::mutex mutex_;
    std::unordered_map<octet*, uint32_t> all_payloads_;
    std::vector<octet*> free_payloads_;
};

template <class TData, class TType>
void do_test(
        const std::string& topic_name,
        std::list<TData>& data,
        bool pool_on_writer,
        bool pool_on_reader,
        bool should_not_copy)
{
    TType type_support;
    uint32_t num_samples = static_cast<uint32_t>(data.size());
    uint32_t num_endpoints = (uint32_t)pool_on_reader + (uint32_t)pool_on_writer;
    uint32_t payload_size = static_cast<uint32_t>(type_support.max_serialized_type_size);
    payload_size += static_cast<uint32_t>(eprosima::fastcdr::Cdr::alignment(payload_size, 4)); /* possible submessage alignment */
    payload_size += 4u; // encapsulation header

    std::shared_ptr<PoolForTest> pool = std::make_shared<PoolForTest>(payload_size, num_endpoints, num_samples);

    {
        RTPSWithRegistrationReader<TType> reader(topic_name);
        RTPSWithRegistrationWriter<TType> writer(topic_name);

        reader.reliability(eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE);
        if (pool_on_reader)
        {
            reader.payload_pool(pool);
        }
        reader.init();

        ASSERT_TRUE(reader.isInitialized());

        if (pool_on_writer)
        {
            writer.payload_pool(pool);
        }
        writer.init();

        ASSERT_TRUE(writer.isInitialized());

        // Wait for discovery.
        writer.wait_discovery();
        reader.wait_discovery();

        reader.expected_data(data);
        reader.startReception();

        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());
        // Block reader until reception finished or timeout.
        reader.block_for_all();
    }

    ASSERT_EQ(pool->num_releases(), pool->num_reserves());
    EXPECT_GE(pool->num_reserves(), num_samples * num_endpoints);

    if (should_not_copy)
    {
        EXPECT_GE(pool->num_references(), num_samples);
        EXPECT_EQ(pool->num_copies(), 0u);
    }
}

TEST_P(RTPSCustomPools, CreateFailsWithInvalidPool)
{
    std::shared_ptr<IPayloadPool> no_pool;
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    reader.payload_pool(no_pool).init();
    EXPECT_FALSE(reader.isInitialized());

    writer.payload_pool(no_pool).init();
    EXPECT_FALSE(writer.isInitialized());
}

TEST_P(RTPSCustomPools, RTPSAsReliableWithRegistrationNoPools)
{
    auto data = default_helloworld_data_generator();
    do_test<HelloWorld, HelloWorldPubSubType>(TEST_TOPIC_NAME, data, false, false, false);
}

TEST_P(RTPSCustomPools, RTPSAsReliableWithRegistrationReaderPool)
{
    auto data = default_helloworld_data_generator();
    do_test<HelloWorld, HelloWorldPubSubType>(TEST_TOPIC_NAME, data, false, true, false);
}

TEST_P(RTPSCustomPools, RTPSAsReliableWithRegistrationWriterPool)
{
    auto data = default_helloworld_data_generator();
    do_test<HelloWorld, HelloWorldPubSubType>(TEST_TOPIC_NAME, data, true, false, false);
}

TEST_P(RTPSCustomPools, RTPSAsReliableWithRegistrationBothPools)
{
    auto data = default_helloworld_data_generator();
    do_test<HelloWorld, HelloWorldPubSubType>(TEST_TOPIC_NAME, data, true, true, GetParam() == INTRAPROCESS);
}

TEST_P(RTPSCustomPools, RTPSAsReliableWithRegistrationNoPools300Kb)
{
    auto data = default_data300kb_data_generator();
    do_test<Data1mb, Data1mbPubSubType>(TEST_TOPIC_NAME, data, false, false, false);
}

TEST_P(RTPSCustomPools, RTPSAsReliableWithRegistrationReaderPool300Kb)
{
    auto data = default_data300kb_data_generator();
    do_test<Data1mb, Data1mbPubSubType>(TEST_TOPIC_NAME, data, false, true, false);
}

TEST_P(RTPSCustomPools, RTPSAsReliableWithRegistrationWriterPool300Kb)
{
    auto data = default_data300kb_data_generator();
    do_test<Data1mb, Data1mbPubSubType>(TEST_TOPIC_NAME, data, true, false, false);
}

TEST_P(RTPSCustomPools, RTPSAsReliableWithRegistrationBothPools300Kb)
{
    auto data = default_data300kb_data_generator();
    do_test<Data1mb, Data1mbPubSubType>(TEST_TOPIC_NAME, data, true, true, false);
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(RTPSCustomPools,
        RTPSCustomPools,
        testing::Values(TRANSPORT, INTRAPROCESS),
        [](const testing::TestParamInfo<RTPSCustomPools::ParamType>& info)
        {
            switch (info.param)
            {
                case INTRAPROCESS:
                    return "Intraprocess";
                    break;
                case TRANSPORT:
                default:
                    return "Transport";
            }

        });
