#include <fastdds/utils/TimedConditionVariable.hpp>

#include <chrono>

#include <gtest/gtest.h>

TEST(TimedConditionVariable, wait_for)
{
    eprosima::fastdds::TimedConditionVariable cv;
    std::timed_mutex mutex;
    std::unique_lock<std::timed_mutex> lock(mutex);

    std::chrono::steady_clock::time_point initial = std::chrono::steady_clock::now();

    ASSERT_FALSE(cv.wait_for(lock, std::chrono::nanoseconds(2000000000), []() -> bool
            {
                return false;
            }));
    std::chrono::steady_clock::time_point final = std::chrono::steady_clock::now();
    ASSERT_LE(std::chrono::seconds(1), final - initial);
}

TEST(TimedConditionVariable, wait_until)
{
    eprosima::fastdds::TimedConditionVariable cv;
    std::timed_mutex mutex;
    std::unique_lock<std::timed_mutex> lock(mutex);

    std::chrono::steady_clock::time_point initial = std::chrono::steady_clock::now();

    ASSERT_FALSE(cv.wait_until(lock, initial + std::chrono::seconds(2), []() -> bool
            {
                return false;
            }));
    std::chrono::steady_clock::time_point final = std::chrono::steady_clock::now();
    ASSERT_LE(std::chrono::seconds(1), final - initial);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
