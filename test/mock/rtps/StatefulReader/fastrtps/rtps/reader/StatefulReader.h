#ifndef _RTPS_READER_STATEFULREADER_H_
#define _RTPS_READER_STATEFULREADER_H_

#include <fastrtps/rtps/attributes/ReaderAttributes.h>
#include <fastrtps/rtps/common/Guid.h>

namespace eprosima
{
    namespace fastrtps
    {
        namespace rtps
        {
            class StatefulReader
            {
                public:

                    // In real class, inherited from Endpoint base class.
                    inline const GUID_t& getGuid() const { return GUID_t(); };

                    inline ReaderTimes& getTimes(){return m_times;};

                private:

                    ReaderTimes m_times;
            };
        } // namespace rtps
    } // namespace fastrtps
} // namespace eprosima
#endif // _RTPS_READER_STATEFULREADER_H_
