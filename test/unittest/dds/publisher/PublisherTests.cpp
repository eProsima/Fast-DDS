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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/qos/DataWriterQos.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/core/types.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

TEST(PublisherTests, ChangeDefaultDataWriterQos)
{
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);

    DataWriterQos qos;
    publisher->get_default_datawriter_qos(qos);
    ASSERT_EQ(qos, DATAWRITER_QOS_DEFAULT);

    qos.reliability.kind = BEST_EFFORT_RELIABILITY_QOS;

    ASSERT_TRUE(publisher->set_default_datawriter_qos(qos) == ReturnCode_t::RETCODE_OK);

    DataWriterQos wqos;
    publisher->get_default_datawriter_qos(wqos);

    ASSERT_EQ(qos, wqos);
    ASSERT_EQ(wqos.reliability.kind, BEST_EFFORT_RELIABILITY_QOS);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
