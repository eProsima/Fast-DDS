// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TopicDescriptionImpl.hpp
 *
 */

#ifndef _FASTDDS_TOPICDESCRIPTIONIMPL_HPP_
#define _FASTDDS_TOPICDESCRIPTIONIMPL_HPP_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

namespace eprosima {
namespace fastdds {
namespace dds {

class Topic;

class TopicDescriptionImpl
{

public:

    TopicDescriptionImpl()
        : num_refs_(0u)
    {
    }

    virtual ~TopicDescriptionImpl()
    {
    }

    bool is_referenced() const
    {
        return num_refs_ != 0u;
    }

    void reference()
    {
        ++num_refs_;
    }

    void dereference()
    {
        --num_refs_;
    }

    virtual const std::string& get_rtps_topic_name() const = 0;

private:

    std::atomic_size_t num_refs_;

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* DOXYGEN_SHOULD_SKIP_THIS_PUBLIC */
#endif /* _FASTDDS_TOPICDESCRIPTIONIMPL_HPP_ */
