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

// HEADER ONLY FILE

#ifndef _SHARED_DUMP_FUNCTIONS_H_
#define _SHARED_DUMP_FUNCTIONS_H_

#include "json.hpp"

#include <fastdds/rtps/common/CacheChange.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

using json = nlohmann::json;

template <typename T>
std::string objectToString(const T& t)
{
    std::ostringstream stream;
    stream << t;
    return  stream.str();
}

template <typename T>
std::string vectorToString(const std::vector<T> v)
{
    std::ostringstream stream;
    std::vector<std::string> str_vec;
    for (T t : v)
    {
        stream << t << ";";
    }
    return stream.str();
}

template <typename T>
json vectorToJson(const std::vector<T> v)
{
    json j;
    std::vector<std::string> str_vec;
    int i = 0;
    // it creates a map to make every collection maps
    for (T t : v)
    {
        j[std::to_string(i)] = objectToString(t);
        ++i;
    }
    return j;
}

// json cacheChangeToJson(eprosima::fastrtps::rtps::CacheChange_t* change);

static json cacheChangeToJson(eprosima::fastrtps::rtps::CacheChange_t* change)
{
    json j;
    j["entity_guid"] = objectToString(fastrtps::rtps::iHandle2GUID(change->instanceHandle));
    j["writer_guid"] = objectToString(change->write_params.sample_identity().writer_guid());
    j["origin_sequence_number"] = objectToString(change->write_params.sample_identity().sequence_number());
    j["sequence_number"] = objectToString(change->sequenceNumber);
    j["kind"] = change->kind;
    
    return j;
}

static json changeVectorToJson(std::vector<eprosima::fastrtps::rtps::CacheChange_t*> v)
{
    json j;
    std::vector<std::string> str_vec;
    int i=0;
    for (eprosima::fastrtps::rtps::CacheChange_t* c : v)
    {
        j[std::to_string(i)] = cacheChangeToJson(c);
        ++i;
    }
    return j;
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */


#endif /* _SHARED_DUMP_FUNCTIONS_H_ */
