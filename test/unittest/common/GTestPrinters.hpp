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
 * @file GTestPrinters.hpp
 *
 */

#ifndef GTESTPRINTERS_HPP
#define GTESTPRINTERS_HPP

#include <dds/core/types.hpp>

/**
 * Workaround for GCC to behave properly and let Google Test know what to do with null_types
 * when printing an ASSERT/EXPECT result. Documentation can be found here:
 * https://github.com/google/googletest/blob/main/googletest/include/gtest/gtest-printers.h
 *
 */
namespace dds {
namespace core {
void PrintTo(
        const null_type,
        std::ostream* os)
{
    *os << "::dds::core::null_type";
}

} // namespace core
} // namespace dds

#endif // GTESTPRINTERS_HPP
