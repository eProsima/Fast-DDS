
// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
//

#define FASTDDS_ENFORCE_LOG_INFO
#ifdef HAVE_LOG_NO_INFO
#undef HAVE_LOG_NO_INFO
#endif // HAVE_LOG_NO_INFO
#define HAVE_LOG_NO_INFO 0

#ifdef HAVE_LOG_NO_WARNING
#undef HAVE_LOG_NO_WARNING
#endif // HAVE_LOG_NO_WARNING
#define HAVE_LOG_NO_WARNING 1

#ifdef HAVE_LOG_NO_ERROR
#undef HAVE_LOG_NO_ERROR
#endif // HAVE_LOG_NO_ERROR
#define HAVE_LOG_NO_ERROR 0

#include <fastdds/dds/log/Log.hpp>
#include "LogMacros.hpp"
#include <gtest/gtest.h>

/* Check warning is not consumed */
TEST_F(LogMacrosTests, no_warning)
{
    EPROSIMA_LOG_ERROR(SampleCategory, "Sample error message");
    EPROSIMA_LOG_WARNING(SampleCategory, "Sample warning message");
    EPROSIMA_LOG_INFO(SampleCategory, "Sample info message");

    auto consumedEntries = HELPER_WaitForEntries(2);
    ASSERT_EQ(2u, consumedEntries.size());
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
