// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file OriginalWriterInfo_t.hpp
 */

#ifndef FASTDDS_RTPS_COMMON__ORIGINALWRITERINFO_HPP
#define FASTDDS_RTPS_COMMON__ORIGINALWRITERINFO_HPP

#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/core/policy/ParameterList.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using ParameterList = eprosima::fastdds::dds::ParameterList;

class FASTDDS_EXPORTED_API OriginalWriterInfo
{
public:

    OriginalWriterInfo() = default;

    OriginalWriterInfo(
            const GUID_t& original_writer_guid,
            const SequenceNumber_t& sequence_number,
            const ParameterList& original_writer_qos)
        : original_writer_guid_(original_writer_guid)
        , sequence_number_(sequence_number)
        , original_writer_qos_(original_writer_qos)
    {
    }

    OriginalWriterInfo(
            GUID_t&& original_writer_guid,
            SequenceNumber_t&& sequence_number,
            ParameterList&& original_writer_qos)
        : original_writer_guid_(std::move(original_writer_guid))
        , sequence_number_(std::move(sequence_number))
        , original_writer_qos_(std::move(original_writer_qos))
    {
    }

    OriginalWriterInfo(
            const OriginalWriterInfo& other) = default;

    OriginalWriterInfo(
            OriginalWriterInfo&& other) = default;

    OriginalWriterInfo& operator =(
            const OriginalWriterInfo& other) = default;

    OriginalWriterInfo& operator =(
            OriginalWriterInfo&& other) = default;

    bool operator ==(
            const OriginalWriterInfo& other) const
    {
        return (original_writer_guid_ == other.original_writer_guid_) &&
               (sequence_number_ == other.sequence_number_) &&
               (original_writer_qos_ == other.original_writer_qos_);
    }

    bool operator !=(
            const OriginalWriterInfo& other) const
    {
        return !(*this == other);
    }

    const GUID_t& original_writer_guid() const
    {
        return original_writer_guid_;
    }

    GUID_t& original_writer_guid()
    {
        return original_writer_guid_;
    }

    void original_writer_guid(
            const GUID_t& guid)
    {
        original_writer_guid_ = guid;
    }

    void original_writer_guid(
            GUID_t&& guid)
    {
        original_writer_guid_ = std::move(guid);
    }

    const SequenceNumber_t& sequence_number() const
    {
        return sequence_number_;
    }

    SequenceNumber_t& sequence_number()
    {
        return sequence_number_;
    }

    void sequence_number(
            const SequenceNumber_t& seq)
    {
        sequence_number_ = seq;
    }

    void sequence_number(
            SequenceNumber_t&& seq)
    {
        sequence_number_ = std::move(seq);
    }

    const ParameterList& original_writer_qos() const
    {
        return original_writer_qos_;
    }

    ParameterList& original_writer_qos()
    {
        return original_writer_qos_;
    }

    void original_writer_qos(
            const ParameterList& qos)
    {
        original_writer_qos_ = qos;
    }

    void original_writer_qos(
            ParameterList&& qos)
    {
        original_writer_qos_ = std::move(qos);
    }

private:

    GUID_t original_writer_guid_ = GUID_t::unknown();

    SequenceNumber_t sequence_number_ = SequenceNumber_t::unknown();

    ParameterList original_writer_qos_;
};

} //namespace rtps
} //namespace fastdds
} //namespace eprosima
#endif
