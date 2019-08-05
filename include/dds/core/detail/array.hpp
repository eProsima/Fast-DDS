/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef OSPL_DDS_CORE_DETAIL_ARRAY_HPP_
#define OSPL_DDS_CORE_DETAIL_ARRAY_HPP_

#include <dds/core/detail/macros.hpp>

/* Compiling explicitly w/ C++ 11 support */
#if defined(OSPL_USE_CXX11)
#  include <array>
#  define OSPL_CXX11_STD_MODULE ::std
/* Compiling to use Tech Report 1 headers */
#elif defined(OSPL_USE_TR1)
#  ifdef _MSC_VER
#    include <array>
#  else
#    include <tr1/array>
#  endif
#  define OSPL_CXX11_STD_MODULE ::std::tr1
/* Compiling with boost */
#elif defined(OSPL_USE_BOOST)
#  include <boost/array.hpp>
#  define OSPL_CXX11_STD_MODULE ::boost
#endif


namespace dds
{
namespace core
{
namespace detail
{
using OSPL_CXX11_STD_MODULE::array;
}
}
}

#endif /* OSPL_DDS_CORE_DETAIL_ARRAY_HPP_ */
