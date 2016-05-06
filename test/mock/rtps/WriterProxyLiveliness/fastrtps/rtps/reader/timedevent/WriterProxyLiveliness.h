#ifndef _RTPS_READER_TIMEDEVENT_WRITERPROXYLIVELINESS_H_
#define _RTPS_READER_TIMEDEVENT_WRITERPROXYLIVELINESS_H_

#include <gmock/gmock.h>

namespace eprosima
{
    namespace fastrtps
    {
        namespace rtps
        {
            // Forward declarations
            class WriterProxy;

            class WriterProxyLiveliness
            {
                public:

                    WriterProxyLiveliness(WriterProxy* /*wp*/, double /*interval*/)
                    {
                    }

                    MOCK_METHOD0(restart_timer, void());

                    MOCK_METHOD0(cancel_timer, void());
            };
        } //namespace rtps
    } //namespace fastrtps
} //namespace eprosima

#endif // _RTPS_READER_TIMEDEVENT_WRITERPROXYLIVELINESS_H_
