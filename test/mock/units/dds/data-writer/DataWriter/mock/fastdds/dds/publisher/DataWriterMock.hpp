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

#ifndef FASTDDS_TEST_MOCK_FASTDDS_DDS_PUBLISHER__DATAWRITERMOCK_HPP
#define FASTDDS_TEST_MOCK_FASTDDS_DDS_PUBLISHER__DATAWRITERMOCK_HPP

#include <gmock/gmock.h>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/rtps/common/WriteParams.hpp>
#include <mock/MockCompanion.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Expectations of a DataWriter created with the mock implementation, reached with mock::mock_of().
 * Nothing is ever published.
 */
class DataWriterMock
{
public:

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD1(write, ReturnCode_t(const void* const data));

    MOCK_METHOD2(write, ReturnCode_t(const void* const data, fastdds::rtps::WriteParams& params));

    MOCK_METHOD2(write, ReturnCode_t(const void* const data, const InstanceHandle_t& handle));
    // *INDENT-ON*
};

/**
 * DataWriter not bound to any implementation, for tests passing writers to the code under test.
 */
class TestDataWriter : public DataWriter
{
public:

    TestDataWriter()
        : DataWriter(static_cast<DataWriterImpl*>(nullptr))
    {
    }

};

} // namespace dds

namespace mock {

template<>
struct MockOf<dds::DataWriter>
{
    using type = testing::StrictMock<dds::DataWriterMock>;
};

/**
 * Get the companion of a TestDataWriter, which is the one of its DataWriter base.
 * @param writer Instance created with the mock implementation.
 * @return Companion holding the expectations of the instance.
 */
inline MockOf<dds::DataWriter>::type& mock_of(
        const dds::TestDataWriter* writer)
{
    return MockCompanion<dds::DataWriter>::get(writer);
}

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_FASTDDS_DDS_PUBLISHER__DATAWRITERMOCK_HPP
