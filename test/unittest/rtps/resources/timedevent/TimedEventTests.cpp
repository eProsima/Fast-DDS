#include "MockEvent.h"

#include <boost/thread.hpp>
#include <gtest/gtest.h>
#include <asm/uaccess.h>

class TimedEventEnvironment : public ::testing::Environment
{
    public:

        TimedEventEnvironment() : work_(service_) {}

        void SetUp()
        {
            thread_ = new boost::thread(&TimedEventEnvironment::run, this);
        }

        void TearDown()
        {
            service_.stop();
            thread_->join();
        }

        void run()
        {
            service_.run();
        }

        boost::asio::io_service service_;

        boost::asio::io_service::work work_;

        boost::thread *thread_;
};

TimedEventEnvironment* const env = dynamic_cast<TimedEventEnvironment*>(testing::AddGlobalTestEnvironment(new TimedEventEnvironment));

TEST(TimedEvent, EventNonAutoDestruc_SuccessEvents)
{
    MockEvent event(&env->service_, 100);

    for(int i = 0; i < 10; ++i)
    {
        event.restart_timer();

        event.wait();
    }

    int successed = event.successed_.load(boost::memory_order_relaxed);

    ASSERT_EQ(successed, 10);
}

TEST(TimedEvent, EventNonAutoDestruc_CancelEvents)
{
    MockEvent event(&env->service_, 100);

    for(int i = 0; i < 10; ++i)
    {
        event.restart_timer();
        event.cancel_timer();

        event.wait();
    }

    int cancelled = event.cancelled_.load(boost::memory_order_relaxed);

    ASSERT_EQ(cancelled, 10);
}

TEST(TimedEvent, EventNonAutoDestruc_RestartEvents)
{
    MockEvent event(&env->service_, 100);

    for(int i = 0; i < 10; ++i)
    {
        event.restart_timer();
    }

    for(int i = 0; i < 10; ++i)
    {
        event.wait();
    }

    int successed = event.successed_.load(boost::memory_order_relaxed);

    ASSERT_EQ(successed, 1);

    int cancelled = event.cancelled_.load(boost::memory_order_relaxed);

    ASSERT_EQ(cancelled, 9);
}

TEST(TimedEvent, EventOnSuccessAutoDestruc_SuccessEvents)
{
    MockEvent *event = new MockEvent(&env->service_, 100);

    event->restart_timer();
    event->wait();
    usleep(10);

    //ASSERT_EQ(access_ok(VERIFY_READ, event, sizeof(MockEvent)), 0);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
#if defined(WIN32) && defined(_DEBUG)
    eprosima::Log::setVerbosity(eprosima::LOG_VERBOSITY_LVL::VERB_ERROR);
#endif
    return RUN_ALL_TESTS();
}
