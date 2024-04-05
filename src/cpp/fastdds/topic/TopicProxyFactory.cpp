// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*
 * TopicProxyFactory.cpp
 */

#include <fastdds/topic/TopicProxyFactory.hpp>

#include <algorithm>

#include <fastdds/topic/TopicProxy.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

TopicProxy* TopicProxyFactory::create_topic()
{
    TopicProxy* ret_val = new TopicProxy(topic_name_, type_name_, status_mask_, &topic_impl_);
    proxies_.emplace_back(ret_val);
    return ret_val;
}

ReturnCode_t TopicProxyFactory::delete_topic(
        TopicProxy* proxy)
{
    auto it = std::find_if(proxies_.begin(), proxies_.end(), [proxy](const std::unique_ptr<TopicProxy>& item)
                    {
                        return item.get() == proxy;
                    });
    if (it != proxies_.end() && !proxy->is_referenced())
    {
        proxies_.erase(it);
        return RETCODE_OK;
    }

    return RETCODE_PRECONDITION_NOT_MET;
}

TopicProxy* TopicProxyFactory::get_topic()
{
    return proxies_.empty() ? nullptr : proxies_.front().get();
}

bool TopicProxyFactory::can_be_deleted()
{
    return proxies_.empty();
}

void TopicProxyFactory::enable_topic()
{
    for (auto& item : proxies_)
    {
        item->get_topic()->enable();
    }
}

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
