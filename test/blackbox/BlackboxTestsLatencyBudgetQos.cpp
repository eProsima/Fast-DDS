// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "BlackboxTests.hpp"

#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "PubSubParticipant.hpp"
#include "ReqRepAsReliableHelloWorldRequester.hpp"
#include "ReqRepAsReliableHelloWorldReplier.hpp"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

TEST(LatencyBudgetQos, DurationCheck)
{
	PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

	Duration_t latency_budget_s(10);

	reader.reliability(RELIABLE_RELIABILITY_QOS)
	.latency_budget_duration(latency_budget_s)
	.init();

	writer.reliability(RELIABLE_RELIABILITY_QOS)
	.latency_budget_duration(latency_budget_s)
	.init();

	ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    EXPECT_EQ(reader.get_latency_budget_duration(), latency_budget_s);
    EXPECT_EQ(writer.get_latency_budget_duration(), latency_budget_s);
}