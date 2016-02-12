#include "MockEvent.h"

#include <gtest/gtest.h>

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
            delete thread_;
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

        ASSERT_TRUE(event.wait(200));
    }

    int successed = event.successed_.load(boost::memory_order_relaxed);

    ASSERT_EQ(successed, 10);

    int cancelled = event.cancelled_.load(boost::memory_order_relaxed);

    ASSERT_EQ(cancelled, 0);
}

TEST(TimedEvent, EventNonAutoDestruc_CancelEvents)
{
    MockEvent event(&env->service_, 100);

    for(int i = 0; i < 10; ++i)
    {
        event.restart_timer();
        event.cancel_timer();

        ASSERT_TRUE(event.wait(200));
    }

    int successed = event.successed_.load(boost::memory_order_relaxed);

    ASSERT_EQ(successed, 0);

    int cancelled = event.cancelled_.load(boost::memory_order_relaxed);

    ASSERT_EQ(cancelled, 10);
}

TEST(TimedEvent, EventNonAutoDestruc_RestartEvents)
{
    MockEvent event(&env->service_, 100);

    for(int i = 0; i < 10; ++i)
        event.restart_timer();

    for(int i = 0; i < 10; ++i)
        ASSERT_TRUE(event.wait(200));

    int successed = event.successed_.load(boost::memory_order_relaxed);

    ASSERT_EQ(successed, 1);

    int cancelled = event.cancelled_.load(boost::memory_order_relaxed);

    ASSERT_EQ(cancelled, 9);
}

TEST(TimedEvent, EventOnSuccessAutoDestruc_SuccessEvents)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;
    MockEvent *event = new MockEvent(&env->service_, 100, eprosima::fastrtps::rtps::TimedEvent::ON_SUCCESS);

    event->restart_timer();
    
    boost::unique_lock<boost::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
        MockEvent::destruction_cond_.wait_for(lock, boost::chrono::seconds(1));

    ASSERT_EQ(MockEvent::destructed_, 1);
}

TEST(TimedEvent, EventOnSuccessAutoDestruc_CancelEvents)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;
    MockEvent *event = new MockEvent(&env->service_, 100, eprosima::fastrtps::rtps::TimedEvent::ON_SUCCESS);

    // Cancel ten times.
    for(int i = 0; i < 10; ++i)
    {
        event->restart_timer();
        event->cancel_timer();

        ASSERT_TRUE(event->wait(200));

        boost::unique_lock<boost::mutex> lock(MockEvent::destruction_mutex_);

        if(MockEvent::destructed_ != 1)
            MockEvent::destruction_cond_.wait_for(lock, boost::chrono::milliseconds(100));

        ASSERT_EQ(MockEvent::destructed_, 0);
    }

    int successed = event->successed_.load(boost::memory_order_relaxed);

    ASSERT_EQ(successed, 0);

    int cancelled = event->cancelled_.load(boost::memory_order_relaxed);

    ASSERT_EQ(cancelled, 10);

    // Last event will be successful.
    event->restart_timer();
    
    boost::unique_lock<boost::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
        MockEvent::destruction_cond_.wait_for(lock, boost::chrono::seconds(1));

    ASSERT_EQ(MockEvent::destructed_, 1);
}

TEST(TimedEvent, EventOnSuccessAutoDestruc_QuickCancelEvents)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;
    MockEvent *event = new MockEvent(&env->service_, 0, eprosima::fastrtps::rtps::TimedEvent::ON_SUCCESS);

    // Cancel ten times.
    for(int i = 0; i < 10; ++i)
    {
        event->restart_timer();
        event->cancel_timer();

        ASSERT_TRUE(event->wait(10));

        boost::unique_lock<boost::mutex> lock(MockEvent::destruction_mutex_);

        if(MockEvent::destructed_ != 1)
            MockEvent::destruction_cond_.wait_for(lock, boost::chrono::milliseconds(100));

        ASSERT_EQ(MockEvent::destructed_, 0);
    }

    int successed = event->successed_.load(boost::memory_order_relaxed);

    ASSERT_EQ(successed, 0);

    int cancelled = event->cancelled_.load(boost::memory_order_relaxed);

    ASSERT_EQ(cancelled, 10);

    // Last event will be successful.
    event->restart_timer();
    
    boost::unique_lock<boost::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
        MockEvent::destruction_cond_.wait_for(lock, boost::chrono::seconds(1));

    ASSERT_EQ(MockEvent::destructed_, 1);
}

TEST(TimedEvent, EventOnSuccessAutoDestruc_RestartEvents)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;
    MockEvent *event = new MockEvent(&env->service_, 100, eprosima::fastrtps::rtps::TimedEvent::ON_SUCCESS);

    for(int i = 0; i < 10; ++i)
        event->restart_timer();

    boost::unique_lock<boost::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
        MockEvent::destruction_cond_.wait_for(lock, boost::chrono::seconds(1));

    ASSERT_EQ(MockEvent::destructed_, 1);
}

TEST(TimedEvent, EventOnSuccessAutoDestruc_QuickRestartEvents)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;
    MockEvent *event = new MockEvent(&env->service_, 0, eprosima::fastrtps::rtps::TimedEvent::ON_SUCCESS);

    for(int i = 0; i < 10; ++i)
        event->restart_timer();

    boost::unique_lock<boost::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
        MockEvent::destruction_cond_.wait_for(lock, boost::chrono::seconds(1));

    ASSERT_EQ(MockEvent::destructed_, 1);
}

TEST(TimedEvent, EventAlwaysAutoDestruc_SuccessEvents)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;
    MockEvent *event = new MockEvent(&env->service_, 100, eprosima::fastrtps::rtps::TimedEvent::ALLWAYS);

    event->restart_timer();
    
    boost::unique_lock<boost::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
        MockEvent::destruction_cond_.wait_for(lock, boost::chrono::seconds(1));

    ASSERT_EQ(MockEvent::destructed_, 1);
}

TEST(TimedEvent, EventAlwaysAutoDestruc_CancelEvents)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;
    MockEvent *event = new MockEvent(&env->service_, 100, eprosima::fastrtps::rtps::TimedEvent::ALLWAYS);

    event->restart_timer();
    event->cancel_timer();

    boost::unique_lock<boost::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
        MockEvent::destruction_cond_.wait_for(lock, boost::chrono::milliseconds(100));

    ASSERT_EQ(MockEvent::destructed_, 1);
}

TEST(TimedEvent, EventAlwaysAutoDestruc_QuickCancelEvents)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;
    MockEvent *event = new MockEvent(&env->service_, 0, eprosima::fastrtps::rtps::TimedEvent::ALLWAYS);

    event->restart_timer();
    event->cancel_timer();

    boost::unique_lock<boost::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
        MockEvent::destruction_cond_.wait_for(lock, boost::chrono::milliseconds(100));

    ASSERT_EQ(MockEvent::destructed_, 1);
}

// Auxiliary functions to be run in multithread tests.
void restart(MockEvent *event, unsigned int num_loop, unsigned int sleep_time)
{
    for(unsigned int i = 0; i < num_loop; ++i)
    {
        event->restart_timer();

        if(sleep_time > 0)
            usleep(sleep_time);
    }
}

void cancel(MockEvent *event, unsigned int num_loop, unsigned int sleep_time)
{
    for(unsigned int i = 0; i < num_loop; ++i)
    {
        event->cancel_timer();

        if(sleep_time > 0)
            usleep(sleep_time);
    }
}

TEST(TimedEventMultithread, EventNonAutoDestruc_TwoStartTwoCancel)
{
    boost::thread *thr1 = nullptr, *thr2 = nullptr,
        *thr3 = nullptr, *thr4 = nullptr;

    MockEvent event(&env->service_, 2);

    // 2 Thread restarting and two thread cancel.
    // Thread 1 -> Restart 100 times waiting 100ms between each one.
    // Thread 2 -> Cancel 100 times waiting 102ms between each one.
    // Thread 3 -> Restart 80 times waiting 110ms between each one.
    // Thread 4 -> Cancel 80 times waiting 112ms between each one.

    thr1 = new boost::thread(restart, &event, 100, 100); 
    thr2 = new boost::thread(cancel, &event, 100, 102); 
    thr3 = new boost::thread(restart, &event, 80, 110); 
    thr4 = new boost::thread(cancel, &event, 80, 112); 

    thr1->join();
    thr2->join();
    thr3->join();
    thr4->join();

    delete thr1;
    delete thr2;
    delete thr3;
    delete thr4;

    // Wait all expected times
    for(unsigned int i = 0; i < 180; ++i)
        ASSERT_TRUE(event.wait(120));

    int successed = event.successed_.load(boost::memory_order_relaxed);
    int cancelled = event.cancelled_.load(boost::memory_order_relaxed);

    ASSERT_EQ(successed + cancelled, 180);
}

TEST(TimedEventMultithread, EventNonAutoDestruc_QuickTwoStartTwoCancel)
{
    boost::thread *thr1 = nullptr, *thr2 = nullptr,
        *thr3 = nullptr, *thr4 = nullptr;

    MockEvent event(&env->service_, 2);

    // 2 Thread restarting and two thread cancel.
    // Thread 1 -> Restart 100 times waiting 2ms between each one.
    // Thread 2 -> Cancel 100 times waiting 2ms between each one.
    // Thread 3 -> Restart 80 times waiting 3ms between each one.
    // Thread 4 -> Cancel 80 times waiting 3ms between each one.

    thr1 = new boost::thread(restart, &event, 100, 2); 
    thr2 = new boost::thread(cancel, &event, 100, 2); 
    thr3 = new boost::thread(restart, &event, 80, 3); 
    thr4 = new boost::thread(cancel, &event, 80, 3); 

    thr1->join();
    thr2->join();
    thr3->join();
    thr4->join();

    delete thr1;
    delete thr2;
    delete thr3;
    delete thr4;

    // Wait all expected times
    for(unsigned int i = 0; i < 180; ++i)
        ASSERT_TRUE(event.wait(10));

    int successed = event.successed_.load(boost::memory_order_relaxed);
    int cancelled = event.cancelled_.load(boost::memory_order_relaxed);

    ASSERT_EQ(successed + cancelled, 180);
}

TEST(TimedEventMultithread, EventNonAutoDestruc_QuickestTwoStartTwoCancel)
{
    boost::thread *thr1 = nullptr, *thr2 = nullptr,
        *thr3 = nullptr, *thr4 = nullptr;

    MockEvent event(&env->service_, 2);

    // 2 Thread restarting and two thread cancel.
    // Thread 1 -> Restart 100 times waiting 0ms between each one.
    // Thread 2 -> Cancel 100 times waiting 0ms between each one.
    // Thread 3 -> Restart 80 times waiting 0ms between each one.
    // Thread 4 -> Cancel 80 times waiting 0ms between each one.

    thr1 = new boost::thread(restart, &event, 100, 0); 
    thr2 = new boost::thread(cancel, &event, 100, 0); 
    thr3 = new boost::thread(restart, &event, 80, 0); 
    thr4 = new boost::thread(cancel, &event, 80, 0); 

    thr1->join();
    thr2->join();
    thr3->join();
    thr4->join();

    delete thr1;
    delete thr2;
    delete thr3;
    delete thr4;

    // Wait all expected times
    for(unsigned int i = 0; i < 180; ++i)
        ASSERT_TRUE(event.wait(10));

    int successed = event.successed_.load(boost::memory_order_relaxed);
    int cancelled = event.cancelled_.load(boost::memory_order_relaxed);

    ASSERT_EQ(successed + cancelled, 180);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
#if defined(WIN32) && defined(_DEBUG)
    eprosima::Log::setVerbosity(eprosima::LOG_VERBOSITY_LVL::VERB_ERROR);
#endif
    return RUN_ALL_TESTS();
}
