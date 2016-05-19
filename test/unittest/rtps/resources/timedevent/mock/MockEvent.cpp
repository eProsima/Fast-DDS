#include "MockEvent.h"

int MockEvent::destructed_ = 0;
std::mutex MockEvent::destruction_mutex_;
std::condition_variable MockEvent::destruction_cond_;

MockEvent::MockEvent(boost::asio::io_service& service, const boost::thread& event_thread, double milliseconds, bool autorestart, TimedEvent::AUTODESTRUCTION_MODE autodestruction) : 
    TimedEvent(service, event_thread, milliseconds, autodestruction), successed_(0), cancelled_(0), sem_count_(0), autorestart_(autorestart)
{
}

MockEvent::~MockEvent()
{
    destroy();

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
        successed_.fetch_add(1, std::memory_order_relaxed);

        if(autorestart_)
            restart_timer();
    }
    else if(code == EventCode::EVENT_ABORT)
        cancelled_.fetch_add(1, std::memory_order_relaxed);

    sem_mutex_.lock();
    ++sem_count_;
    sem_mutex_.unlock();
    sem_cond_.notify_one();
}

bool MockEvent::wait(unsigned int milliseconds)
{
    std::unique_lock<std::mutex> lock(sem_mutex_);

    if(sem_count_ == 0)
    {
        if(sem_cond_.wait_for(lock, std::chrono::milliseconds(milliseconds)) != std::cv_status::no_timeout)
            return false;
    }

    --sem_count_;
    return true;
}
