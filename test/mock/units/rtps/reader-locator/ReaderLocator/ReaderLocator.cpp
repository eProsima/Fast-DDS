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

// Link-seam implementation of ReaderLocator: replaces src/cpp/rtps/writer/ReaderLocator.cpp. The remote reader is
// never set, so it is neither local nor datasharing, and nothing is sent.

#include <rtps/writer/ReaderLocator.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

ReaderLocator::ReaderLocator(
        BaseWriter* owner,
        size_t max_unicast_locators,
        size_t max_multicast_locators)
    : owner_(owner)
    , participant_owner_(nullptr)
    , general_locator_info_(max_unicast_locators, max_multicast_locators)
    , async_locator_info_(max_unicast_locators, max_multicast_locators)
    , expects_inline_qos_(false)
    , is_local_reader_(false)
    , local_reader_()
    , guid_prefix_as_vector_(1u)
    , guid_as_vector_(1u)
    , datasharing_notifier_(nullptr)
{
}

ReaderLocator::~ReaderLocator()
{
}

bool ReaderLocator::start(
        const GUID_t&,
        const ResourceLimitedVector<Locator_t>&,
        const ResourceLimitedVector<Locator_t>&,
        bool,
        bool)
{
    return true;
}

bool ReaderLocator::update(
        const ResourceLimitedVector<Locator_t>&,
        const ResourceLimitedVector<Locator_t>&,
        bool)
{
    return true;
}

bool ReaderLocator::stop(
        const GUID_t&)
{
    return true;
}

void ReaderLocator::stop()
{
}

bool ReaderLocator::send(
        const std::vector<eprosima::fastdds::rtps::NetworkBuffer>&,
        const uint32_t&,
        std::chrono::steady_clock::time_point)
{
    return true;
}

LocalReaderPointer::Instance ReaderLocator::local_reader()
{
    return LocalReaderPointer::Instance(local_reader_);
}

bool ReaderLocator::is_datasharing_reader() const
{
    return false;
}

void ReaderLocator::datasharing_notify()
{
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
