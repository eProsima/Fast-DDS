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
 * @file constructor_macros.hpp
 */

#ifndef UTILS_CONSTRUCTOR_MACROS_HPP_
#define UTILS_CONSTRUCTOR_MACROS_HPP_

#ifndef FASTDDS_COPY_OPERATIONS
#define FASTDDS_COPY_OPERATIONS(ClassName, access) \
    ClassName(const ClassName&) = access; \
    ClassName& operator = (const ClassName&) = access
#endif // !FASTDDS_COPY_OPERATIONS

#ifndef FASTDDS_MOVE_OPERATIONS
#define FASTDDS_MOVE_OPERATIONS(ClassName, access) \
    ClassName(ClassName &&) = access; \
    ClassName& operator = (ClassName &&) = access
#endif // !FASTDDS_COPY_OPERATIONS

#ifndef FASTDDS_DEFAULT_COPY
#define FASTDDS_DEFAULT_COPY(ClassName) FASTDDS_COPY_OPERATIONS(ClassName, default)
#endif // !FASTDDS_DEFAULT_COPY

#ifndef FASTDDS_DELETED_COPY
#define FASTDDS_DELETED_COPY(ClassName) FASTDDS_COPY_OPERATIONS(ClassName, delete)
#endif // !FASTDDS_DELETED_COPY

#ifndef FASTDDS_DEFAULT_MOVE
#define FASTDDS_DEFAULT_MOVE(ClassName) FASTDDS_MOVE_OPERATIONS(ClassName, default)
#endif // !FASTDDS_DEFAULT_COPY

#ifndef FASTDDS_DELETED_MOVE
#define FASTDDS_DELETED_MOVE(ClassName) FASTDDS_MOVE_OPERATIONS(ClassName, delete)
#endif // !FASTDDS_DELETED_COPY


#endif  // UTILS_CONSTRUCTOR_MACROS_HPP
