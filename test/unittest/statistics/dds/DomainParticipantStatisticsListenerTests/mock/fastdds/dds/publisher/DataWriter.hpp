// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file DataWriter.hpp
 */

#ifndef _FASTDDS_DDS_PUBLISHER_DATAWRITER_HPP_
#define _FASTDDS_DDS_PUBLISHER_DATAWRITER_HPP_

#include <gmock/gmock.h>

#include <fastdds/dds/core/Entity.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DataWriterImpl;
class Publisher;
class Topic;

class DataWriter : public Entity
{
public:

    MOCK_METHOD1(write, bool(void* data));

    DataWriter() = default;

    DataWriter(
            DataWriterImpl* impl,
            const StatusMask& mask = StatusMask::all())
    {
        impl_ = impl;
        static_cast<void>(mask);
    }

    enum class LoanInitializationKind
    {
        NO_LOAN_INITIALIZATION,
        ZERO_LOAN_INITIALIZATION,
        CONSTRUCTED_LOAN_INITIALIZATION
    };

    virtual ~DataWriter() = default;

    Publisher* get_publisher() const
    {
        return nullptr;
    }

    Topic* get_topic() const
    {
        return nullptr;
    }

    DataWriterImpl* impl_;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif  // _FASTDDS_DDS_PUBLISHER_DATAWRITER_HPP_
