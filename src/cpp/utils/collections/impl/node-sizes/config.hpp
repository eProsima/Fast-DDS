// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file config.hpp
 *
 * Node size detection implementation is controlled by the following, mutually exclusive, #define'd symbols
 *
 * USE_FOONATHAN_NODE_SIZES
 *   Will use foonathan::memory::xxx_node_size functions to get the node sizes.
 *   This requires that foonathan_memory_node_size_debugger tool has been built and run when
 *   building foonathan::memory. On some cases, specially when cross-compiling, the tool would
 *   not have been run. This would be indicated by FOONATHAN_MEMORY_NO_NODE_SIZE.
 *
 * USE_STD_NODE_SIZES
 *   User provided STD_TREE_NODE_TYPE and STD_LIST_NODE_TYPE are used directly with sizeof() operator.
 *   This implies user knowledge on STD implementation, but the name of the types are not likely to change.
 *
 *   Known values table
 *
 *     +---------------+--------------------+--------------------+
 *     | Library       | STD_TREE_NODE_TYPE | STD_LIST_NODE_TYPE |
 *     +---------------+--------------------+--------------------+
 *     | MSVC          | std::_Tree_node    | std::_List_node    |
 *     | GNU libstdc++ | std::_Rb_tree_node | std::_List_node    |
 *     | LLVM libc++   | std::__tree_node   | std::__list_node   |
 *     +---------------+--------------------+--------------------+
 *
 * USE_CUSTOM_NODE_SIZES
 *   Will use the sizeof() operator over custom declared tree and list node types.
 *   These types are declared with details similar to the most common std implementations.
 */

#ifndef SRC_CPP_UTILS_COLLECTIONS_IMPL_NODE_SIZES_CONFIG_HPP_
#define SRC_CPP_UTILS_COLLECTIONS_IMPL_NODE_SIZES_CONFIG_HPP_

// Only set defaults when not forced by the user
#if !defined(USE_STD_NODE_SIZES) && !defined(USE_FOONATHAN_NODE_SIZES) && !defined(USE_CUSTOM_NODE_SIZES)

// User can also set node types directly (both should be defined)
#  if defined(STD_TREE_NODE_TYPE) || defined(STD_LIST_NODE_TYPE)
#    if !defined(STD_TREE_NODE_TYPE) || !defined(STD_LIST_NODE_TYPE)
#      error "Both STD_TREE_NODE_TYPE and STD_LIST_NODE_TYPE should be defined, but only one of them found"
#    else
#      define USE_STD_NODE_SIZES
#    endif  // STD types check
// Select foonathan node size calculation when available
#  elif !defined(FOONATHAN_MEMORY_NO_NODE_SIZE)
#    define USE_FOONATHAN_NODE_SIZES
// Use custom solution as last resource
#  else
#    define USE_CUSTOM_NODE_SIZES
#  endif  // Defaults selection

#else

// Check only one option has been selected
#  if (defined(USE_FOONATHAN_NODE_SIZES) && defined(USE_STD_NODE_SIZES)) || \
    (defined(USE_FOONATHAN_NODE_SIZES) && defined(USE_CUSTOM_NODE_SIZES)) || \
    (defined(USE_CUSTOM_NODE_SIZES) && defined(USE_STD_NODE_SIZES))
#    error "USE_STD_NODE_SIZES, USE_FOONATHAN_NODE_SIZES and USE_CUSTOM_NODE_SIZES are mutually exclusive"
// Check foonathan is available when selected
#  elif defined(USE_FOONATHAN_NODE_SIZES) && defined(FOONATHAN_MEMORY_NO_NODE_SIZE)
#    error "USE_FOONATHAN_NODE_SIZES selected but not available"
// Check std option is correctly configured
#  elif defined(USE_STD_NODE_SIZES) && (!defined(STD_TREE_NODE_TYPE) || !defined(STD_LIST_NODE_TYPE))
#    error "When USE_STD_NODE_SIZES is defined, STD_TREE_NODE_TYPE and STD_LIST_NODE_TYPE should be defined"
#  endif  // Misconfiguration checks

#endif  // Options check

#endif  /* SRC_CPP_UTILS_COLLECTIONS_IMPL_NODE_SIZES_CONFIG_HPP_ */