// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_EXAMPLES_CPP_CONTENT_FILTER__CUSTOMCONTENTFILTER_HPP
#define FASTDDS_EXAMPLES_CPP_CONTENT_FILTER__CUSTOMCONTENTFILTER_HPP

#include <fastcdr/Cdr.h>

#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastdds/rtps/common/CdrSerialization.hpp>

//! Custom filter class
//! It requieres two parameters 'low_mark_' and 'high_mark_'.
//! Filter samples which index is lower than 'low_mark_' and higher than 'high_mark_'.
class CustomContentFilter : public eprosima::fastdds::dds::IContentFilter
{
public:

    /**
     * @brief Construct a new CustomContentFilter object
     *
     * @param low_mark
     * @param high_mark
     */
    CustomContentFilter(
            int low_mark,
            int high_mark)
        : low_mark_(low_mark)
        , high_mark_(high_mark)
    {
    }

    //! Destructor
    virtual ~CustomContentFilter() = default;

    /**
     * @brief Evaluate filter discriminating whether the sample is relevant or not, i.e. whether it meets the filtering
     * criteria
     *
     * @param payload Serialized sample
     * @return true if sample meets filter requirements. false otherwise.
     */
    bool evaluate(
            const SerializedPayload& payload,
            const FilterSampleInfo& /*sample_info*/,
            const GUID_t& /*reader_guid*/) const override
    {
        // Deserialize the `index` field from the serialized sample.
        eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload.data), payload.length);
        eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                eprosima::fastdds::rtps::DEFAULT_XCDR_VERSION);
        // Deserialize encapsulation.
        deser.read_encapsulation();
        uint32_t index = 0;

        // Deserialize `index` field.
        try
        {
            deser >> index;
        }
        catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
        {
            return false;
        }

        // Custom filter: reject samples where index > low_mark_ and index < high_mark_.
        if (index < low_mark_ || index > high_mark_)
        {
            return true;
        }

        return false;
    }

private:

    //! Low mark: lower threshold below which the samples are relevant
    uint32_t low_mark_ = 0;
    //! High mark: upper threshold over which the samples are relevant
    uint32_t high_mark_ = 0;

};

#endif // FASTDDS_EXAMPLES_CPP_CONTENT_FILTER__CUSTOMCONTENTFILTER_HPP
