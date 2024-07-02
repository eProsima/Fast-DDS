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

#ifndef FASTDDS_EXAMPLES_CPP_CONTENT_FILTER__CUSTOMCONTENTFILTERFACTORY_HPP
#define FASTDDS_EXAMPLES_CPP_CONTENT_FILTER__CUSTOMCONTENTFILTERFACTORY_HPP

#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastdds/dds/topic/IContentFilterFactory.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>

#include "CustomContentFilter.hpp"

//! Custom filter factory
class CustomContentFilterFactory : public eprosima::fastdds::dds::IContentFilterFactory
{
public:

    /**
     * @brief Create a ContentFilteredTopic using this factory.
     *        Updating the filter implies deleting the previous one and creating a new one using the new given
     *        parameters.
     *
     * @param filter_class_name Custom filter name
     * @param type_name Data type name
     * @param filter_parameters Parameters required by the filter
     * @param filter_instance Instance of the filter to be evaluated
     *
     * @return eprosima::fastdds::dds::RETCODE_BAD_PARAMETER if the requirements for creating the
     *         ContentFilteredTopic using this factory are not met
     *         eprosima::fastdds::dds::RETCODE_OK if the ContentFilteredTopic is correctly created
     */
    eprosima::fastdds::dds::ReturnCode_t create_content_filter(
            const char* filter_class_name, // My custom filter class name is 'MY_CUSTOM_FILTER'.
            const char* type_name, // This custom filter only supports one type: 'HelloWorld'.
            const eprosima::fastdds::dds::TopicDataType* /*data_type*/, // Not used in this implementation.
            const char* /*filter_expression*/, // This Custom Filter doesn't implement a filter expression.
            const ParameterSeq& filter_parameters, // Always need two parameters to be set: low_mark and high_mark.
            eprosima::fastdds::dds::IContentFilter*& filter_instance) override
    {
        // Check the ContentFilteredTopic should be created by this factory.
        if (0 != strcmp(filter_class_name, "CUSTOM_FILTER") ||
                // Check the ContentFilteredTopic is created for the unique type this Custom Filter supports.
                0 != strcmp(type_name, "HelloWorld") ||
                // Check that the two mandatory filter parameters were set.
                2 != filter_parameters.length())
        {
            return eprosima::fastdds::dds::RETCODE_BAD_PARAMETER;
        }

        // If there is an update, delete previous instance.
        if (nullptr != filter_instance)
        {
            delete(dynamic_cast<CustomContentFilter*>(filter_instance));
        }

        // Instantiation of the Custom Filter.
        filter_instance = new CustomContentFilter(std::stoi(filter_parameters[0]), std::stoi(filter_parameters[1]));

        return eprosima::fastdds::dds::RETCODE_OK;
    }

    /**
     * @brief Delete a ContentFilteredTopic created by this factory
     *
     * @param filter_class_name Custom filter name
     * @param filter_instance Instance of the filter to be deleted.
     *                        After returning, the passed pointer becomes invalid.
     * @return eprosima::fastdds::dds::RETCODE_BAD_PARAMETER if the instance was created with another
     *         factory
     *         eprosima::fastdds::dds::RETCODE_OK if correctly deleted
     */
    eprosima::fastdds::dds::ReturnCode_t delete_content_filter(
            const char* filter_class_name,
            eprosima::fastdds::dds::IContentFilter* filter_instance) override
    {
        // Check the ContentFilteredTopic should be created by this factory.
        if (0 != strcmp(filter_class_name, "CUSTOM_FILTER") ||
                // Check the filter instance is valid
                nullptr != filter_instance)
        {
            return eprosima::fastdds::dds::RETCODE_BAD_PARAMETER;
        }

        // Deletion of the Custom Filter.
        if (nullptr != filter_instance)
        {
            delete(dynamic_cast<CustomContentFilter*>(filter_instance));
        }

        return eprosima::fastdds::dds::RETCODE_OK;
    }

};
#endif // FASTDDS_EXAMPLES_CPP_CONTENT_FILTER__CUSTOMCONTENTFILTERFACTORY_HPP
