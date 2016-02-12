#include "MockEvent.h"

int MockEvent::destructed_ = 0;
boost::mutex MockEvent::destruction_mutex_;
boost::condition_variable MockEvent::destruction_cond_;

MockEvent::MockEvent(boost::asio::io_service *service, double milliseconds, bool autorestart, TimedEvent::AUTODESTRUCTION_MODE autodestruction) : 
    TimedEvent(service, milliseconds, autodestruction), successed_(0), cancelled_(0), semaphore_(0), autorestart_(autorestart)
{
}

MockEvent::~MockEvent()
{
    destruction_mutex_.lock();
    ++destructed_;
    destruction_mutex_.unlock();
    destruction_cond_.notify_one();
}

void MockEvent::event(EventCode code, const char* msg)
{
    (void)msg;

    if(code == EventCode::EVENT_SUCCESS)
    {
        successed_.fetch_add(1, boost::memory_order_relaxed);

        if(autorestart_)
            restart_timer();
    }
    else if(code == EventCode::EVENT_ABORT)
        cancelled_.fetch_add(1, boost::memory_order_relaxed);

    semaphore_.post();
}

bool MockEvent::wait(unsigned int milliseconds)
{
    boost::system_time const timeout = boost::get_system_time() + boost::posix_time::milliseconds(milliseconds);
    return semaphore_.timed_wait(timeout);
}
