#ifndef _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKEVENT_H_
#define  _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKEVENT_H_

#include <fastrtps/rtps/resources/TimedEvent.h>

#include <boost/asio.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/atomic.hpp>

class MockEvent : public eprosima::fastrtps::rtps::TimedEvent
{
    public:

        MockEvent(boost::asio::io_service *service, double milliseconds, TimedEvent::AUTODESTRUCTION_MODE autodestruction = TimedEvent::NONE);

        virtual ~MockEvent();

        void event(EventCode code, const char* msg= nullptr);

        void wait();

        boost::atomic_int successed_;
        boost::atomic_int cancelled_;

    private:

        boost::interprocess::interprocess_semaphore semaphore_;
};

#endif // _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKEVENT_H_
