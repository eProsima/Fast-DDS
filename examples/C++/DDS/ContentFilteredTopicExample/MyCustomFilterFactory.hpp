#ifndef _CONTENTFILTEREDTOPICEXAMPLE_MYCUSTOMFILTERFACTORY_HPP_
#define _CONTENTFILTEREDTOPICEXAMPLE_MYCUSTOMFILTERFACTORY_HPP_

#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastdds/dds/topic/IContentFilterFactory.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>

#include "MyCustomFilter.hpp"

class MyCustomFilterFactory : public eprosima::fastdds::dds::IContentFilterFactory
{
public:

    eprosima::fastrtps::types::ReturnCode_t create_content_filter(
            const char* filter_class_name, // My custom filter class name is 'MY_CUSTOM_FILTER'.
            const char* type_name, // This custom filter only supports one type: 'HelloWorld'.
            const eprosima::fastdds::dds::TopicDataType* /*data_type*/, // Not used in this implementation.
            const char* /*filter_expression*/, // This Custom Filter doesn't implement a filter expression.
            const ParameterSeq& filter_parameters, // Always need two parameters to be set: low_mark and high_mark.
            eprosima::fastdds::dds::IContentFilter*& filter_instance) override
    {
        // Check the ContentFilteredTopic should be created by my factory.
        if (0 != strcmp(filter_class_name, "MY_CUSTOM_FILTER"))
        {
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }

        // Check the ContentFilteredTopic is created for the unique type this Custom Filter supports.
        if (0 != strcmp(type_name, "HelloWorld"))
        {
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }

        // Checks there were set the two mandatory filter parameters.
        if (2 != filter_parameters.length())
        {
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }

        // If there is an update, delete previous instance.
        if (nullptr != filter_instance)
        {
            delete(dynamic_cast<MyCustomFilter*>(filter_instance));
        }

        // Instantiation of the Custom Filter.
        filter_instance = new MyCustomFilter(std::stoi(filter_parameters[0]), std::stoi(filter_parameters[1]));

        return ReturnCode_t::RETCODE_OK;
    }

    eprosima::fastrtps::types::ReturnCode_t delete_content_filter(
            const char* filter_class_name,
            eprosima::fastdds::dds::IContentFilter* filter_instance) override
    {
        // Check the ContentFilteredTopic should be created by my factory.
        if (0 != strcmp(filter_class_name, "MY_CUSTOM_FILTER"))
        {
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }

        // Deletion of the Custom Filter.
        delete(dynamic_cast<MyCustomFilter*>(filter_instance));

        return ReturnCode_t::RETCODE_OK;
    }

};
#endif // _CONTENTFILTEREDTOPICEXAMPLE_MYCUSTOMFILTERFACTORY_HPP_
