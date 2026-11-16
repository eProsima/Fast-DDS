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

// Link-seam implementation of StatisticsWriterImpl: replaces src/cpp/statistics/rtps/writer/StatisticsWriterImpl.cpp.
// No statistics are collected or notified.

#include <fastdds/statistics/rtps/StatisticsCommon.hpp>

#include <rtps/writer/BaseWriter.hpp>
#include <statistics/rtps/StatisticsBase.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {

using eprosima::fastdds::rtps::BaseWriter;
using eprosima::fastdds::rtps::GUID_t;

StatisticsWriterImpl::StatisticsWriterImpl()
{
}

StatisticsWriterAncillary* StatisticsWriterImpl::get_members() const
{
    return nullptr;
}

RecursiveTimedMutex& StatisticsWriterImpl::get_statistics_mutex()
{
    return static_cast<BaseWriter*>(this)->getMutex();
}

const GUID_t& StatisticsWriterImpl::get_guid() const
{
    return static_cast<const BaseWriter*>(this)->getGuid();
}

void StatisticsWriterImpl::on_heartbeat(
        uint32_t)
{
}

void StatisticsWriterImpl::on_data_generated(
        size_t)
{
}

void StatisticsWriterImpl::on_data_sent()
{
}

void StatisticsWriterImpl::on_publish_throughput(
        uint32_t)
{
}

void StatisticsWriterImpl::on_gap()
{
}

} // namespace statistics
} // namespace fastdds
} // namespace eprosima
