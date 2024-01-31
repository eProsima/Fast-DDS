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

#include "fastdds/rtps/common/Types.h"
#include "fastrtps/qos/QosPolicies.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/rtps/RTPSDomain.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/history/WriterHistory.h>

#include <rtps/history/BasicPayloadPool.hpp>
#include <rtps/history/CacheChangePool.h>
#include <rtps/history/PoolConfig.h>

#include <rtps/DataSharing/WriterPool.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

using namespace testing;

// This test will attempt to create enable DataSharing while being unable to compute the segment size due to a lack
// of permissions. This should fail but not propagate an uncaught boost::interprocess::interprocess_exception
TEST(SHMSegmentTests, Writer)
{
    RTPSParticipantAttributes p_attr;
    p_attr.builtin.discovery_config.discoveryProtocol = eprosima::fastrtps::rtps::DiscoveryProtocol::SIMPLE;
    p_attr.builtin.use_WriterLivelinessProtocol = true;
    RTPSParticipant* participant = RTPSDomain::createParticipant(
        0, p_attr);

    ASSERT_NE(participant, nullptr);

    HistoryAttributes history_attr;
    history_attr.payloadMaxSize = 255;
    history_attr.maximumReservedCaches = 50;
    WriterHistory* history = new WriterHistory(history_attr);
    WriterAttributes writer_attr;
    DataSharingQosPolicy dsq;
    // We select a folder to force the use of SharedFileSegment
    dsq.automatic("tmp");
    writer_attr.endpoint.set_data_sharing_configuration(dsq);

    std::shared_ptr<WriterPool> payload_pool(new WriterPool(history_attr.payloadMaxSize,
            history_attr.maximumReservedCaches));

    RTPSWriter* writer = nullptr;
    EXPECT_NO_THROW(writer = RTPSDomain::createRTPSWriter(participant, writer_attr, payload_pool, history));
    // RTPSWriter creation failed, as expected.
    EXPECT_EQ(writer, nullptr);

    RTPSDomain::removeRTPSParticipant(participant);
    delete(history);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
