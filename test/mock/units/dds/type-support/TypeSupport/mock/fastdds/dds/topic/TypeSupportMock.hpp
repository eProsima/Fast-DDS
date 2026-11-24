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

#ifndef FASTDDS_TEST_MOCK_FASTDDS_DDS_TOPIC__TYPESUPPORTMOCK_HPP
#define FASTDDS_TEST_MOCK_FASTDDS_DDS_TOPIC__TYPESUPPORTMOCK_HPP

#include <string>

#include <gmock/gmock.h>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipant;

/**
 * Expectations of the TypeSupport mock implementation.
 *
 * While an instance exists, TypeSupport::register_type() forwards to it, receiving the type name that the real
 * implementation would register. Otherwise it returns RETCODE_UNSUPPORTED. Only one instance can exist at a time.
 */
class TypeSupportMock
{
public:

    TypeSupportMock()
    {
        instance() = this;
    }

    virtual ~TypeSupportMock()
    {
        instance() = nullptr;
    }

    MOCK_METHOD2(register_type, ReturnCode_t(DomainParticipant*, const std::string&));

    /**
     * Get the instance the TypeSupport mock implementation forwards to.
     *
     * @return Reference to the pointer to the existing instance, nullptr if there is none.
     */
    static TypeSupportMock*& instance();
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_FASTDDS_DDS_TOPIC__TYPESUPPORTMOCK_HPP
