#include "MockParentEvent.h"

int MockParentEvent::destructed_ = 0;
boost::mutex MockParentEvent::destruction_mutex_;
boost::condition_variable MockParentEvent::destruction_cond_;

MockParentEvent::MockParentEvent(boost::asio::io_service& service, const boost::thread& event_thread, double milliseconds, unsigned int countUntilDestruction,
        TimedEvent::AUTODESTRUCTION_MODE autodestruction) : 
    TimedEvent(service, event_thread, milliseconds, autodestruction), successed_(0), cancelled_(0), sem_count_(0),
    event_(nullptr), countUntilDestruction_(countUntilDestruction), currentCount_(0)
{
    event_ = new MockEvent(service, event_thread, milliseconds / 2.0, false, autodestruction);
    event_->restart_timer();
}

MockParentEvent::~MockParentEvent()
{
    if(event_ != nullptr)
    {
        delete event_;
        event_ = nullptr;
    }

    destroy();

    destruction_mutex_.lock();
    ++destructed_;
    destruction_mutex_.unlock();
    destruction_cond_.notify_one();
}

void MockParentEvent::event(EventCode code, const char* msg)
{
    (void)msg;

    if(code == EventCode::EVENT_SUCCESS)
    {
        successed_.fetch_add(1, boost::memory_order_relaxed);

        if(event_ != nullptr)
        {
            event_->restart_timer();

            if(++currentCount_ == countUntilDestruction_)
            {
                delete event_;
                event_ = nullptr;
            }
        }

        restart_timer();

    }
    else if(code == EventCode::EVENT_ABORT)
        cancelled_.fetch_add(1, boost::memory_order_relaxed);

    sem_mutex_.lock();
    ++sem_count_;
    sem_mutex_.unlock();
    sem_cond_.notify_one();
}

bool MockParentEvent::wait(unsigned int milliseconds)
{
    boost::unique_lock<boost::mutex> lock(sem_mutex_);

    if(sem_count_ == 0)
    {
        if(sem_cond_.wait_for(lock, boost::chrono::milliseconds(milliseconds)) != boost::cv_status::no_timeout)
            return false;
    }

    --sem_count_;
    return true;
}

