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

#include <tuple>
#include <vector>

#include <fastcdr/Cdr.h>
#include <gtest/gtest.h>

#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

using namespace eprosima::fastdds::dds;

class MockHelloWorldPubSubType : public HelloWorldPubSubType
{
public:

    bool serialize(
            const void* const data,
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            DataRepresentationId_t data_representation) override
    {
        last_data_representation = data_representation;

        return HelloWorldPubSubType::serialize(data, payload, data_representation);
    }

    bool deserialize(
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            void* data) override
    {
        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload.data), payload.length);

        // Object that deserializes the data.
        eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN);

        // Deserialize encapsulation.
        deser.read_encapsulation();

        last_encoding = deser.get_encoding_flag();

        return HelloWorldPubSubType::deserialize(payload, data);
    }

    static eprosima::fastcdr::EncodingAlgorithmFlag last_encoding;

    static DataRepresentationId_t last_data_representation;

private:

    using HelloWorldPubSubType::serialize;
};

eprosima::fastcdr::EncodingAlgorithmFlag
MockHelloWorldPubSubType::last_encoding {eprosima::fastcdr::EncodingAlgorithmFlag::PL_CDR};

DataRepresentationId_t MockHelloWorldPubSubType::last_data_representation { XML_DATA_REPRESENTATION};

using DataRepresentationQosVector = std::vector<DataRepresentationId_t>;

class DataRepresentationQosCompatibility : public ::testing::TestWithParam<std::tuple<
                DataRepresentationQosVector,
                DataRepresentationQosVector,
                bool,
                DataRepresentationId_t,
                eprosima::fastcdr::EncodingAlgorithmFlag
                >>
{
};


DataRepresentationQosVector EMPTY_VECTOR {};
DataRepresentationQosVector XCDR_VECTOR {XCDR_DATA_REPRESENTATION};
DataRepresentationQosVector XCDR2_VECTOR {XCDR2_DATA_REPRESENTATION};
DataRepresentationQosVector BOTH_XCDR_VECTOR {XCDR_DATA_REPRESENTATION, XCDR2_DATA_REPRESENTATION};

TEST_P(DataRepresentationQosCompatibility, check_compatibility)
{
    PubSubWriter<MockHelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<MockHelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.data_representation(std::get<0>(GetParam())).init();
    reader.reliability(RELIABLE_RELIABILITY_QOS).data_representation(std::get<1>(GetParam())).init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    if (std::get<2>(GetParam()))
    {
        writer.wait_discovery();
        reader.wait_discovery();

        auto data = default_helloworld_data_generator();

        reader.startReception(data);
        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());
        // Block reader until reception finished or timeout.
        reader.block_for_all();

        MockHelloWorldPubSubType* writer_type =
                dynamic_cast<MockHelloWorldPubSubType*>(writer.get_type_support().get());
        MockHelloWorldPubSubType* reader_type =
                dynamic_cast<MockHelloWorldPubSubType*>(reader.get_type_support().get());
        ASSERT_EQ(std::get<3>(GetParam()), writer_type->last_data_representation);
        ASSERT_EQ(std::get<4>(GetParam()), reader_type->last_encoding);
    }
    else
    {
        writer.wait_incompatible_qos();
        reader.wait_incompatible_qos();

        ASSERT_EQ(DATAREPRESENTATION_QOS_POLICY_ID, writer.last_incompatible_qos());
        ASSERT_EQ(DATAREPRESENTATION_QOS_POLICY_ID, reader.last_incompatible_qos());
    }
}

INSTANTIATE_TEST_SUITE_P(
    DataRepresentationQos,
    DataRepresentationQosCompatibility,
    ::testing::Values(
        std::make_tuple(EMPTY_VECTOR, EMPTY_VECTOR, true, XCDR_DATA_REPRESENTATION, eprosima::fastcdr::PLAIN_CDR),
        std::make_tuple(EMPTY_VECTOR, XCDR_VECTOR, true, XCDR_DATA_REPRESENTATION, eprosima::fastcdr::PLAIN_CDR),
        std::make_tuple(EMPTY_VECTOR, XCDR2_VECTOR, false, XCDR_DATA_REPRESENTATION, eprosima::fastcdr::PLAIN_CDR),
        std::make_tuple(EMPTY_VECTOR, BOTH_XCDR_VECTOR, true, XCDR_DATA_REPRESENTATION, eprosima::fastcdr::PLAIN_CDR),
        std::make_tuple(XCDR_VECTOR, EMPTY_VECTOR, true, XCDR_DATA_REPRESENTATION, eprosima::fastcdr::PLAIN_CDR),
        std::make_tuple(XCDR_VECTOR, XCDR_VECTOR, true, XCDR_DATA_REPRESENTATION, eprosima::fastcdr::PLAIN_CDR),
        std::make_tuple(XCDR_VECTOR, XCDR2_VECTOR, false, XCDR_DATA_REPRESENTATION, eprosima::fastcdr::PLAIN_CDR),
        std::make_tuple(XCDR_VECTOR, BOTH_XCDR_VECTOR, true, XCDR_DATA_REPRESENTATION, eprosima::fastcdr::PLAIN_CDR),
        std::make_tuple(XCDR2_VECTOR, EMPTY_VECTOR, false, XCDR2_DATA_REPRESENTATION, eprosima::fastcdr::PLAIN_CDR2),
        std::make_tuple(XCDR2_VECTOR, XCDR_VECTOR, false, XCDR2_DATA_REPRESENTATION, eprosima::fastcdr::PLAIN_CDR2),
        std::make_tuple(XCDR2_VECTOR, XCDR2_VECTOR, true, XCDR2_DATA_REPRESENTATION, eprosima::fastcdr::PLAIN_CDR2),
        std::make_tuple(XCDR2_VECTOR, BOTH_XCDR_VECTOR, true, XCDR2_DATA_REPRESENTATION, eprosima::fastcdr::PLAIN_CDR2)
        ));
