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

/**
 * @file CustomTopicManager.hpp
 */

#ifndef FASTDDS_DDS_TOPIC__CUSTOMTOPICMANAGER_HPP
#define FASTDDS_DDS_TOPIC__CUSTOMTOPICMANAGER_HPP

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * @brief Interface for managing custom topics in Fast DDS.
 */
class CustomTopicManager
{
public:

    virtual ~CustomTopicManager() = default;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_TOPIC__CUSTOMTOPICMANAGER_HPP
