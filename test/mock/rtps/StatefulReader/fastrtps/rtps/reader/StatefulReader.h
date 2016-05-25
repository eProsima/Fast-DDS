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
                    inline const GUID_t& getGuid() const { return guid_; };

                    inline ReaderTimes& getTimes(){return times_;};

                private:

                    GUID_t guid_;

                    ReaderTimes times_;
            };
        } // namespace rtps
    } // namespace fastrtps
} // namespace eprosima
#endif // _RTPS_READER_STATEFULREADER_H_
