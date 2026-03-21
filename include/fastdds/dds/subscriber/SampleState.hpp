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
 * @file SampleState.hpp
 */

#ifndef FASTDDS_DDS_SUBSCRIBER__SAMPLESTATE_HPP
#define FASTDDS_DDS_SUBSCRIBER__SAMPLESTATE_HPP

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Indicates whether or not a sample has ever been read.
 *
 * For each sample received, the middleware internally maintains a sample state relative to each @ref DataReader. This
 * sample state can have the following values:
 *
 * @li @ref READ_SAMPLE_STATE indicates that the \ref DataReader has already accessed that sample by means of a read
 *          or take operation
 *
 * @li @ref NOT_READ_SAMPLE_STATE indicates that the \ref DataReader has not accessed that sample before.
 *
 * The sample state will, in general, be different for each sample in the collection returned by read or take.
 */
enum SampleStateKind : uint16_t
{
    /// Sample has been read.
    READ_SAMPLE_STATE = 0x0001 << 0,

    /// Sample has not been read.
    NOT_READ_SAMPLE_STATE = 0x0001 << 1,
};

/// A bit-mask (list) of sample states, i.e. @ref SampleStateKind
using SampleStateMask = uint16_t;

/// Any sample state
constexpr SampleStateMask ANY_SAMPLE_STATE = READ_SAMPLE_STATE | NOT_READ_SAMPLE_STATE;

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_DDS_SUBSCRIBER__SAMPLESTATE_HPP
