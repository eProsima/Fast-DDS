// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_TEST_BLACKBOX_COMMON_ROS2__NODE_HPP
#define FASTDDS_TEST_BLACKBOX_COMMON_ROS2__NODE_HPP

#include <memory>
#include <mutex>
#include <string>

#include "./Context.hpp"

namespace eprosima {
namespace testing {
namespace ros2 {

using namespace eprosima::fastdds::dds;

class Node
{
public:

    Node(
            const std::shared_ptr<Context>& context,
            const std::string& node_name)
        : context_(context)
        , node_name_(node_name)
    {
    }

    ~Node()
    {
    }

    void start()
    {
        std::lock_guard<std::mutex> _(mutex_);

        if (started_)
        {
            return;
        }

        started_ = true;
        create_builtin();
        do_start();
    }

    void stop()
    {
        std::lock_guard<std::mutex> _(mutex_);

        if (!started_)
        {
            return;
        }

        started_ = false;
        do_stop();
        destroy_builtin();
    }

protected:

    std::shared_ptr<Context> context_{};
    std::string node_name_{};

    mutable std::mutex mutex_{};
    bool started_ = false;

    virtual void do_start() = 0;

    virtual void do_stop() = 0;

private:

    void create_builtin()
    {
    }

    void destroy_builtin()
    {
    }

};

}  // namespace ros2
}  // namespace testing
}  // namespace eprosima

#endif  // FASTDDS_TEST_BLACKBOX_COMMON_ROS2__NODE_HPP
