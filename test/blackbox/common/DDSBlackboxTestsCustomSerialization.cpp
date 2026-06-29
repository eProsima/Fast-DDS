// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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


#include <cstdint>
#include <limits>
#include <memory>

#include <gtest/gtest.h>

#include <fastcdr/Cdr.h>
#include <fastcdr/CdrSizeCalculator.hpp>

#include <fastdds/dds/topic/TopicDataType.hpp>

#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <cstring>
#include <fastdds/utils/md5.hpp>

//{ Type and context definitions

namespace eprosima {
namespace fastdds {
namespace dds {
namespace custom_serialization_test {

/**
 * @brief CustomData type that will be used in the custom serialization tests.
 */
struct CustomData
{
    uint8_t data;
};

/**
 * @brief CustomContext type that will be used in the custom serialization tests.
 */
struct CustomContext : public TopicDataType::Context
{
    ~CustomContext() override = default;

    /**
     * @brief Returns a shared pointer to the singleton instance of CustomContext.
     */
    static std::shared_ptr<CustomContext> instance()
    {
        static std::shared_ptr<CustomContext> instance = std::make_shared<CustomContext>();
        return instance;
    }

    /**
     * @brief Checks that the given context is the same as the singleton instance of CustomContext.
     */
    static void check_context(
            const std::shared_ptr<TopicDataType::Context>& context)
    {
        EXPECT_EQ(context, instance());
    }

};

}  // namespace custom_serialization_test
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

//}

