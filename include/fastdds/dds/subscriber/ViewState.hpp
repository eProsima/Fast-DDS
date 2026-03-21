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
 * @file ViewState.hpp
 */

#ifndef FASTDDS_DDS_SUBSCRIBER__VIEWSTATE_HPP
#define FASTDDS_DDS_SUBSCRIBER__VIEWSTATE_HPP

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Indicates whether or not an instance is new.
 *
 * For each instance (identified by the key), the middleware internally maintains a view state
 * relative to each @ref DataReader. This view state can have the following values:
 *
 * @li @ref NEW_VIEW_STATE indicates that either this is the first time that the @ref DataReader has
 *     ever accessed samples of that instance, or else that the @ref DataReader has accessed previous
 *     samples of the instance, but the instance has since been reborn (i.e. become not-alive and
 *     then alive again). These two cases are distinguished by examining the
 *     @ref SampleInfo::disposed_generation_count and the @ref SampleInfo::no_writers_generation_count.
 *
 * @li @ref NOT_NEW_VIEW_STATE indicates that the @ref DataReader has already accessed samples of the
 *     same instance and that the instance has not been reborn since.
 *
 * The view_state available in the @ref SampleInfo is a snapshot of the view state of the instance
 * relative to the @ref DataReader used to access the samples at the time the collection was obtained
 * (i.e. at the time read or take was called). The view_state is therefore the same for all samples in
 * the returned collection that refer to the same instance.
 *
 * Once an instance has been detected as not having any "live" writers and all the samples associated
 * with the instance are "taken" from the DDSDataReader, the middleware can reclaim all local resources
 * regarding the instance. Future samples will be treated as "never seen."
 */
enum ViewStateKind : uint16_t
{
    /// New instance.This latest generation of the instance has not previously been accessed.
    NEW_VIEW_STATE = 0x0001 << 0,

    /// Not a new instance. This latest generation of the instance has previously been accessed.
    NOT_NEW_VIEW_STATE = 0x0001 << 1,
};

/// A bit-mask (list) of view states, i.e. @ref ViewStateKind
using ViewStateMask = uint16_t;

/// Any view state
constexpr ViewStateMask ANY_VIEW_STATE = NEW_VIEW_STATE | NOT_NEW_VIEW_STATE;

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_DDS_SUBSCRIBER__VIEWSTATE_HPP
