/*
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/sub/SampleInfo.hpp>
#include <dds/core/InstanceHandle.hpp>
#include <dds/core/Time.hpp>
#include <dds/core/detail/Value.hpp>

namespace dds {
namespace sub {

SampleInfo::SampleInfo()
    : ::dds::core::Value<detail::SampleInfo>()
{
}

SampleInfo::SampleInfo(
        detail::SampleInfo sample_info)
    : ::dds::core::Value<detail::SampleInfo>(sample_info)
{
}

SampleInfo::~SampleInfo()
{
}

const dds::core::Time SampleInfo::timestamp() const
{
    return dds::core::Time(delegate().source_timestamp.seconds(), delegate().source_timestamp.nanosec());
}

const dds::sub::status::DataState SampleInfo::state() const
{
    status::DataState state;
    state.view_state(delegate().view_state);
    state.sample_state(delegate().sample_state);
    state.instance_state(delegate().instance_state);
    return state;
}

dds::sub::GenerationCount SampleInfo::generation_count() const
{
    GenerationCount count;
    count.disposed_generation_count = delegate().disposed_generation_count;
    count.no_writers_generation_count = delegate().no_writers_generation_count;
    return count;
}

dds::sub::Rank SampleInfo::rank() const
{
    Rank rank;
    rank.sample_rank = delegate().sample_rank;
    rank.generation_rank = delegate().generation_rank;
    rank.absolute_generation_rank = delegate().absolute_generation_rank;
    return rank;
}

bool SampleInfo::valid() const
{
    return delegate().valid_data;
}

dds::core::InstanceHandle SampleInfo::instance_handle() const
{
    return delegate().instance_handle;
}

dds::core::InstanceHandle SampleInfo::publication_handle() const
{
    return delegate().publication_handle;
}

} //namespace sub
} //namespace dds
