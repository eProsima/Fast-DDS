// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file OriginalWriterInfo.hpp
 */

#ifndef FASTDDS_RTPS_COMMON__ORIGINALWRITERINFO_HPP
#define FASTDDS_RTPS_COMMON__ORIGINALWRITERINFO_HPP

#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {


class FASTDDS_EXPORTED_API OriginalWriterInfo
{
public:

    OriginalWriterInfo() = default;

    OriginalWriterInfo(
            const GUID_t& original_writer_guid,
            const SequenceNumber_t& sequence_number)
        : original_writer_guid_(original_writer_guid)
        , sequence_number_(sequence_number)
    {
    }

    OriginalWriterInfo(
            GUID_t&& original_writer_guid,
            SequenceNumber_t&& sequence_number)
        : original_writer_guid_(std::move(original_writer_guid))
        , sequence_number_(std::move(sequence_number))
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

    static OriginalWriterInfo unknown()
    {
        return OriginalWriterInfo();
    }

    bool operator ==(
            const OriginalWriterInfo& other) const
    {
        return (original_writer_guid_ == other.original_writer_guid_) &&
               (sequence_number_ == other.sequence_number_);
    }

    bool operator !=(
            const OriginalWriterInfo& other) const
    {
        return !(*this == other);
    }

private:

    GUID_t original_writer_guid_ = GUID_t::unknown();

    SequenceNumber_t sequence_number_ = SequenceNumber_t::unknown();
};

} //namespace rtps
} //namespace fastdds
} //namespace eprosima
#endif // ifndef FASTDDS_RTPS_COMMON__ORIGINALWRITERINFO_HPP
