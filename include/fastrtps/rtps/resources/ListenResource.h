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
 * @file ListenResource.h
 *
 */

#ifndef LISTENRESOURCE_H_
#define LISTENRESOURCE_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include <vector>
#include <cstdlib>

namespace boost
{
    class mutex;
}

namespace eprosima {
namespace fastrtps{
namespace rtps {

class ListenResourceImpl;
class RTPSWriter;
class RTPSReader;
class Endpoint;
class MessageReceiver;
class RTPSParticipantImpl;

/**
*@ingroup MANAGEMENT_MODULE
*/
class ListenResource {
public:
	ListenResource(RTPSParticipantImpl* partimpl,uint32_t ID,bool isDefault);
	virtual ~ListenResource();
	
	/**
	 * Add an associated enpoint to the list.
	 * @param end Pointer to the endpoint.
	 * @return True if correct.
	 */
	bool addAssociatedEndpoint(Endpoint* end);
	
	/**
	 * Remove an endpoint from the associated endpoint list.
	 * @param end Pointer to the endpoint.
	 * @return True if correct.
	 */
	bool removeAssociatedEndpoint(Endpoint* end);
	
	//!Actual implementation of the ListenResource
	ListenResourceImpl* mp_impl;
	//!Message receiver
	MessageReceiver* mp_receiver;
	//!List of associated writers
	std::vector<RTPSWriter*> m_assocWriters;
	//!List of associated readers
	std::vector<RTPSReader*> m_assocReaders;
	/**
	* Check if the instance has associated endpoints
	* @return true if the instance has associated endpoints
	*/
	inline bool hasAssociatedEndpoints()
	{
		return !(m_assocWriters.empty() && m_assocReaders.empty());
	};
	
	/**
	* Set the received message length
	* @param length received message length
	*/
	void setMsgRecMsgLength(uint32_t length);
	
	/**
	* Initialize the listening thread.
	*
	* @param pimpl Actual implementation of the listen resource
	* @param loc Locator to open the socket.
	* @param listenSockSize Maximum size of the socket
	* @param isMulti Boolean for when is multicast.
	* @param isFixed Boolean to indicate whether another locator can be use in case the default is already being used.
	* @return True if the socket was openned.
	*/
	bool init_thread(RTPSParticipantImpl* pimpl,Locator_t& loc,
			uint32_t listenSockSize,bool isMulti,bool isFixed);

	/**
	* Check if the instance is listening to a locator
	* @param loc Locator to check
	* @return true is the instance is listening to the given locator
	*/
	bool isListeningTo(Locator_t&loc);

	/**
	* Get the listen locator list
	* @return Listen locator list
	*/
	const LocatorList_t& getListenLocators();
	//!Get the pointer to the RTPSParticipantImpl object.
	inline RTPSParticipantImpl* getRTPSParticipantImpl() {return mp_RTPSParticipantImpl;}
	//!Pointer to the RTPSParticipantImpl.
	RTPSParticipantImpl* mp_RTPSParticipantImpl;

	//!Get the associated Mutex
	boost::mutex* getMutex();

	const uint32_t m_ID;

	const bool m_isDefaultListenResource;

private:

    ListenResource& operator=(const ListenResource&) NON_COPYABLE_CXX11;
};
}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* LISTENRESOURCE_H_ */
