#ifndef _RTPS_READER_TIMEDEVENT_HEARTBEATRESPONSEDELAY_H_
#define _RTPS_READER_TIMEDEVENT_HEARTBEATRESPONSEDELAY_H_

namespace eprosima
{
    namespace fastrtps
    {
        namespace rtps
        {
            // Forward declarations
            class WriterProxy;

            class HeartbeatResponseDelay
            {
                public:

                    HeartbeatResponseDelay(WriterProxy* /*wp*/,double /*interval*/)
                    {
                    }
            };
        } // namespace rtps
    } // namespace fastrtps
} // namespace eprosima
#endif // _RTPS_READER_TIMEDEVENT_HEARTBEATRESPONSEDELAY_H_
