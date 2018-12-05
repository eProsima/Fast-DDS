// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file AllocTestCommon.h
 *
 */

#ifndef ALLOCTESTCOMMON_H_
#define ALLOCTESTCOMMON_H_

/**
 * Used to run callgrind with --zero-before=callgrind_zero_count. 
 * See http://valgrind.org/docs/manual/cl-manual.html#cl-manual.options.activity
 */
void callgrind_zero_count();

/**
 * Used to run callgrind with --dump-before=callgrind_dump. 
 * See http://valgrind.org/docs/manual/cl-manual.html#cl-manual.options.activity
 */
void callgrind_dump();

#endif

