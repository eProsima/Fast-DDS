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

// Link-seam implementation of DataWriter: replaces src/cpp/fastdds/publisher/DataWriter.cpp, for unit tests passing
// writers to the code under test. Only the constructors, enable() and write() are implemented, and writing is
// forwarded to the DataWriterMock companion of the instance.

#include <fastdds/dds/publisher/DataWriter.hpp>

#include <mock/fastdds/dds/publisher/DataWriterMock.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

using Companion = mock::MockCompanion<DataWriter>;

DataWriter::DataWriter(
        DataWriterImpl* impl,
        const StatusMask& mask)
    : DomainEntity(mask)
    , impl_(impl)
{
    Companion::attach(this);
}

DataWriter::DataWriter(
        Publisher*,
        Topic*,
        const DataWriterQos&,
        DataWriterListener*,
        const StatusMask& mask)
    : DomainEntity(mask)
    , impl_(nullptr)
{
    Companion::attach(this);
}

DataWriter::~DataWriter()
{
    Companion::detach(this);
}

ReturnCode_t DataWriter::enable()
{
    return DomainEntity::enable();
}

ReturnCode_t DataWriter::write(
        const void* const data)
{
    return Companion::get(this).write(data);
}

ReturnCode_t DataWriter::write(
        const void* const data,
        fastdds::rtps::WriteParams& params)
{
    return Companion::get(this).write(data, params);
}

ReturnCode_t DataWriter::write(
        const void* const data,
        const InstanceHandle_t& handle)
{
    return Companion::get(this).write(data, handle);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
