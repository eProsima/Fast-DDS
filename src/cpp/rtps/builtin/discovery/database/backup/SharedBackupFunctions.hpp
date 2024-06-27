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

#include <nlohmann/json.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {

using json = nlohmann::json;

template <typename T>
std::string object_to_string(
        const T& t)
{
    std::ostringstream stream;
    stream << t;
    return stream.str();
}

// Encode bytes into b64 string
const std::string b64encode(
        const unsigned char* data,
        const size_t& len);

// Decode a string in b64 into an array of bytes
void b64decode(
        unsigned char* data,
        const std::string input);

// Writes the info from a change into a json object
void to_json(
        json& j,
        const CacheChange_t& change);

// Deserialize a cacheChange from a json object. The change must have been
// already created from a pool and reserved the payload length
void from_json(
        const json& j,
        CacheChange_t& change);

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
                "sample_identity":<sample_identity>,
                "serialized_payload":{
                    "encapsulation":<encapsulation>
                    "length":<length>
                    "data":b64x<data>
                },
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
    }
   }
 */

} /* ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */


#endif /* _SHARED_DUMP_FUNCTIONS_H_ */
