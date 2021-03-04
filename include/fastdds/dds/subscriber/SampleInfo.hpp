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

#ifndef _FASTDDS_DDS_SUBSCRIBER_SAMPLEINFO_HPP_
#define _FASTDDS_DDS_SUBSCRIBER_SAMPLEINFO_HPP_

#include <fastdds/dds/subscriber/InstanceState.hpp>
#include <fastdds/dds/subscriber/SampleState.hpp>
#include <fastdds/dds/subscriber/ViewState.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/rtps/common/SampleIdentity.h>
#include <fastdds/rtps/common/Time_t.h>
#include <fastdds/rtps/common/Types.h>

namespace eprosima {
namespace fastdds {
namespace dds {

FASTDDS_DEPRECATED_UNTIL(3, "eprosima::fastdds::dds::READ",
        "Use eprosima::fastdds::dds::READ_SAMPLE_STATE instead.")
constexpr SampleStateKind READ = READ_SAMPLE_STATE;

FASTDDS_DEPRECATED_UNTIL(3, "eprosima::fastdds::dds::NOT_READ",
        "Use eprosima::fastdds::dds::NOT_READ_SAMPLE_STATE instead.")
constexpr SampleStateKind NOT_READ = NOT_READ_SAMPLE_STATE;

FASTDDS_DEPRECATED_UNTIL(3, "eprosima::fastdds::dds::NEW",
        "Use eprosima::fastdds::dds::NEW_VIEW_STATE instead.")
constexpr ViewStateKind NEW = NEW_VIEW_STATE;

FASTDDS_DEPRECATED_UNTIL(3, "eprosima::fastdds::dds::NOT_NEW",
        "Use eprosima::fastdds::dds::NOT_NEW_VIEW_STATE instead.")
constexpr ViewStateKind NOT_NEW = NOT_NEW_VIEW_STATE;

FASTDDS_DEPRECATED_UNTIL(3, "eprosima::fastdds::dds::ALIVE",
        "Use eprosima::fastdds::dds::ALIVE_INSTANCE_STATE instead.")
constexpr InstanceStateKind ALIVE = ALIVE_INSTANCE_STATE;

FASTDDS_DEPRECATED_UNTIL(3, "eprosima::fastdds::dds::NOT_ALIVE_DISPOSED",
        "Use eprosima::fastdds::dds::NOT_ALIVE_DISPOSED_INSTANCE_STATE instead.")
constexpr InstanceStateKind NOT_ALIVE_DISPOSED = NOT_ALIVE_DISPOSED_INSTANCE_STATE;

FASTDDS_DEPRECATED_UNTIL(3, "eprosima::fastdds::dds::NOT_ALIVE_NO_WRITERS",
        "Use eprosima::fastdds::dds::NOT_ALIVE_NO_WRITERS_INSTANCE_STATE instead.")
constexpr InstanceStateKind NOT_ALIVE_NO_WRITERS = NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;

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
    int32_t absoulte_generation_rank;

    //! time provided by the DataWriter when the sample was written
    fastrtps::rtps::Time_t source_timestamp;

    //! time provided by the DataReader when the sample was added to its history
    fastrtps::rtps::Time_t reception_timestamp;

    //! identifies locally the corresponding instance
    InstanceHandle_t instance_handle;

    //! identifies locally the DataWriter that modified the instance
    //!
    //! Is the same InstanceHandle_t that is returned by the operation get_matched_publications on the DataReader
    InstanceHandle_t publication_handle;

    //! whether the DataSample contains data or is only used to communicate of a change in the instance
    bool valid_data;

    //!Sample Identity (Extension for RPC)
    fastrtps::rtps::SampleIdentity sample_identity;

    //!Related Sample Identity (Extension for RPC)
    fastrtps::rtps::SampleIdentity related_sample_identity;

};

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif /* _FASTDDS_DDS_SUBSCRIBER_SAMPLEINFO_HPP_*/
