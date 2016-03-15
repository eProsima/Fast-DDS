#ifndef _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKPARENTEVENT_H_
#define  _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKPARENTEVENT_H_

#include <fastrtps/rtps/resources/TimedEvent.h>
#include "MockEvent.h"

#include <boost/asio.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/atomic.hpp>
#include <boost/thread.hpp>

class MockParentEvent : public eprosima::fastrtps::rtps::TimedEvent
{
    public:

        MockParentEvent(boost::asio::io_service &service, const boost::thread& event_thread, double milliseconds, unsigned int countUntilDestruction,
                TimedEvent::AUTODESTRUCTION_MODE autodestruction = TimedEvent::NONE);

        virtual ~MockParentEvent();

        void event(EventCode code, const char* msg= nullptr);

        bool wait(unsigned int milliseconds);

        boost::atomic_int successed_;
        boost::atomic_int cancelled_;
        static int destructed_;
        static boost::mutex destruction_mutex_;
        static boost::condition_variable destruction_cond_;

    private:

        int sem_count_;
        boost::mutex sem_mutex_;
        boost::condition_variable sem_cond_;
        MockEvent *event_;
        unsigned int countUntilDestruction_;
        unsigned int currentCount_;
};

#endif // _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKPARENTEVENT_H_

