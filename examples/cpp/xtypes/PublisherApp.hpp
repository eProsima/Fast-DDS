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

/**
 * @file PublisherApp.hpp
 *
 */

#ifndef FASTDDS_EXAMPLES_CPP_XTYPES__PUBLISHERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_XTYPES__PUBLISHERAPP_HPP

#include <condition_variable>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace xtypes {

class PublisherApp : public Application, public DataWriterListener
{
public:

    PublisherApp(
            const CLIParser::config& config,
            const std::string& topic_name);

    ~PublisherApp();

    //! Publisher matched method
    void on_publication_matched(
            DataWriter* writer,
            const PublicationMatchedStatus& info) override;

    //! Run publisher
    void run() override;

    //! Stop publisher
    void stop() override;

private:

    //! Return the current state of execution
    bool is_stopped();

    //! Publish a sample
    bool publish();

    //! Create the dynamic type used by the PublisherApp
    static DynamicType::_ref_type create_type(
            bool use_xml_type);

    //! Auxilary function to get a uint32_t value from a DynamicData object
    static uint32_t get_uint32_value(
            const DynamicData::_ref_type data,
            const std::string& member_name);

    //! Auxilary function to set a uint32_t value in a DynamicData object
    static void set_uint32_value(
            DynamicData::_ref_type data,
            const std::string& member_name,
            const uint32_t value);

    //! Auxilary function to get a string value from a DynamicData object
    static std::string get_string_value(
            const DynamicData::_ref_type data,
            const std::string& member_name);

    DynamicData::_ref_type hello_;

    DomainParticipant* participant_;

    Publisher* publisher_;

    Topic* topic_;

    DataWriter* writer_;

    int16_t matched_;

    uint16_t samples_;

    std::mutex mutex_;

    std::condition_variable cv_;

    std::atomic<bool> stop_;

    const uint32_t period_ms_ = 100; // in ms
};

} // namespace xtypes
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_XTYPES__PUBLISHERAPP_HPP
