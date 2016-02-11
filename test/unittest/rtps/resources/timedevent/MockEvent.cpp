#include "MockEvent.h"

MockEvent::MockEvent(boost::asio::io_service *service, double milliseconds, TimedEvent::AUTODESTRUCTION_MODE autodestruction) : 
    TimedEvent(service, milliseconds, autodestruction), successed_(0), cancelled_(0), semaphore_(0)
{
}

MockEvent::~MockEvent()
{
}

void MockEvent::event(EventCode code, const char* msg)
{
    (void)msg;

    if(code == EventCode::EVENT_SUCCESS)
        successed_.fetch_add(1, boost::memory_order_relaxed);
    else if(code == EventCode::EVENT_ABORT)
        cancelled_.fetch_add(1, boost::memory_order_relaxed);

    semaphore_.post();
}

void MockEvent::wait()
{
    semaphore_.wait();
}
