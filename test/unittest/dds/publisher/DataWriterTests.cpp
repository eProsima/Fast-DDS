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
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/pub/qos/DataWriterQos.hpp>
#include <dds/pub/AnyDataWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

TEST(DataWriterTests, DISABLED_ChangeDataWriterQos)
{
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    (void) publisher;
    //DataWriter* datawriter = publisher->create_datawriter(DATAWRITER_QOS_DEFAULT);
    DataWriterQos qos;
    //datawriter->get_qos(qos);
    ASSERT_EQ(qos, DATAWRITER_QOS_DEFAULT);

    qos.deadline().period = 260;

    //ASSERT_TRUE(datawriter->set_qos(qos) == ReturnCode_t::RETCODE_OK);
    DataWriterQos wqos;
    //datawriter->get_qos(wqos);

    ASSERT_EQ(qos, wqos);
    ASSERT_EQ(wqos.deadline().period, 260);
}


TEST(DataWriterTests, DISABLED_ChangePSMDataWriterQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::pub::Publisher publisher = ::dds::pub::Publisher(participant);
    (void) publisher;
    //::dds::pub::AnyDataWriter datawriter = ::dds::pub::AnyDataWriter();

    //::dds::pub::qos::DataWriterQos qos = datawriter.qos();
    //    ASSERT_EQ(qos, DATAWRITER_QOS_DEFAULT);

    //    qos.deadline().period = 540;

    //    ASSERT_NO_THROW(datawriter.qos(qos));
    //    ::dds::pub::qos::DataWriterQos wqos = datawriter.qos();

    //    ASSERT_EQ(qos, wqos);
    //    ASSERT_EQ(wqos.deadline().period, 540);
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
