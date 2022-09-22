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
 * @file optionparser.hpp
 *
 */

#ifndef FASTDDS_OPTIONPARSER_HPP_
#define FASTDDS_OPTIONPARSER_HPP_

// specific optionparser.h includes must be moved out of the namespace to prevent macro issues
#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(_BitScanReverse)
#endif // ifdef _MSC_VER

namespace eprosima {

#ifdef OPTIONPARSER_H_
#define NESTED_OPTIONPARSER_H_INCLUDED_
// allow including again the header because is in another namespace
#undef OPTIONPARSER_H_
#endif // ifdef OPTIONPARSER_H_

#include "./optionparser/optionparser.h"

#ifndef NESTED_OPTIONPARSER_H_INCLUDED_
// restore original state
#undef OPTIONPARSER_H_
#else
#undef NESTED_OPTIONPARSER_H_INCLUDED_
#endif // ifndef NESTED_OPTIONPARSER_H_INCLUDED_

} // namespace eprosima

#endif // FASTDDS_OPTIONPARSER_HPP_
