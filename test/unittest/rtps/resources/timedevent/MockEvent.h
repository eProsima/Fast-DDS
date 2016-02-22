#ifndef _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKEVENT_H_
#define  _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKEVENT_H_

#include <fastrtps/rtps/resources/TimedEvent.h>

#include <boost/asio.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/atomic.hpp>
#include <boost/thread.hpp>

class MockEvent : public eprosima::fastrtps::rtps::TimedEvent
{
    public:

        MockEvent(boost::asio::io_service &service, const boost::thread& event_thread, double milliseconds, bool autorestart, TimedEvent::AUTODESTRUCTION_MODE autodestruction = TimedEvent::NONE);

        virtual ~MockEvent();

        void event(EventCode code, const char* msg= nullptr);

        bool wait(unsigned int milliseconds);

        boost::atomic_int successed_;
        boost::atomic_int cancelled_;
        static int destructed_;
        static boost::mutex destruction_mutex_;
        static boost::condition_variable destruction_cond_;

    private:

        boost::interprocess::interprocess_semaphore semaphore_;
        bool autorestart_;
};

#endif // _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKEVENT_H_
