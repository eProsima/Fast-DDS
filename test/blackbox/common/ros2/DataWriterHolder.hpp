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

#ifndef FASTDDS_TEST_BLACKBOX_COMMON_ROS2__DATAWRITERHOLDER_HPP
#define FASTDDS_TEST_BLACKBOX_COMMON_ROS2__DATAWRITERHOLDER_HPP

#include <gtest/gtest.h>

#include <fastdds/dds/core/detail/DDSReturnCode.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>

#include "./GenericHolder.hpp"

namespace eprosima {
namespace testing {
namespace ros2 {

using namespace eprosima::fastdds::dds;

GENERIC_HOLDER_CLASS(Publisher, DataWriter, delete_datawriter, writer)

}  // namespace ros2
}  // namespace testing
}  // namespace eprosima

#endif  // FASTDDS_TEST_BLACKBOX_COMMON_ROS2__DATAWRITERHOLDER_HPP
