#ifndef _RTPS_READER_STATEFULREADER_H_
#define _RTPS_READER_STATEFULREADER_H_

#include <fastrtps/rtps/attributes/ReaderAttributes.h>

namespace eprosima
{
    namespace fastrtps
    {
        namespace rtps
        {
            class StatefulReader
            {
                public:

                    inline ReaderTimes& getTimes(){return m_times;};

                private:

                    ReaderTimes m_times;
            };
        } // namespace rtps
    } // namespace fastrtps
} // namespace eprosima
#endif // _RTPS_READER_STATEFULREADER_H_
