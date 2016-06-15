// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ResourceSend.cpp
 *
 */

#include <fastrtps/rtps/resources/ResourceSend.h>
#include "ResourceSendImpl.h"
#include "../participant/RTPSParticipantImpl.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

ResourceSend::ResourceSend()
{
	// TODO Auto-generated constructor stub
	mp_impl = new ResourceSendImpl();

}

ResourceSend::~ResourceSend() {
	// TODO Auto-generated destructor stub
	delete(mp_impl);
}

bool ResourceSend::initSend(RTPSParticipantImpl* pimpl, const Locator_t& loc,
		uint32_t sendsockBuffer,bool useIP4, bool useIP6)
{
	return mp_impl->initSend(pimpl,loc,sendsockBuffer,useIP4,useIP6);
}

void ResourceSend::sendSync(CDRMessage_t* msg, const Locator_t& loc)
{
	return mp_impl->sendSync(msg,loc);
}

boost::recursive_mutex* ResourceSend::getMutex()
{
	return mp_impl->getMutex();
}

void ResourceSend::loose_next_change()
{
	return mp_impl->loose_next();
}

}
} /* namespace rtps */
} /* namespace eprosima */
