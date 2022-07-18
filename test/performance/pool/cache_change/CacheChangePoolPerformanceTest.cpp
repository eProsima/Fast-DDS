// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest/gtest.h>

#include <iostream>
#include <fstream>
#include <vector>

#include <fastdds/rtps/history/IChangePool.h>
#include <fastdds/rtps/resources/ResourceManagement.h>

#include <rtps/history/CacheChangePool.h>
#include <rtps/history/PoolConfig.h>

constexpr const unsigned int TEST_ITERATIONS = 1000;
constexpr const unsigned int TEST_LOW_RESERVE = 1;
constexpr const unsigned int TEST_HIGH_RESERVE = 2048;

/**
 * Timer class to measure time easily
 */
class Timer
{
public:
    Timer()
        : start_time_(std::chrono::high_resolution_clock::now())
    {
    }

    void reset() noexcept
    {
        start_time_ = std::chrono::high_resolution_clock::now();
    }

    virtual double elapsed() const noexcept
    {
        return std::chrono::duration<double, std::milli>(
            std::chrono::high_resolution_clock::now() - start_time_).count();
    }

protected:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
};


//////////
// Data structure to hold the results of the test

using PerformanceTestData = std::vector<double>;

struct PerformanceDatum
{
    PerformanceDatum() = default;

    PerformanceDatum(PerformanceTestData data)
    {
        for (double& d : data)
        {
            avg_time += d;
        }
        avg_time /= data.size();

        for (double& d : data)
        {
            std_dev += std::pow(d - avg_time, 2);
        }
        std_dev /= data.size();

        std::sort(data.begin(), data.end());
        median_time = data[data.size() / 2];
    }

    double avg_time = 0;
    double std_dev = 0;
    double median_time = 0;

    void print(
        std::ostream &os,
        std::string separator = ",") const
    {
        os << separator << avg_time << separator << std_dev << separator << median_time;
    }
};

struct PerformanceData
{
    double creation;
    double low_reserve;
    double high_reserve;
    double high_release;
    double high_reserve_after_release;
    double all_release;
    double destruction;
    double total;
};

struct PerformanceResult
{
    PerformanceResult(std::vector<PerformanceData> data)
    {
        PerformanceTestData creation_data;
        PerformanceTestData low_reserve_data;
        PerformanceTestData high_reserve_data;
        PerformanceTestData high_release_data;
        PerformanceTestData high_reserve_after_release_data;
        PerformanceTestData all_release_data;
        PerformanceTestData destruction_data;
        PerformanceTestData total_data;

        for (PerformanceData& d : data)
        {
            creation_data.push_back(d.creation);
            low_reserve_data.push_back(d.low_reserve);
            high_reserve_data.push_back(d.high_reserve);
            high_release_data.push_back(d.high_release);
            high_reserve_after_release_data.push_back(d.high_reserve_after_release);
            all_release_data.push_back(d.all_release);
            destruction_data.push_back(d.destruction);
            total_data.push_back(d.total);
        }

        // Creation
        creation = PerformanceDatum(creation_data);
        low_reserve = PerformanceDatum(low_reserve_data);
        high_reserve = PerformanceDatum(high_reserve_data);
        high_release = PerformanceDatum(high_release_data);
        high_reserve_after_release = PerformanceDatum(high_reserve_after_release_data);
        all_release = PerformanceDatum(all_release_data);
        destruction = PerformanceDatum(destruction_data);
        total = PerformanceDatum(total_data);
    }

    PerformanceDatum creation;
    PerformanceDatum low_reserve;
    PerformanceDatum high_reserve;
    PerformanceDatum high_release;
    PerformanceDatum high_reserve_after_release;
    PerformanceDatum all_release;
    PerformanceDatum destruction;
    PerformanceDatum total;

    void print(
        std::ostream &os,
        std::string name,
        std::string separator = ",") const
    {
        os << name;
        creation.print(os, separator);
        low_reserve.print(os, separator);
        high_reserve.print(os, separator);
        high_release.print(os, separator);
        high_reserve_after_release.print(os, separator);
        all_release.print(os, separator);
        destruction.print(os, separator);
        total.print(os, separator);
        os << std::endl;
    }
};

using namespace eprosima::fastrtps::rtps;

//////////
// Performance test

PerformanceResult execute_performance_test(
    PoolConfig pool_configuration,
    unsigned int iterations,
    unsigned int low_reserve,
    unsigned int high_reserve)
{
    std::vector<PerformanceData> all_performance_data(iterations);

    for (unsigned int i = 0; i < TEST_ITERATIONS; ++i)
    {
        // Create new Performance data
        PerformanceData performance_data;
        Timer timer;
        // Vector to store every cache change created
        std::vector<CacheChange_t*> cache_changes(low_reserve + high_reserve);

        // POOL CREATION
        timer.reset();
        IChangePool* pool = new CacheChangePool(pool_configuration);
        performance_data.creation = timer.elapsed();

        // LOW RESERVE
        timer.reset();
        for (unsigned int i = 0; i < low_reserve; ++i)
        {
            pool->reserve_cache(cache_changes[i]);
        }
        performance_data.low_reserve = timer.elapsed();

        // HIGH RESERVE
        timer.reset();
        for (unsigned int i = low_reserve; i < high_reserve + low_reserve; ++i)
        {
            pool->reserve_cache(cache_changes[i]);
        }
        performance_data.high_reserve = timer.elapsed();

        // HIGH RELEASE
        timer.reset();
        for (unsigned int i = low_reserve; i < high_reserve + low_reserve; ++i)
        {
            pool->release_cache(cache_changes[i]);
        }
        performance_data.high_release = timer.elapsed();

        // HIGH RESERVE AFTER RELEASE
        timer.reset();
        for (unsigned int i = low_reserve; i < high_reserve + low_reserve; ++i)
        {
            pool->reserve_cache(cache_changes[i]);
        }
        performance_data.high_reserve_after_release = timer.elapsed();

        // ALL RELEASE
        timer.reset();
        for (unsigned int i = 0; i < high_reserve + low_reserve; ++i)
        {
            pool->release_cache(cache_changes[i]);
        }
        performance_data.all_release = timer.elapsed();

        // DESTRUCTION
        timer.reset();
        delete pool;
        performance_data.destruction = timer.elapsed();

        // Add this test execution to the result vector
        performance_data.total = performance_data.creation + performance_data.low_reserve + performance_data.high_reserve + performance_data.high_release + performance_data.high_reserve_after_release + performance_data.all_release + performance_data.destruction;
        all_performance_data[i] = performance_data;
    }

    return PerformanceResult(all_performance_data);
}

void print_performance_test_header(
    std::ostream &os,
    std::string separator = ",")
{
    std::vector<std::string> names = {
        "creation",
        "low_reserve",
        "high_reserve",
        "high_release",
        "high_reserve_after_release",
        "all_release",
        "destruction",
        "total"
    };
    std::vector<std::string> statistics = {
        "avg",
        "std_dev",
        "median"
    };

    os << "Configuration"; // first cell empty
    for (std::string name : names)
    {
        for (std::string statistic : statistics)
        {
            os << separator << name << "[" << statistic << "]";
        }
    }
    os << std::endl;
}

std::vector<std::pair<std::string, PoolConfig>> generate_pool_configurations()
{
    return {

        std::make_pair<std::string, PoolConfig>(
            "preallocated initialized",
            PoolConfig(
                PREALLOCATED_MEMORY_MODE,
                0,
                TEST_LOW_RESERVE + TEST_HIGH_RESERVE,
                TEST_LOW_RESERVE + TEST_HIGH_RESERVE)),

        std::make_pair<std::string, PoolConfig>(
            "preallocated not initialized",
            PoolConfig(
                PREALLOCATED_MEMORY_MODE,
                0,
                0,
                TEST_LOW_RESERVE + TEST_HIGH_RESERVE)),

        std::make_pair<std::string, PoolConfig>(
            "dynamic reserve",
            PoolConfig(
                DYNAMIC_RESERVE_MEMORY_MODE,
                0,
                0,
                TEST_LOW_RESERVE + TEST_HIGH_RESERVE)),

        std::make_pair<std::string, PoolConfig>(
            "dynamic reuse",
            PoolConfig(
                DYNAMIC_REUSABLE_MEMORY_MODE,
                0,
                0,
                TEST_LOW_RESERVE + TEST_HIGH_RESERVE)),
    };
}

TEST(ChangePoolPerformanceTest, fastdds_pool)
{
    /////
    // Start file for result
    // Create file
    std::ofstream file;
    file.open("cache_change_pool_performance_results.csv");
    // Write header
    std::string separator = ",";
    print_performance_test_header(file, separator);

    /////
    // Create Pool configurations
    // Create Pool
    auto pool_configurations = generate_pool_configurations();

    /////
    // Execute tests
    for (auto& pool_configuration : pool_configurations)
    {
        // Create result
        PerformanceResult result = execute_performance_test(
            pool_configuration.second,
            TEST_ITERATIONS,
            TEST_LOW_RESERVE,
            TEST_HIGH_RESERVE);
        // Print result
        result.print(file, pool_configuration.first, separator);
    }
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
