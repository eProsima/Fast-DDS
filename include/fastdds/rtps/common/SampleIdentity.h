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
 * @file SampleIdentity.h
 */

#ifndef _FASTDDS_RTPS_COMMON_SAMPLEIDENTITY_H_
#define _FASTDDS_RTPS_COMMON_SAMPLEIDENTITY_H_

#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/SequenceNumber.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/*!
 * @brief This class is used to specify a sample
 * @ingroup COMMON_MODULE
 */
class RTPS_DllAPI SampleIdentity
{
public:

    /*!
     * @brief Default constructor. Constructs an unknown SampleIdentity.
     */
    SampleIdentity()
        : writer_guid_(GUID_t::unknown())
        , sequence_number_(SequenceNumber_t::unknown())
    {
    }

    /*!
     * @brief Copy constructor.
     */
    SampleIdentity(
            const SampleIdentity& sample_id)
        : writer_guid_(sample_id.writer_guid_)
        , sequence_number_(sample_id.sequence_number_)
    {
    }

    /*!
     * @brief Move constructor.
     */
    SampleIdentity(
            SampleIdentity&& sample_id)
        : writer_guid_(std::move(sample_id.writer_guid_))
        , sequence_number_(std::move(sample_id.sequence_number_))
    {
    }

    /*!
     * @brief Assignment operator.
     */
    SampleIdentity& operator =(
            const SampleIdentity& sample_id)
    {
        writer_guid_ = sample_id.writer_guid_;
        sequence_number_ = sample_id.sequence_number_;
        return *this;
    }

    /*!
     * @brief Move constructor.
     */
    SampleIdentity& operator =(
            SampleIdentity&& sample_id)
    {
        writer_guid_ = std::move(sample_id.writer_guid_);
        sequence_number_ = std::move(sample_id.sequence_number_);
        return *this;
    }

    /*!
     * @brief
     */
    bool operator ==(
            const SampleIdentity& sample_id) const
    {
        return (writer_guid_ == sample_id.writer_guid_) && (sequence_number_ == sample_id.sequence_number_);
    }

    /*!
     * @brief
     */
    bool operator !=(
            const SampleIdentity& sample_id) const
    {
        return !(*this == sample_id);
    }

    /**
     * @brief To allow using SampleIdentity as map key.
     * @param sample
     * @return
     */
    bool operator <(
            const SampleIdentity& sample) const
    {
        return writer_guid_ < sample.writer_guid_
               || (writer_guid_ == sample.writer_guid_
               && sequence_number_ < sample.sequence_number_);
    }

    SampleIdentity& writer_guid(
            const GUID_t& guid)
    {
        writer_guid_ = guid;
        return *this;
    }

    SampleIdentity& writer_guid(
            GUID_t&& guid)
    {
        writer_guid_ = std::move(guid);
        return *this;
    }

    const GUID_t& writer_guid() const
    {
        return writer_guid_;
    }

    GUID_t& writer_guid()
    {
        return writer_guid_;
    }

    SampleIdentity& sequence_number(
            const SequenceNumber_t& seq)
    {
        sequence_number_ = seq;
        return *this;
    }

    SampleIdentity& sequence_number(
            SequenceNumber_t&& seq)
    {
        sequence_number_ = std::move(seq);
        return *this;
    }

    const SequenceNumber_t& sequence_number() const
    {
        return sequence_number_;
    }

    SequenceNumber_t& sequence_number()
    {
        return sequence_number_;
    }

    static SampleIdentity unknown()
    {
        return SampleIdentity();
    }

private:

    GUID_t writer_guid_;

    SequenceNumber_t sequence_number_;

    friend std::istream& operator >>(
            std::istream& input,
            SampleIdentity& sid);
    friend std::ostream& operator <<(
            std::ostream& output,
            const SampleIdentity& sid);
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * Stream operator, retrieves a GUID.
 * @param input Input stream.
 * @param sid SampleIdentity to read.
 * @return Stream operator.
 */
inline std::istream& operator >>(
        std::istream& input,
        SampleIdentity& sid)
{
    std::istream::sentry s(input);

    if (s)
    {
        std::ios_base::iostate excp_mask = input.exceptions();

        try
        {
            input.exceptions(excp_mask | std::ios_base::failbit | std::ios_base::badbit);

            char sep;
            input >> sid.writer_guid_ >> sep >> sid.sequence_number_;

            if (sep != '|')
            {
                input.setstate(std::ios_base::failbit);
            }
        }
        catch (std::ios_base::failure&)
        {
            // maybe is unknown or just invalid
            sid.writer_guid_ = GUID_t::unknown();
            sid.sequence_number_ = SequenceNumber_t::unknown();
        }

        input.exceptions(excp_mask);
    }

    return input;
}

/**
 * Stream operator, prints a GUID.
 * @param output Output stream.
 * @param sid SampleIdentity to print.
 * @return Stream operator.
 */
inline std::ostream& operator <<(
        std::ostream& output,
        const SampleIdentity& sid)
{
    output << sid.writer_guid_ << '|' << sid.sequence_number_;

    return output;
}

#endif // DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // _FASTDDS_RTPS_COMMON_SAMPLEIDENTITY_H_
