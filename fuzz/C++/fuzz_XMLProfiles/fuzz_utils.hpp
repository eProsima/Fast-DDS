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

/**
 * @file fuzz_utils.hpp
 *
 */

// Helpers functions for fuzz targets.
#ifndef FASTDDS_FUZZ__FUZZ_UTILS_HPP
#define FASTDDS_FUZZ__FUZZ_UTILS_HPP

#include <stddef.h>
#include <stdint.h>

// Redirect stdout to /dev/null. Useful to ignore output from verbose fuzz
// target functions.
//
// Return 0 on success, -1 otherwise.
extern "C" int ignore_stdout(
        void);


#endif  // FASTDDS_FUZZ__FUZZ_UTILS_HPP
