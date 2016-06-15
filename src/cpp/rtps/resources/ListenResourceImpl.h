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
 * @file ListenResourceImpl.h
 *
 */

#ifndef LISTENRESOURCEIMPL_H_
#define LISTENRESOURCEIMPL _H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC


#include <fastrtps/rtps/common/Locator.h>


#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4005)
#endif  // _MSC_VER
#include <boost/asio.hpp>
#ifdef _MSC_VER
# pragma warning(pop)
#endif  // _MSC_VER
#include <boost/asio/ip/udp.hpp>

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4100)
#endif  // _MSC_VER
#include <boost/thread.hpp>
#ifdef _MSC_VER
# pragma warning(pop)
#endif  // _MSC_VER

namespace eprosima {
namespace fastrtps{
namespace rtps {

class RTPSParticipantImpl;
class ListenResource;

/**
 * Class ListenResourceImpl, used to control the listen sockets and the received messages.
 *@ingroup MANAGEMENT_MODULE
 */
class ListenResourceImpl
{
public:
	/**
	* @param LR Listen resource
	*/
	ListenResourceImpl(ListenResource* LR);
	
	virtual ~ListenResourceImpl();
	/**
	 * Initialize the listening thread.
	 * @param loc Locator to open the socket.
	 * @param listenSockSize Maximum size of the socket
	 * @param isMulti Boolean for when is multicast.
	 * @param isFixed Boolean to indicate whether another locator can be use in case the default is already being used.
	 * @return If the openning was succesful
	 */
	bool init_thread(RTPSParticipantImpl* pimpl,Locator_t& loc,uint32_t listenSocketSize,bool isMulti,bool isFixed);

	/**
	* Check if the instance is listening to a locator
	* @param loc Locator to check
	* @return true is the instance is listening to the given locator
	*/
	bool isListeningTo(const Locator_t& loc);

	//!
	void putToListen();

	/**
	* Get the mutex
	* @return Associated mutex
	*/
	inline boost::mutex* getMutex() {return &mutex_;};

	/**
	* Get the listen locators
	* @return Listen locators
	*/
	inline const LocatorList_t& getListenLocators() const {return mv_listenLoc;}

	private:
	RTPSParticipantImpl* mp_RTPSParticipantImpl;
	ListenResource* mp_listenResource;
	boost::thread* mp_thread;
	boost::asio::io_service m_io_service;
	boost::asio::ip::udp::socket m_listen_socket;
	boost::asio::ip::udp::endpoint m_sender_endpoint;
	boost::asio::ip::udp::endpoint m_listen_endpoint;

	void getLocatorAddresses(Locator_t& loc, bool isMulti);
	void joinMulticastGroup(boost::asio::ip::address& addr);
	LocatorList_t mv_listenLoc;
	Locator_t m_senderLocator;


//
//	MessageReceiver m_MessageReceiver;

	/**
	 * Callback to be executed when a new Message is received in the socket.
	 * @param error Error code associated with the operation.
	 * @param size NUmber of bytes received
	 */
	void newCDRMessage(const boost::system::error_code& error, std::size_t size);

	//! Method to run the io_service.
	void run_io_service();

	boost::mutex mutex_;
	boost::condition_variable cond_;
    bool runningAsync_;
    bool stopped_;

};
}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* LISTENRESOURCE_H_ */
