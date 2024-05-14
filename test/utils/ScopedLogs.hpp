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
//
#ifndef _TEST_UTILS_SCOPEDLOGS_HPP_
#define _TEST_UTILS_SCOPEDLOGS_HPP_

#include <regex>

#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace testing {

//! RAII to setup Logging
struct ScopeLogs
{
    //! Set a specific category filter
    ScopeLogs(
            std::string category_filter)
    {
        if (dds::Log::HasCategoryFilter())
        {
#ifdef __cpp_lib_make_unique
            filter_ = std::make_unique<std::regex>(dds::Log::GetCategoryFilter());
#else
            filter_ = std::unique_ptr<std::regex>(new std::regex(dds::Log::GetCategoryFilter()));
#endif // ifdef __cpp_lib_make_unique
        }
        old_ = dds::Log::GetVerbosity();
        dds::Log::SetCategoryFilter(std::regex{category_filter});
    }

    //! Set a specified level
    ScopeLogs(
            dds::Log::Kind new_verbosity = dds::Log::Error)
    {
        old_ = dds::Log::GetVerbosity();
        dds::Log::SetVerbosity(new_verbosity);
    }

    ~ScopeLogs()
    {
        dds::Log::Flush();

        if (filter_)
        {
            dds::Log::SetCategoryFilter(*filter_);
        }
        else
        {
            dds::Log::UnsetCategoryFilter();
        }

        dds::Log::SetVerbosity(old_);
    }

    dds::Log::Kind old_;
    std::unique_ptr<std::regex> filter_;
};

} // namespace testing
} // namespace fastdds
} // namespace eprosima

#endif // _TEST_UTILS_SCOPEDLOGS_HPP_
