// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SharedDumpFunctions.hpp
 *
 */

#ifndef _SHARED_DUMP_FUNCTIONS_H_
#define _SHARED_DUMP_FUNCTIONS_H_

#include "json.hpp"

#include <fastdds/rtps/common/CacheChange.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb { 

using json = nlohmann::json;

template <typename T>
std::string object_to_string(const T& t)
{
    std::ostringstream stream;
    stream << t;
    return  stream.str();
}

const std::string b64encode(const unsigned char* data, const size_t &len);

void b64decode(unsigned char* data, const std::string input);

void to_json(json& j, const eprosima::fastrtps::rtps::CacheChange_t& change);

void from_json(const json& j, eprosima::fastrtps::rtps::CacheChange_t& change);

// INFO TO STORE IN DDB
/*
{
    "participants":{
        <guid_prefix>:{
            "change":{
                "kind":<kind>,
                "writer_GUID":<writerGUID>,
                "instance_handle":<instanceHandle>,
                "sequence_number":<sequenceNumber>,
                "source_timestamp":<sourceTimestamp>,
                "reception_timestamp":<receptionTimestamp>,
                "sample_identity":<sample_identity>
            },
            "ack_status":{
                <guid_prefix>:<is_acked>
            }
            "metatraffic_locators":{<metatraffic_locators>}
            "is_client":<is_client>,
            "is_local":<is_local>
        }
    },
    "writers":{
        <guid>:{
            "change":{
                "kind":<kind>,
                "writer_GUID":<writerGUID>,
                "instance_handle":<instanceHandle>,
                "sequence_number":<sequenceNumber>,
                "source_timestamp":<sourceTimestamp>,
                "reception_timestamp":<receptionTimestamp>,
                "sample_identity":<sample_identity>
            },
            "ack_status":{
                <guid_prefix>:<is_acked>
            }
            "topic":<topic>
        }
    },
    "readers":{
        <guid>:{
            "change":{
                "kind":<kind>,
                "writer_GUID":<writerGUID>,
                "instance_handle":<instanceHandle>,
                "sequence_number":<sequenceNumber>,
                "source_timestamp":<sourceTimestamp>,
                "reception_timestamp":<receptionTimestamp>,
                "sample_identity":<sample_identity>
            },
            "ack_status":{
                <guid_prefix>:<is_acked>
            }
            "topic":<topic>
        }
    },
    "server_prefix":<server_prefix>,
    "version":<version>
}
*/

} /* ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */


#endif /* _SHARED_DUMP_FUNCTIONS_H_ */
