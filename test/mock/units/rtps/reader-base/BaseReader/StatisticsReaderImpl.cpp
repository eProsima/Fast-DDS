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

// Link-seam implementation of StatisticsReaderImpl: replaces src/cpp/statistics/rtps/reader/StatisticsReaderImpl.cpp.
// No statistics are collected or notified.

#include <fastdds/statistics/rtps/StatisticsCommon.hpp>

#include <rtps/reader/BaseReader.hpp>
#include <statistics/rtps/StatisticsBase.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {

using eprosima::fastdds::rtps::BaseReader;
using eprosima::fastdds::rtps::GUID_t;

StatisticsReaderImpl::StatisticsReaderImpl()
{
}

StatisticsReaderAncillary* StatisticsReaderImpl::get_members() const
{
    return nullptr;
}

RecursiveTimedMutex& StatisticsReaderImpl::get_statistics_mutex()
{
    return static_cast<BaseReader*>(this)->getMutex();
}

const GUID_t& StatisticsReaderImpl::get_guid() const
{
    return static_cast<const BaseReader*>(this)->getGuid();
}

void StatisticsReaderImpl::on_data_notify(
        const fastdds::rtps::GUID_t&,
        const fastdds::rtps::Time_t&)
{
}

void StatisticsReaderImpl::on_acknack(
        int32_t)
{
}

void StatisticsReaderImpl::on_nackfrag(
        int32_t)
{
}

void StatisticsReaderImpl::on_subscribe_throughput(
        uint32_t)
{
}

} // namespace statistics
} // namespace fastdds
} // namespace eprosima
