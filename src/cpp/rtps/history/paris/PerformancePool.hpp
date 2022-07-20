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

/**
 * @file IPool.hpp
 */

#ifndef _DDSROUTERUTILS_POOL_PERFORMANCEPOOL_HPP_
#define _DDSROUTERUTILS_POOL_PERFORMANCEPOOL_HPP_

#include <functional>
#include <memory>
#include <stack>

#include <fastdds/rtps/common/CacheChange.h>
#include <rtps/history/PoolConfig.h>

namespace eprosima {
namespace ddsrouter {
namespace utils {

using namespace eprosima::fastrtps::rtps;

struct PoolConfiguration
{
    PoolConfiguration() = default;

    bool reusable = true;
    bool allow_reallocation = true;

    unsigned int initial_size = 0;
    unsigned int maximum_size = 0;

};


class PerformancePool
{
public:

    virtual bool loan(
            CacheChange_t*& element) = 0;

    virtual bool return_loan(
            CacheChange_t* element) = 0;

protected:

    void new_element_(CacheChange_t*& element)
    {
        element = new CacheChange_t();
    }

    void reset_element_(CacheChange_t*& )
    {
    }

    void delete_element_(CacheChange_t*& element)
    {
        delete element;
    }
};


class NonReusablePool : public PerformancePool
{
public:

    NonReusablePool(
            const PoolConfiguration& configuration)
        : maximum_size_(configuration.maximum_size)
        , reserved_(0)
    {
    }

    bool loan(
            CacheChange_t*& element) override
    {
        if (reserved_ < maximum_size_)
        {
            this->new_element_(element);
            reserved_++;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool return_loan(
            CacheChange_t* element) override
    {
        this->delete_element_(element);
        reserved_--;
        return true;
    }

protected:

    unsigned int maximum_size_;
    unsigned int reserved_;
};


class FixedPool : public PerformancePool
{
public:

    FixedPool(
            const PoolConfiguration& configuration)
        : elements_(configuration.maximum_size)
        , first_free_(configuration.maximum_size)
    {
        for (unsigned int i = 0; i < configuration.maximum_size; i++)
        {
            this->new_element_(elements_[i]);
        }
    }

    ~FixedPool()
    {
        for (unsigned int i = 0; i < elements_.size(); i++)
        {
            this->delete_element_(elements_[i]);
        }
    }

    bool loan(
            CacheChange_t*& element) override
    {
        if (first_free_ == 0)
        {
            return false;
        }
        else
        {
            first_free_--;
            element = elements_[first_free_];
            return true;
        }
    }

    bool return_loan(
            CacheChange_t* element) override
    {
        this->reset_element_(element);
        elements_[first_free_] = element;
        first_free_++;
        return true;
    }

protected:

    std::vector<CacheChange_t*> elements_;

    unsigned int first_free_;
};


class ReallocatingPool : public PerformancePool
{
public:

    ReallocatingPool(
            const PoolConfiguration& configuration)
        : maximum_size_(configuration.maximum_size)
        , reserved_(configuration.initial_size)
    {
        for (size_t i = 0; i < configuration.initial_size; ++i)
        {
            CacheChange_t* element;
            this->new_element_(element);
            elements_.push(element);
        }
    }

    ~ReallocatingPool()
    {
        while (!elements_.empty())
        {
            this->delete_element_(elements_.top());
            elements_.pop();
        }
    }

    bool loan(
            CacheChange_t*& element) override
    {
        if (elements_.empty())
        {
            if (reserved_ < maximum_size_)
            {
                this->new_element_(element);
                reserved_++;
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            element = elements_.top();
            elements_.pop();
            return true;
        }
    }

    bool return_loan(
            CacheChange_t* element) override
    {
        this->reset_element_(element);
        elements_.push(element);
        return true;
    }

protected:

    std::stack<CacheChange_t*> elements_;

    unsigned int maximum_size_;
    unsigned int reserved_;
};


std::unique_ptr<PerformancePool> create_cache_change_pool(
        PoolConfiguration configuration)
{
    if (!configuration.reusable)
    {
        return std::unique_ptr<PerformancePool>(
            new NonReusablePool(configuration));
    }
    else if(!configuration.allow_reallocation)
    {
        return std::unique_ptr<PerformancePool>(
            new FixedPool(configuration));
    }
    else
    {
        return std::unique_ptr<PerformancePool>(
            new ReallocatingPool(configuration));
    }
}

std::unique_ptr<PerformancePool> create_cache_change_pool(
        const eprosima::fastrtps::rtps::PoolConfig& config)
{
    PoolConfiguration configuration;
    configuration.initial_size = config.initial_size;
    configuration.maximum_size = config.maximum_size;

    if (config.maximum_size < config.initial_size)
    {
        // logWarning(POOL, "Max size configured to be less than initial size.");
        configuration.maximum_size = config.initial_size;
    }

    if (config.memory_policy == eprosima::fastrtps::rtps::DYNAMIC_RESERVE_MEMORY_MODE)
    {
        configuration.reusable = false;
    }
    else
    {
        configuration.reusable = true;
    }

    if (config.memory_policy == eprosima::fastrtps::rtps::PREALLOCATED_MEMORY_MODE)
    {
        configuration.allow_reallocation = false;
    }
    else
    {
        configuration.allow_reallocation = true;
    }

    return create_cache_change_pool(configuration);
}


} /* namespace utils */
} /* namespace ddsrouter */
} /* namespace eprosima */

#endif /* _DDSROUTERUTILS_POOL_PERFORMANCEPOOL_HPP_ */
