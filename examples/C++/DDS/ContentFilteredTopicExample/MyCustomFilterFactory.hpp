#ifndef _CONTENTFILTEREDTOPICEXAMPLE_MYCUSTOMFILTERFACTORY_HPP_
#define _CONTENTFILTEREDTOPICEXAMPLE_MYCUSTOMFILTERFACTORY_HPP_

#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastdds/dds/topic/IContentFilterFactory.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>

#include "MyCustomFilter.hpp"

//! Custom filter factory
class MyCustomFilterFactory : public eprosima::fastdds::dds::IContentFilterFactory
{
public:

    /**
     * @brief Create a ContentFilteredTopic using this factory. Updating the filter implies deleting the previous one
     *        and creating a new one using the new given parameters.
     *
     * @param filter_class_name Custom filter name
     * @param type_name Data type name
     * @param filter_parameters Parameters required by the filter
     * @param filter_instance Instance of the filter to be evaluated
     */
    eprosima::fastrtps::types::ReturnCode_t create_content_filter(
            const char* filter_class_name, // My custom filter class name is 'MY_CUSTOM_FILTER'.
            const char* type_name, // This custom filter only supports one type: 'HelloWorld'.
            const eprosima::fastdds::dds::TopicDataType* /*data_type*/, // Not used in this implementation.
            const char* /*filter_expression*/, // This Custom Filter doesn't implement a filter expression.
            const ParameterSeq& filter_parameters, // Always need two parameters to be set: low_mark and high_mark.
            eprosima::fastdds::dds::IContentFilter*& filter_instance) override
    {
        // Check the ContentFilteredTopic should be created by this factory.
        if (0 != strcmp(filter_class_name, "MY_CUSTOM_FILTER") ||
                // Check the ContentFilteredTopic is created for the unique type this Custom Filter supports.
                0 != strcmp(type_name, "HelloWorld") ||
                // Check that the two mandatory filter parameters were set.
                2 != filter_parameters.length())
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

    //! Delete a ContentFilteredTopic created by this factory
    eprosima::fastrtps::types::ReturnCode_t delete_content_filter(
            const char* filter_class_name,
            eprosima::fastdds::dds::IContentFilter* filter_instance) override
    {
        // Check the ContentFilteredTopic should be created by this factory.
        if (0 != strcmp(filter_class_name, "MY_CUSTOM_FILTER") ||
                // Check the filter instance is valid
                nullptr != filter_instance)
        {
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }

        // Deletion of the Custom Filter.
        delete(dynamic_cast<MyCustomFilter*>(filter_instance));

        return ReturnCode_t::RETCODE_OK;
    }

};
#endif // _CONTENTFILTEREDTOPICEXAMPLE_MYCUSTOMFILTERFACTORY_HPP_
