#ifndef _CONTENTFILTEREDTOPICEXAMPLE_MYCUSTOMFILTER_HPP_
#define _CONTENTFILTEREDTOPICEXAMPLE_MYCUSTOMFILTER_HPP_

#include <fastcdr/Cdr.h>

#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastdds/rtps/common/CdrSerialization.hpp>

//! Custom filter class
//! It requieres two parameters 'low_mark_' and 'high_mark_'.
//! Filter samples which index is lower than 'low_mark_' and higher than 'high_mark_'.
class MyCustomFilter : public eprosima::fastdds::dds::IContentFilter
{
public:

    /**
     * @brief Construct a new MyCustomFilter object
     *
     * @param low_mark
     * @param high_mark
     */
    MyCustomFilter(
            int low_mark,
            int high_mark)
        : low_mark_(low_mark)
        , high_mark_(high_mark)
    {
    }

    //! Destructor
    virtual ~MyCustomFilter() = default;

    /**
     * @brief Evaluate filter discriminating whether the sample is relevant or not, i.e. whether it meets the filtering
     * criteria
     *
     * @param payload Serialized sample
     * @return true if sample meets filter requirements. false otherwise.
     */
    bool evaluate(
            const SerializedPayload& payload,
            const FilterSampleInfo& /*sample_info*/,
            const GUID_t& /*reader_guid*/) const override
    {
        // Deserialize the `index` field from the serialized sample.
        eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload.data), payload.length);
        eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                eprosima::fastdds::rtps::DEFAULT_XCDR_VERSION);
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

    //! Low mark: lower threshold below which the samples are relevant
    uint32_t low_mark_ = 0;
    //! High mark: upper threshold over which the samples are relevant
    uint32_t high_mark_ = 0;

};

#endif // _CONTENTFILTEREDTOPICEXAMPLE_MYCUSTOMFILTER_HPP_
