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

#include <fastdds/topic/TopicProxy.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

TopicProxy* TopicProxyFactory::create_topic()
{
	return nullptr;
}

ReturnCode_t TopicProxyFactory::delete_topic(
		TopicProxy* proxy)
{
	static_cast<void>(proxy);

	return ReturnCode_t::RETCODE_UNSUPPORTED;
}

bool TopicProxyFactory::can_be_deleted()
{
	return proxies_.empty();
}

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
