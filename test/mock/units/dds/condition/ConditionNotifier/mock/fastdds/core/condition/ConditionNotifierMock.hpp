// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_TEST_MOCK_FASTDDS_CORE_CONDITION__CONDITIONNOTIFIERMOCK_HPP
#define FASTDDS_TEST_MOCK_FASTDDS_CORE_CONDITION__CONDITIONNOTIFIERMOCK_HPP

#include <gmock/gmock.h>

#include <fastdds/core/condition/ConditionNotifier.hpp>
#include <fastdds/dds/core/condition/Condition.hpp>
#include <mock/MockCompanion.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

/**
 * Expectations of a ConditionNotifier created with the mock implementation, reached with mock::mock_of().
 * No WaitSet implementation is ever attached or notified.
 */
class ConditionNotifierMock
{
public:

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD1(attach_to, void(WaitSetImpl * wait_set));

    MOCK_METHOD1(detach_from, void(WaitSetImpl * wait_set));

    MOCK_METHOD0(notify, void());

    MOCK_METHOD1(will_be_deleted, void(const Condition& condition));
    // *INDENT-ON*
};

} // namespace detail
} // namespace dds

namespace mock {

template<>
struct MockOf<dds::detail::ConditionNotifier>
{
    using type = testing::StrictMock<dds::detail::ConditionNotifierMock>;
};

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_FASTDDS_CORE_CONDITION__CONDITIONNOTIFIERMOCK_HPP
