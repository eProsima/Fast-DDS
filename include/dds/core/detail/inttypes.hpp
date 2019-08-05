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
#ifndef OSPL_DDS_CORE_DETAIL_INTTYPES_HPP_
#define OSPL_DDS_CORE_DETAIL_INTTYPES_HPP_

/**
 * @file
 */

// Implementation

/* (from spec:) This implementation-defined header stands in for the C99 header files
 * inttypes.h. Under toolchains that support inttypes.h, this header can
 * simply include that one. Under toolchains that do not, this header must
 * provide equivalent definitions.
 */
#if defined _MSC_VER && (_MSC_VER <= 1500)
// VS 2005 & 2008 confirmed to have no stdint.h; predecessors presumed likewise
#include <dds/core/detail/old_win_stdint.h>
#else
#include <stdint.h>
#endif

// End of implementation

#endif /* OSPL_DDS_CORE_DETAIL_INTTYPES_HPP_ */
