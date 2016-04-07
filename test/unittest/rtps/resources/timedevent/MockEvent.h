#ifndef _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKEVENT_H_
#define  _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKEVENT_H_

#include <fastrtps/rtps/resources/TimedEvent.h>

#include <atomic>
#include <condition_variable>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

class MockEvent : public eprosima::fastrtps::rtps::TimedEvent
{
    public:

        MockEvent(boost::asio::io_service &service, const boost::thread& event_thread, double milliseconds, bool autorestart, TimedEvent::AUTODESTRUCTION_MODE autodestruction = TimedEvent::NONE);

        virtual ~MockEvent();

        void event(EventCode code, const char* msg= nullptr);

        bool wait(unsigned int milliseconds);

        std::atomic<int> successed_;
        std::atomic<int> cancelled_;
        static int destructed_;
        static std::mutex destruction_mutex_;
        static std::condition_variable destruction_cond_;

    private:

        int sem_count_;
        std::mutex sem_mutex_;
        std::condition_variable sem_cond_;
        bool autorestart_;
};

#endif // _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKEVENT_H_
