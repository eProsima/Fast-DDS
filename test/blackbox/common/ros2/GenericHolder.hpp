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

#ifndef FASTDDS_TEST_BLACKBOX_COMMON_ROS2__GENERICHOLDER_HPP
#define FASTDDS_TEST_BLACKBOX_COMMON_ROS2__GENERICHOLDER_HPP

#include <fastdds/dds/core/detail/DDSReturnCode.hpp>

#include <gtest/gtest.h>

#define GENERIC_HOLDER_CLASS(_Factory, _Entity, _Release, _Getter) \
class _Entity##Holder                                              \
{                                                                  \
public:                                                            \
    _Entity##Holder(                                               \
            _Factory* factory,                                     \
            _Entity* entity)                                       \
        : factory_(factory)                                        \
        , entity_(entity)                                          \
    {                                                              \
    }                                                              \
                                                                   \
    ~_Entity##Holder()                                             \
    {                                                              \
        if (nullptr != factory_ && nullptr != entity_)             \
        {                                                          \
            EXPECT_EQ(RETCODE_OK, factory_->_Release(entity_));    \
        }                                                          \
    }                                                              \
                                                                   \
    _Entity* _Getter()                                             \
    {                                                              \
        return entity_;                                            \
    }                                                              \
                                                                   \
private:                                                           \
                                                                   \
    _Factory* factory_ = nullptr;                                  \
    _Entity* entity_ = nullptr;                                    \
                                                                   \
};

#endif  // FASTDDS_TEST_BLACKBOX_COMMON_ROS2__GENERICHOLDER_HPP
