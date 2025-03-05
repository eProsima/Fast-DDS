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

#ifndef FASTDDS_RPC__REQUESTREPLYCONTENTFILTER_HPP
#define FASTDDS_RPC__REQUESTREPLYCONTENTFILTER_HPP

#include <fastdds/dds/topic/IContentFilter.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * @brief Content filter that allows to filter samples based on the GUID of the reader that received the sample
 * @note This filter is used to filter samples in a multiple requester - single replier service scenario.
 * Each requester will receive only the replies that match their requests
 */
class RequestReplyContentFilter : public IContentFilter
{
public:

    /**
     * @brief Constructor
     */
    RequestReplyContentFilter() = default;

    /**
     * @brief Destructor
     */
    virtual ~RequestReplyContentFilter() = default;

    /**
     * The sample is relevant if the reply reader GUID and the sample identity match.
     * (i.e: the reply received is the response of a request sent by this requester)
     */
    bool evaluate(
            const SerializedPayload& payload,
            const FilterSampleInfo& sample_info,
            const GUID_t& reader_guid) const override
    {
        static_cast<void>(payload);
        return sample_info.related_sample_identity.writer_guid().guidPrefix == reader_guid.guidPrefix;
    }

};

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RPC__REQUESTREPLYCONTENTFILTER_HPP
