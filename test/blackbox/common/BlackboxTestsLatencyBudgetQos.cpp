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

#include "ReqRepHelloWorldRequester.hpp"

#include <gtest/gtest.h>

TEST(LatencyBudgetQos, DurationCheck)
{
    ReqRepHelloWorldRequester requester;

    eprosima::fastdds::dds::Duration_t latency_budget_pub(10);
    eprosima::fastdds::dds::Duration_t latency_budget_sub(20);

    requester.init_with_latency(latency_budget_pub, latency_budget_sub);

    ASSERT_TRUE(requester.isInitialized());

    EXPECT_EQ(requester.datawriter_latency_budget_duration(), latency_budget_pub);
    EXPECT_EQ(requester.datareader_latency_budget_duration(), latency_budget_sub);
}
