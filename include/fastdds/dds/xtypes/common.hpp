// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*!
 * @file
 * This file contains common definitions for the different XTypes modules.
 */

#ifndef _FASTDDS_DDS_XTYPES_COMMON_HPP_
#define _FASTDDS_DDS_XTYPES_COMMON_HPP_

/**
 * @brief Extensibility kinds
 */
enum ExtensibilityKind
{
    FINAL,
    APPENDABLE,
    MUTABLE
};

/**
 * @brief Try construct kinds
 */
enum TryConstructKind
{
    USE_DEFAULT,
    DISCARD,
    TRIM
};

/**
 * @brief EquivalenceKind values
 */
enum EquivalenceKindValue
{
    EK_MINIMAL,
    EK_COMPLETE,
    EK_BOTH
};

#endif // _FASTDDS_DDS_XTYPES_COMMON_HPP_
