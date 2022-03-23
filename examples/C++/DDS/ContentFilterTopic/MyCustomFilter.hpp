#ifndef _MYCUSTOMFILTER_HPP_
#define _MYCUSTOMFILTER_HPP_

#include <fastdds/dds/topic/IContentFilter.hpp>

#include <fastcdr/Cdr.h>

class MyCustomFilter : public eprosima::fastdds::dds::IContentFilter
{
public:

    MyCustomFilter(
            int low_mark,
            int high_mark)
        : low_mark_(low_mark)
        , high_mark_(high_mark)
    {
    }

    virtual ~MyCustomFilter() = default;

    bool evaluate(
            const SerializedPayload& payload,
            const FilterSampleInfo& /*sample_info*/,
            const GUID_t& /*reader_guid*/) const override
    {
        // Deserialize the `index` field from the serialized sample.
        eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload.data), payload.length);
        eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                eprosima::fastcdr::Cdr::DDS_CDR);
        // Deserialize encapsulation.
        deser.read_encapsulation();
        uint32_t index = 0;

        // Deserialize `index` field.
        try
        {
            deser >> index;
        }
        catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
        {
            return false;
        }

        // Custom filter: reject samples where index > low_mark_ and index < high_mark_.
        if (index < low_mark_ || index > high_mark_)
        {
            return true;
        }

        return false;
    }

private:

    uint32_t low_mark_ = 0;
    uint32_t high_mark_ = 0;

};

#endif // _MYCUSTOMFILTER_HPP_
