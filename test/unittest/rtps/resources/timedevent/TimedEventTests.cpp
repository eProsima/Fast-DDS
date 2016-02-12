#include "MockEvent.h"

#include <boost/random.hpp>
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
    MockEvent event(&env->service_, 100, false);

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
    MockEvent event(&env->service_, 100, false);

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
    MockEvent event(&env->service_, 100, false);

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
    MockEvent *event = new MockEvent(&env->service_, 100, false, eprosima::fastrtps::rtps::TimedEvent::ON_SUCCESS);

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
    MockEvent *event = new MockEvent(&env->service_, 100, false, eprosima::fastrtps::rtps::TimedEvent::ON_SUCCESS);

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
    MockEvent *event = new MockEvent(&env->service_, 0, false, eprosima::fastrtps::rtps::TimedEvent::ON_SUCCESS);

    // Cancel ten times.
    for(int i = 0; i < 10; ++i)
    {
        event->restart_timer();
        event->cancel_timer();

        ASSERT_TRUE(event->wait(100));

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
    MockEvent *event = new MockEvent(&env->service_, 100, false, eprosima::fastrtps::rtps::TimedEvent::ON_SUCCESS);

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
    MockEvent *event = new MockEvent(&env->service_, 0, false, eprosima::fastrtps::rtps::TimedEvent::ON_SUCCESS);

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
    MockEvent *event = new MockEvent(&env->service_, 100, false, eprosima::fastrtps::rtps::TimedEvent::ALLWAYS);

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
    MockEvent *event = new MockEvent(&env->service_, 100, false, eprosima::fastrtps::rtps::TimedEvent::ALLWAYS);

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
    MockEvent *event = new MockEvent(&env->service_, 0, false, eprosima::fastrtps::rtps::TimedEvent::ALLWAYS);

    event->restart_timer();
    event->cancel_timer();

    boost::unique_lock<boost::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
        MockEvent::destruction_cond_.wait_for(lock, boost::chrono::milliseconds(100));

    ASSERT_EQ(MockEvent::destructed_, 1);
}

TEST(TimedEvent, EventNonAutoDestruct_AutoRestart)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;
    MockEvent *event = new MockEvent(&env->service_, 10 , true);

    for(unsigned int i = 0; i < 100; ++i)
    {
        event->restart_timer();
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    }

    int successed = event->successed_.load(boost::memory_order_relaxed);

    ASSERT_GE(successed , 100);

    delete event;

    boost::unique_lock<boost::mutex> lock(MockEvent::destruction_mutex_);

    if(MockEvent::destructed_ != 1)
        MockEvent::destruction_cond_.wait_for(lock, boost::chrono::milliseconds(100));

    ASSERT_EQ(MockEvent::destructed_, 1);
}


TEST(TimedEvent, EventNonAutoDestruc_AutoRestartAndDeleteRandomly)
{
    // Restart destriction counter.
    MockEvent::destructed_ = 0;

    boost::mt19937 rng;
    boost::uniform_int<> range(10, 100);
    boost::variate_generator<boost::mt19937, boost::uniform_int<>> random(rng, range);

    MockEvent* event = new MockEvent(&env->service_, 2, true);

    event->restart_timer();
    boost::this_thread::sleep(boost::posix_time::milliseconds(random()));

    delete event;

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
            boost::this_thread::sleep(boost::posix_time::milliseconds(sleep_time));
    }
}

void cancel(MockEvent *event, unsigned int num_loop, unsigned int sleep_time)
{
    for(unsigned int i = 0; i < num_loop; ++i)
    {
        event->cancel_timer();

        if(sleep_time > 0)
            boost::this_thread::sleep(boost::posix_time::milliseconds(sleep_time));
    }
}

TEST(TimedEventMultithread, EventNonAutoDestruc_TwoStartTwoCancel)
{
    boost::thread *thr1 = nullptr, *thr2 = nullptr,
        *thr3 = nullptr, *thr4 = nullptr;

    MockEvent event(&env->service_, 2, false);

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
        ASSERT_TRUE(event.wait(200));

    int successed = event.successed_.load(boost::memory_order_relaxed);
    int cancelled = event.cancelled_.load(boost::memory_order_relaxed);

    ASSERT_EQ(successed + cancelled, 180);
}

TEST(TimedEventMultithread, EventNonAutoDestruc_QuickTwoStartTwoCancel)
{
    boost::thread *thr1 = nullptr, *thr2 = nullptr,
        *thr3 = nullptr, *thr4 = nullptr;

    MockEvent event(&env->service_, 2, false);

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
        ASSERT_TRUE(event.wait(100));

    int successed = event.successed_.load(boost::memory_order_relaxed);
    int cancelled = event.cancelled_.load(boost::memory_order_relaxed);

    ASSERT_EQ(successed + cancelled, 180);
}

TEST(TimedEventMultithread, EventNonAutoDestruc_QuickestTwoStartTwoCancel)
{
    boost::thread *thr1 = nullptr, *thr2 = nullptr,
        *thr3 = nullptr, *thr4 = nullptr;

    MockEvent event(&env->service_, 2, false);

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
        ASSERT_TRUE(event.wait(100));

    int successed = event.successed_.load(boost::memory_order_relaxed);
    int cancelled = event.cancelled_.load(boost::memory_order_relaxed);

    ASSERT_EQ(successed + cancelled, 180);
}

TEST(TimedEventMultithread, EventNonAutoDestruc_FourAutoRestart)
{
    boost::thread *thr1 = nullptr, *thr2 = nullptr,
        *thr3 = nullptr, *thr4 = nullptr;

    MockEvent event(&env->service_, 2, true);

    // 2 Thread restarting and two thread cancel.
    // Thread 1 -> AutoRestart 100 times waiting 2ms between each one.
    // Thread 2 -> AutoRestart 100 times waiting 3ms between each one.
    // Thread 3 -> AutoRestart 80 times waiting 4ms between each one.
    // Thread 4 -> AutoRestart 80 times waiting 5ms between each one.

    thr1 = new boost::thread(restart, &event, 100, 2); 
    thr2 = new boost::thread(restart, &event, 100, 3); 
    thr3 = new boost::thread(restart, &event, 80, 4); 
    thr4 = new boost::thread(restart, &event, 80, 5); 

    thr1->join();
    thr2->join();
    thr3->join();
    thr4->join();

    delete thr1;
    delete thr2;
    delete thr3;
    delete thr4;
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
