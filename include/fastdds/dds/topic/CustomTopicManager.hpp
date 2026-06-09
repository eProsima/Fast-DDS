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

#include <memory>
#include <string>
#include <unordered_map>

#include <fastdds/dds/topic/TopicDataType.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DataReader;
class UserDataQosPolicy;

/**
 * @brief Interface for managing custom topics in Fast DDS.
 */
class CustomTopicManager
{
public:

    using CustomTopicsMap = std::unordered_map<std::string, std::shared_ptr<TopicDataType::Context>>;

    virtual ~CustomTopicManager() = default;

    /**
     * @brief Callback invoked when a local DataReader is enabled, just before it is added to the discovery database.
     *
     * Allows for the configuration of custom topics and QoS settings for the DataReader.
     *
     * @param[in]     reader     The DataReader that has been enabled.
     * @param[in,out] user_data  The user data QoS policy of the DataReader.
     *
     * When a CustomTopicManager is set for a DataReader, this callback is invoked inside the enable() method of
     * the DataReader, just before the DataReader is added to the discovery database.
     *
     * @return A map where the keys are the names of custom topics and the values are shared pointers to their type
     *         support contexts. Additional DataReaders will be created for each custom topic, with the same QoS as the
     *         original DataReader.
     */
    virtual CustomTopicsMap on_local_reader_enabled(
            const DataReader& reader,
            UserDataQosPolicy& user_data) = 0;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_TOPIC__CUSTOMTOPICMANAGER_HPP
