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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/rtps/builtin/discovery/participant/PDP.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

using ::testing::Return;
using ::testing::ReturnRef;

class PDPMock : public PDP
{
public:

    PDPMock(
            BuiltinProtocols* pdp,
            RTPSParticipantImpl* part)
    {
    }

    virtual ~PDPMock()
    {
    }

};

class PDPTests : public ::testing::Test
{

protected:

    void SetUp() override
    {
        RTPSParticipantAllocationAttributes attrs;

        //pdp_ = new PDP(&builtin_prots_, attrs);
    }

    void TearDown() override
    {
    }

    void set_incompatible_topic()
    {
        //rdata->topicName("AnotherTopic");
    }

    void set_incompatible_topic_kind()
    {
        //rdata->topicKind(TopicKind_t::WITH_KEY);
    }

    void set_incompatible_type()
    {
        //rdata->typeName("AnotherTypeName");
    }

    PDPMock pdp_;
    ::testing::NiceMock<BuiltinProtocols> builtin_prots_;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
