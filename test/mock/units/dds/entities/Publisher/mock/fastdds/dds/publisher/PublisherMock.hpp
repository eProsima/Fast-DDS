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

#ifndef FASTDDS_TEST_MOCK_DDS_PUBLISHER__PUBLISHERMOCK_HPP
#define FASTDDS_TEST_MOCK_DDS_PUBLISHER__PUBLISHERMOCK_HPP

#include <gmock/gmock.h>

#include <fastdds/dds/publisher/Publisher.hpp>
#include <mock/MockCompanion.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Expectations of a Publisher created with the mock implementation, reached with mock::mock_of().
 */
class PublisherMock
{
public:

    //! Called by create_datawriter(), which fails returning nullptr when this returns true.
    MOCK_METHOD0(create_datawriter_mock, bool());

    //! Called by delete_datawriter(), which fails returning RETCODE_ERROR when this returns true.
    MOCK_METHOD0(delete_datawriter_mock, bool());

};

/**
 * Marks a Publisher created with the mock implementation as enabled, which the real one is once its participant is
 * enabled. The DataWriters created on it are then enabled as well, so that their creation can fail.
 *
 * Publisher::enable() does nothing in the mock implementation, since every Publisher is created through it. Only the
 * tests that need a DataWriter creation to fail call this.
 */
inline void mark_enabled(
        Publisher* publisher)
{
    publisher->Entity::enable();
}

} // namespace dds

namespace mock {

template<>
struct MockOf<dds::Publisher>
{
    using type = dds::PublisherMock;
};

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_DDS_PUBLISHER__PUBLISHERMOCK_HPP
