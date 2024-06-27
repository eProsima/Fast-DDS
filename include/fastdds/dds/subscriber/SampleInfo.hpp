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
 * @file SampleInfo.hpp
 *
 */

#ifndef FASTDDS_DDS_SUBSCRIBER__SAMPLEINFO_HPP
#define FASTDDS_DDS_SUBSCRIBER__SAMPLEINFO_HPP

#include <fastdds/dds/subscriber/InstanceState.hpp>
#include <fastdds/dds/subscriber/SampleState.hpp>
#include <fastdds/dds/subscriber/ViewState.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/rtps/common/SampleIdentity.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/dds/common/InstanceHandle.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/*!
 * @brief SampleInfo is the information that accompanies each sample that is ‘read’ or ‘taken.’
 */
struct SampleInfo
{
    //! indicates whether or not the corresponding data sample has already been read
    SampleStateKind sample_state;

    //! indicates whether the DataReader has already seen samples for the most-current generation of the related instance.
    ViewStateKind view_state;

    //! indicates whether the instance is currently in existence or, if it has been disposed, the reason why it was disposed.
    InstanceStateKind instance_state;

    //! number of times the instance had become alive after it was disposed
    int32_t disposed_generation_count;

    //! number of times the instance had become alive after it was disposed because no writers
    int32_t no_writers_generation_count;

    //! number of samples related to the same instance that follow in the collection
    int32_t sample_rank;

    //! the generation difference between the time the sample was received, and the time the most recent sample in the collection was received.
    int32_t generation_rank;

    //! the generation difference between the time the sample was received, and the time the most recent sample was received.
    //! The most recent sample used for the calculation may or may not be in the returned collection
    int32_t absolute_generation_rank;

    //! time provided by the DataWriter when the sample was written
    rtps::Time_t source_timestamp;

    //! time provided by the DataReader when the sample was added to its history
    rtps::Time_t reception_timestamp;

    //! identifies locally the corresponding instance
    InstanceHandle_t instance_handle;

    //! identifies locally the DataWriter that modified the instance
    //!
    //! Is the same InstanceHandle_t that is returned by the operation get_matched_publications on the DataReader
    InstanceHandle_t publication_handle;

    //! whether the DataSample contains data or is only used to communicate of a change in the instance
    bool valid_data;

    //!Sample Identity (Extension for RPC)
    rtps::SampleIdentity sample_identity;

    //!Related Sample Identity (Extension for RPC)
    rtps::SampleIdentity related_sample_identity;

};

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif // FASTDDS_DDS_SUBSCRIBER__SAMPLEINFO_HPP
