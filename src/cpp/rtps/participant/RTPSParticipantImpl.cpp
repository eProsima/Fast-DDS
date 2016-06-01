/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSParticipant.cpp
 *
 */

#include "RTPSParticipantImpl.h"

#include <fastrtps/rtps/resources/ResourceSend.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>
#include <fastrtps/rtps/resources/AsyncWriterThread.h>
#include <fastrtps/rtps/resources/ListenResource.h>

#include <fastrtps/rtps/messages/MessageReceiver.h>

#include <fastrtps/rtps/writer/StatelessWriter.h>
#include <fastrtps/rtps/writer/StatefulWriter.h>

#include <fastrtps/rtps/reader/StatelessReader.h>
#include <fastrtps/rtps/reader/StatefulReader.h>

#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/transport/UDPv4Transport.h>

#include <fastrtps/rtps/RTPSDomain.h>

#include <fastrtps/rtps/builtin/BuiltinProtocols.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>

#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/utils/eClock.h>

#include <boost/thread.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

//#include <fastrtps/utils/RTPSLog.h>



namespace eprosima {
namespace fastrtps{
namespace rtps {


static const char* const CLASS_NAME = "RTPSParticipantImpl";

static EntityId_t TrustedWriter(const EntityId_t& reader)
{
	if(reader == c_EntityId_SPDPReader) return c_EntityId_SPDPWriter;
	if(reader == c_EntityId_SEDPPubReader) return c_EntityId_SEDPPubWriter;
	if(reader == c_EntityId_SEDPSubReader) return c_EntityId_SEDPSubWriter;
	if(reader == c_EntityId_ReaderLiveliness) return c_EntityId_WriterLiveliness;

	return c_EntityId_Unknown;
}

Locator_t RTPSParticipantImpl::applyLocatorAdaptRule(Locator_t loc){
	switch (loc.kind){
	case LOCATOR_KIND_UDPv4:
		//This is a completely made up rule
		loc.port += 10;
		break;
	case LOCATOR_KIND_UDPv6:
		//TODO - Define the rest of rules
		break;
	}
	return loc;
}

RTPSParticipantImpl::RTPSParticipantImpl(const RTPSParticipantAttributes& PParam,
		const GuidPrefix_t& guidP,
		RTPSParticipant* par,
		RTPSParticipantListener* plisten):	m_guid(guidP,c_EntityId_RTPSParticipant),
				mp_event_thr(nullptr),
				mp_builtinProtocols(nullptr),
				mp_ResourceSemaphore(new boost::interprocess::interprocess_semaphore(0)),
				IdCounter(0),
				mp_participantListener(plisten),
				mp_userParticipant(par),
				mp_mutex(new boost::recursive_mutex()),
				m_threadID(0)

{
   // Builtin transport by default
   if (PParam.useBuiltinTransports)
   {
      UDPv4Transport::TransportDescriptor descriptor; 
      descriptor.sendBufferSize = m_att.listenSocketBufferSize;
      descriptor.receiveBufferSize = m_att.listenSocketBufferSize;
      descriptor.granularMode = true;
      m_network_Factory.RegisterTransport<UDPv4Transport>(descriptor);
   }

   // User defined transports
   for (const auto& transportDescriptor : PParam.userTransports)
      m_network_Factory.RegisterTransport(transportDescriptor.get());
   
	//const char* const METHOD_NAME = "RTPSParticipantImpl";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	mp_userParticipant->mp_impl = this;
	m_att = PParam;
	Locator_t loc;
	loc.port = PParam.defaultSendPort;
	mp_event_thr = new ResourceEvent();
	mp_event_thr->init_thread(this);

   for (const auto& descriptor : PParam.sizeFilters)
   {
      std::unique_ptr<FlowFilter> filter(new SizeFilter(descriptor, this));
      m_filters.push_back(std::move(filter));
   }

	bool hasLocatorsDefined = true;
	//If no default locators are defined we define some.
	/* The reasoning here is the following.
		If the parameters of the RTPS Participant don't hold default listening locators for the creation
		of Endpoints, we make some for Unicast only.
		If there is at least one listen locator of any kind, we do not create any default ones.
		If there are no sending locators defined, we create default ones for the transports we implement.
	*/
	if(m_att.defaultUnicastLocatorList.empty() && m_att.defaultMulticastLocatorList.empty())
	{
		//Default Unicast Locators in case they have not been provided
		/* INSERT DEFAULT UNICAST LOCATORS FOR THE PARTICIPANT */
		hasLocatorsDefined = false;
		Locator_t loc2;

		LocatorList_t loclist;
		IPFinder::getIP4Address(&loclist);
		for(auto it=loclist.begin();it!=loclist.end();++it){
			(*it).port=m_att.port.portBase+
				m_att.port.domainIDGain*PParam.builtin.domainId+
				m_att.port.offsetd3+
				m_att.port.participantIDGain*m_att.participantID;
			(*it).kind = LOCATOR_KIND_UDPv4;

			m_att.defaultUnicastLocatorList.push_back((*it));
		}
		// FIXME -- We have to  discuss the rules for deafult locator assignment for each transport
		loc2.port= m_att.port.portBase+
				m_att.port.domainIDGain*PParam.builtin.domainId+
				m_att.port.offsetd3+
				m_att.port.participantIDGain*m_att.participantID;
		loc2.set_IP4_address(239,255,1,4);
      m_att.defaultMulticastLocatorList.push_back(loc2);
		/* INSERT DEFAULT MULTICAST LOCATORS FOR THE PARTICIPANT */
	}

	/*	
		Since nothing guarantees the correct creation of the Resources on the Locators we have specified, and 
		in order to maintain synchrony between the defaultLocator list and the actuar ReceiveResources,
		We create the resources for these Locators now. Furthermore, in case these resources are taken, 
		we create them on another Locator and then update de defaultList.
	*/
	createReceiverResources(m_att.defaultUnicastLocatorList, true);
	
	if(!hasLocatorsDefined)
		//logInfo(RTPS_PARTICIPANT,m_att.getName()<<" Created with NO default Unicast Locator List, adding Locators: "<<m_att.defaultUnicastLocatorList);

	//Multicast
	createReceiverResources(m_att.defaultMulticastLocatorList, true);
	
	//Check if defaultOutLocatorsExist, create some if they don't
	hasLocatorsDefined = true;
	if (m_att.defaultOutLocatorList.empty()){
		hasLocatorsDefined = false;
		Locator_t SendLocator;
		/*TODO - Fill with desired default Send Locators for our transports*/
		//Warning - Mock rule being used (and only for IPv4)!
		SendLocator.port = m_att.port.portBase +
			m_att.port.domainIDGain*PParam.builtin.domainId +
			m_att.port.offsetd3 +
			m_att.port.participantIDGain*m_att.participantID + 50;
		SendLocator.kind = LOCATOR_KIND_UDPv4;
		m_att.defaultOutLocatorList.push_back(SendLocator);
	}
	//Create the default sendResources - For the same reason as in the ReceiverResources
	std::vector<SenderResource > newSenders;
	std::vector<SenderResource > newSendersBuffer;
	LocatorList_t defcopy = m_att.defaultOutLocatorList;
	for (auto it = defcopy.begin(); it != defcopy.end(); ++it){
		/* Try to build resources with that specific Locator*/
		newSendersBuffer = m_network_Factory.BuildSenderResources((*it));
		while (newSendersBuffer.empty()){
			//No ReceiverResources have been added, therefore we have to change the Locator 
			(*it) = applyLocatorAdaptRule(*it);			//Mutate the Locator to find a suitable rule. Overwrite the old one as it is useless now.
			newSendersBuffer = m_network_Factory.BuildSenderResources((*it));
		}
		//Now we DO have resources, and the new locator is already replacing the old one.
		for(auto mit= newSendersBuffer.begin(); mit!= newSendersBuffer.end(); ++mit){
			newSenders.push_back(std::move(*mit));	
		}
		
		//newSenders.insert(newSenders.end(), newSendersBuffer.begin(), newSendersBuffer.end());
		newSendersBuffer.clear();
	}
	for(auto mit=newSenders.begin(); mit!=newSenders.end();++mit){
		m_senderResource.push_back(std::move(*mit));
	}
	//m_senderResource.insert(m_senderResource.end(), newSenders.begin(), newSenders.end());
	m_att.defaultOutLocatorList = defcopy;

	if (!hasLocatorsDefined)
		//logInfo(RTPS_PARTICIPANT, m_att.getName() << " Created with NO default Send Locator List, adding Locators: " << m_att.defaultOutLocatorList);

	//logInfo(RTPS_PARTICIPANT,"RTPSParticipant \"" <<  m_att.getName() << "\" with guidPrefix: " <<m_guid.guidPrefix);
	//START BUILTIN PROTOCOLS
	mp_builtinProtocols = new BuiltinProtocols();
	if(!mp_builtinProtocols->initBuiltinProtocols(this,m_att.builtin))
	{
		//logWarning(RTPS_PARTICIPANT, "The builtin protocols were not corecctly initialized");
	}
	//eClock::my_sleep(300);
}

std::vector<RTPSWriter*> RTPSParticipantImpl::getAllWriters() const
{
   return m_allWriterList;
}

RTPSParticipantImpl::~RTPSParticipantImpl()
{
	//const char* const METHOD_NAME = "~RTPSParticipantImpl";
	//logInfo(RTPS_PARTICIPANT,"removing "<<this->getGuid());


	while(m_userReaderList.size()>0)
		RTPSDomain::removeRTPSReader(*m_userReaderList.begin());

	while(m_userWriterList.size()>0)
		RTPSDomain::removeRTPSWriter(*m_userWriterList.begin());

	//Destruct ReceiverResources
   for (auto& block : m_receiverResourcelist)
   {
      block.resourceAlive = false;
      block.Receiver.Abort();
      block.m_thread->join();
      delete block.m_thread;
      delete block.mp_receiver;
   }

   m_receiverResourcelist.clear();
	delete(this->mp_builtinProtocols);
	delete(this->mp_ResourceSemaphore);
	delete(this->mp_userParticipant);

	m_senderResource.clear();
	delete(this->mp_event_thr);
	delete(this->mp_mutex);
}

/*
 *
 * MAIN RTPSParticipant IMPL API
 *
 */


bool RTPSParticipantImpl::createWriter(RTPSWriter** WriterOut,
		WriterAttributes& param,WriterHistory* hist,WriterListener* listen, const EntityId_t& entityId,bool isBuiltin)
{
	//const char* const METHOD_NAME = "createWriter";
	std::string type = (param.endpoint.reliabilityKind == RELIABLE) ? "RELIABLE" :"BEST_EFFORT";
	//logInfo(RTPS_PARTICIPANT," of type " << type,C_B_YELLOW);
	EntityId_t entId;
	if(entityId== c_EntityId_Unknown)
	{
		if(param.endpoint.topicKind == NO_KEY)
			entId.value[3] = 0x03;
		else if(param.endpoint.topicKind == WITH_KEY)
			entId.value[3] = 0x02;
		uint32_t idnum;
		if(param.endpoint.getEntityID()>0)
			idnum = param.endpoint.getEntityID();
		else
		{
			IdCounter++;
			idnum = IdCounter;
		}

		octet* c = (octet*)&idnum;
		entId.value[2] = c[0];
		entId.value[1] = c[1];
		entId.value[0] = c[2];
		if(this->existsEntityId(entId,WRITER))
		{
			//logError(RTPS_PARTICIPANT,"A writer with the same entityId already exists in this RTPSParticipant");
			return false;
		}
	}
	else
	{
		entId = entityId;
	}
	if(!param.endpoint.unicastLocatorList.isValid())
	{
		//logError(RTPS_PARTICIPANT,"Unicast Locator List for Writer contains invalid Locator");
		return false;
	}
	if(!param.endpoint.multicastLocatorList.isValid())
	{
		//logError(RTPS_PARTICIPANT,"Multicast Locator List for Writer contains invalid Locator");
		return false;
	}

	RTPSWriter* SWriter = nullptr;
	GUID_t guid(m_guid.guidPrefix,entId);
	if(param.endpoint.reliabilityKind == BEST_EFFORT)
		SWriter = (RTPSWriter*)new StatelessWriter(this,guid,param,hist,listen);
	else if(param.endpoint.reliabilityKind == RELIABLE)
		SWriter = (RTPSWriter*)new StatefulWriter(this,guid,param,hist,listen);

	if(SWriter==nullptr)
		return false;

	createSendResources((Endpoint *)SWriter);
	if(param.endpoint.reliabilityKind == RELIABLE)
	{
		if (!createAndAssociateReceiverswithEndpoint((Endpoint *)SWriter))
		{
			delete(SWriter);
			return false;
		}
	}

   // Asynchronous thread runs regardless of mode because of
   // nack response duties.
   AsyncWriterThread::addWriter(*SWriter);

	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	m_allWriterList.push_back(SWriter);
	if(!isBuiltin)
		m_userWriterList.push_back(SWriter);
	*WriterOut = SWriter;

   for (const auto& descriptor : param.sizeFilters)
   {
      std::unique_ptr<FlowFilter> filter(new SizeFilter(descriptor, SWriter));
      SWriter->add_flow_filter(std::move(filter));
   }
	return true;
}


bool RTPSParticipantImpl::createReader(RTPSReader** ReaderOut,
		ReaderAttributes& param,ReaderHistory* hist,ReaderListener* listen, const EntityId_t& entityId,bool isBuiltin, bool enable)
{
	//const char* const METHOD_NAME = "createReader";
	std::string type = (param.endpoint.reliabilityKind == RELIABLE) ? "RELIABLE" :"BEST_EFFORT";
	//logInfo(RTPS_PARTICIPANT," of type " << type,C_B_YELLOW);
	EntityId_t entId;
	if(entityId== c_EntityId_Unknown)
	{
		if(param.endpoint.topicKind == NO_KEY)
			entId.value[3] = 0x04;
		else if(param.endpoint.topicKind == WITH_KEY)
			entId.value[3] = 0x07;
		uint32_t idnum;
		if(param.endpoint.getEntityID()>0)
			idnum = param.endpoint.getEntityID();
		else
		{
			IdCounter++;
			idnum = IdCounter;
		}

		octet* c = (octet*)&idnum;
		entId.value[2] = c[0];
		entId.value[1] = c[1];
		entId.value[0] = c[2];
		if(this->existsEntityId(entId,WRITER))
		{
			//logError(RTPS_PARTICIPANT,"A reader with the same entityId already exists in this RTPSParticipant");
			return false;
		}
	}
	else
	{
		entId = entityId;
	}
	if(!param.endpoint.unicastLocatorList.isValid())
	{
		//logError(RTPS_PARTICIPANT,"Unicast Locator List for Reader contains invalid Locator");
		return false;
	}
	if(!param.endpoint.multicastLocatorList.isValid())
	{
		//logError(RTPS_PARTICIPANT,"Multicast Locator List for Reader contains invalid Locator");
		return false;
	}
	RTPSReader* SReader = nullptr;
	GUID_t guid(m_guid.guidPrefix,entId);
	if(param.endpoint.reliabilityKind == BEST_EFFORT)
		SReader = (RTPSReader*)new StatelessReader(this,guid,param,hist,listen);
	else if(param.endpoint.reliabilityKind == RELIABLE)
		SReader = (RTPSReader*)new StatefulReader(this,guid,param,hist,listen);

	if(SReader==nullptr)
		return false;

	//SReader->setListener(inlisten);
	//SReader->setQos(param.qos,true);
	if (param.endpoint.reliabilityKind == RELIABLE)
		createSendResources((Endpoint *)SReader);

	if(isBuiltin)
	{
		SReader->setTrustedWriter(TrustedWriter(SReader->getGuid().entityId));
	}

    if(enable)
    {
		if (!createAndAssociateReceiverswithEndpoint((Endpoint *)SReader))
        {
            delete(SReader);
            return false;
        }
    }

	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	m_allReaderList.push_back(SReader);
	if(!isBuiltin)
		m_userReaderList.push_back(SReader);
	*ReaderOut = SReader;
	return true;
}

bool RTPSParticipantImpl::enableReader(RTPSReader *reader)
{
    if(!assignEndpointListenResources((Endpoint*)reader))
    {
        return false;
    }

    return true;
}




bool RTPSParticipantImpl::registerWriter(RTPSWriter* Writer,TopicAttributes& topicAtt,WriterQos& wqos)
{
	return this->mp_builtinProtocols->addLocalWriter(Writer,topicAtt,wqos);
}

bool RTPSParticipantImpl::registerReader(RTPSReader* reader,TopicAttributes& topicAtt,ReaderQos& rqos)
{
	return this->mp_builtinProtocols->addLocalReader(reader,topicAtt,rqos);
}

bool RTPSParticipantImpl::updateLocalWriter(RTPSWriter* Writer,WriterQos& wqos)
{
	return this->mp_builtinProtocols->updateLocalWriter(Writer,wqos);
}

bool RTPSParticipantImpl::updateLocalReader(RTPSReader* reader,ReaderQos& rqos)
{
	return this->mp_builtinProtocols->updateLocalReader(reader,rqos);
}

/*
 *
 * AUXILIARY METHODS
 *
 *  */


bool RTPSParticipantImpl::existsEntityId(const EntityId_t& ent,EndpointKind_t kind) const
{
	if(kind == WRITER)
	{
		for(std::vector<RTPSWriter*>::const_iterator it = m_userWriterList.begin();
				it!=m_userWriterList.end();++it)
		{
			if(ent == (*it)->getGuid().entityId)
				return true;
		}
	}
	else
	{
		for(std::vector<RTPSReader*>::const_iterator it = m_userReaderList.begin();
				it!=m_userReaderList.end();++it)
		{
			if(ent == (*it)->getGuid().entityId)
				return true;
		}
	}
	return false;
}


/*
 *
 * RECEIVER RESOURCE METHODS
 *
 */


bool RTPSParticipantImpl::assignEndpointListenResources(Endpoint* endp)
{
	//Tag the endpoint with the ReceiverResources
	//const char* const METHOD_NAME = "assignEndpointListenResources";
	bool valid = true;

	/* No need to check for emptiness on the lists, as it was already done on part function
	In case are using the default list of Locators they have already been embedded to the parameters */

	//UNICAST
	assignEndpoint2LocatorList(endp, endp->getAttributes()->unicastLocatorList);
	//MULTICAST
	assignEndpoint2LocatorList(endp, endp->getAttributes()->multicastLocatorList);
	return valid;
}

bool RTPSParticipantImpl::createAndAssociateReceiverswithEndpoint(Endpoint * pend){
	/*	This function...
		- Asks the network factory for new resources
		- Encapsulates the new resources within the ReceiverControlBlock list
		- Associated the endpoint to the new elements in the list
		- Launches the listener thread
	*/
	// 1 - Ask the network factory to generate the elements that do still not exist
	std::vector<ReceiverResource> newItems;							//Store the newly created elements
	std::vector<ReceiverResource> newItemsBuffer;					//Store intermediate results
	//Iterate through the list of unicast and multicast locators the endpoint has... unless its empty
	//In that case, just use the standard
	if (pend->getAttributes()->unicastLocatorList.empty()){
		//Default unicast
		pend->getAttributes()->unicastLocatorList = m_att.defaultUnicastLocatorList;
	}
	createReceiverResources(pend->getAttributes()->unicastLocatorList, false);
		
	if (pend->getAttributes()->multicastLocatorList.empty()){
		//Default Multicast
		pend->getAttributes()->multicastLocatorList = m_att.defaultMulticastLocatorList;
	}
	createReceiverResources(pend->getAttributes()->multicastLocatorList, false);
	// Associate the Endpoint with ReceiverResources inside ReceiverControlBlocks
	assignEndpointListenResources(pend); 
	return true;
}

void RTPSParticipantImpl::performListenOperation(ReceiverControlBlock *receiver, Locator_t input_locator)
{
   while(receiver->resourceAlive)
   {	
      // Blocking receive.
      if(!receiver->Receiver.Receive(receiver->m_receiveBuffer, input_locator))
         continue;

      // Wraps a CDRMessage around the underlying vector array.
      CDRMessage::wrapVector(&(receiver->mp_receiver->m_rec_msg), receiver->m_receiveBuffer);

      // Processes the data through the CDR Message interface.
      receiver->mp_receiver->processCDRMsg(getGuid().guidPrefix, &input_locator, &receiver->mp_receiver->m_rec_msg);
   }	
}


bool RTPSParticipantImpl::assignEndpoint2LocatorList(Endpoint* endp,LocatorList_t& list)
{
	/* Note:
		The previous version of this function associated (or created) ListenResources and added the endpoint to them.
		It then requested the list of Locators the Listener is listening to and appended to the LocatorList_t from the paremeters.
		
		This has been removed becuase it is considered redundant. For ReceiveResources that listen on multiple interfaces, only
		one of the supported Locators is needed to make the match, and the case of new ListenResources being created has been removed
		since its the NetworkFactory the one that takes care of Resource creation.
	 */
	LocatorList_t finalList;
	for(auto lit = list.begin();lit != list.end();++lit){
		//Iteration of all Locators within the Locator list passed down as argument
		boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
		//Check among ReceiverResources whether the locator is supported or not
		for (auto it = m_receiverResourcelist.begin(); it != m_receiverResourcelist.end(); ++it){
			//Take mutex for the resource since we are going to interact with shared resources
			//boost::lock_guard<boost::mutex> guard((*it).mtx);
			if ((*it).Receiver.SupportsLocator(*lit)){
				//Supported! Take mutex and update lists - We maintain reader/writer discrimination just in case
				(*it).mp_receiver->associateEndpoint(endp);	
				// end association between reader/writer and the receive resources
			}

		}
		//Finished iteratig through all ListenResources for a single Locator (from the parameter list).
		//Since this function is called after checking with NetFactory we do not have to create any more resource. 
	}
	return true;
}
bool RTPSParticipantImpl::createSendResources(Endpoint *pend){
	std::vector<SenderResource> newSenders;
	std::vector<SenderResource> SendersBuffer;
	if (pend->m_att.outLocatorList.empty()){
		//Output locator ist is empty, use predetermined ones
		pend->m_att.outLocatorList = m_att.defaultOutLocatorList;		//Tag the Endpoint with the Default list so it can use it to send
		//Already created them on constructor, so we can skip the creation
		return true;
	}
	//Output locators have been specified, create them
	for (auto it = pend->m_att.outLocatorList.begin(); it != pend->m_att.outLocatorList.end(); ++it){
		SendersBuffer = m_network_Factory.BuildSenderResources((*it));
		for(auto mit = SendersBuffer.begin(); mit!= SendersBuffer.end(); ++mit){
			newSenders.push_back(std::move(*mit));
		}
		//newSenders.insert(newSenders.end(), SendersBuffer.begin(), SendersBuffer.end());
		SendersBuffer.clear();
	}
	for(auto mit = SendersBuffer.begin();mit!=SendersBuffer.end();++mit){
		m_senderResource.push_back(std::move(*mit));
	}
	//m_senderResource.insert(m_senderResource.end(), SendersBuffer.begin(), SendersBuffer.end());

	return true;
}

void RTPSParticipantImpl::createReceiverResources(LocatorList_t& Locator_list, bool ApplyMutation){
	std::vector<ReceiverResource> newItemsBuffer;

	for(auto it_loc = Locator_list.begin(); it_loc != Locator_list.end(); ++it_loc){
		newItemsBuffer = m_network_Factory.BuildReceiverResources((*it_loc));
		if(ApplyMutation){
         int tries = 0;
			while(newItemsBuffer.empty() && (tries < MutationTries)){
            tries++;
				(*it_loc) = applyLocatorAdaptRule(*it_loc);
				newItemsBuffer = m_network_Factory.BuildReceiverResources((*it_loc));
			}	
		}
		for(auto it_buffer = newItemsBuffer.begin(); it_buffer != newItemsBuffer.end(); ++it_buffer){
			//Push the new items into the ReceiverResource buffer
			m_receiverResourcelist.push_back(ReceiverControlBlock(std::move(*it_buffer)));
			//Create and init the MessageReceiver
			m_receiverResourcelist.back().mp_receiver = new MessageReceiver();
      			m_receiverResourcelist.back().mp_receiver->init(m_att.listenSocketBufferSize);

         //Reserve the receive buffer
         m_receiverResourcelist.back().m_receiveBuffer.reserve(m_att.listenSocketBufferSize);
			//Init the thread
			m_receiverResourcelist.back().m_thread = new boost::thread(&RTPSParticipantImpl::performListenOperation,this, &(m_receiverResourcelist.back()),(*it_loc));
		}
		newItemsBuffer.clear();
	}	
}



bool RTPSParticipantImpl::deleteUserEndpoint(Endpoint* p_endpoint)
{
	for(auto it=m_receiverResourcelist.begin();it!=m_receiverResourcelist.end();++it){
		(*it).mp_receiver->removeEndpoint(p_endpoint);
	}
	bool found = false;
	{
		if(p_endpoint->getAttributes()->endpointKind == WRITER)
		{
			boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
			for(auto wit=m_userWriterList.begin();
					wit!=m_userWriterList.end();++wit)
			{
				if((*wit)->getGuid().entityId == p_endpoint->getGuid().entityId) //Found it
				{
					m_userWriterList.erase(wit);
					found = true;
					break;
				}
			}
		}
		else
		{
			boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
			for(auto rit=m_userReaderList.begin()
					;rit!=m_userReaderList.end();++rit)
			{
				if((*rit)->getGuid().entityId == p_endpoint->getGuid().entityId) //Found it
				{
					m_userReaderList.erase(rit);
					found = true;
					break;
				}
			}
		}
		if(!found)
			return false;
		//REMOVE FOR BUILTINPROTOCOLS
		if(p_endpoint->getAttributes()->endpointKind == WRITER)
			mp_builtinProtocols->removeLocalWriter((RTPSWriter*)p_endpoint);
		else
			mp_builtinProtocols->removeLocalReader((RTPSReader*)p_endpoint);
		//BUILTINPROTOCOLS
		boost::lock_guard<boost::recursive_mutex> guardParticipant(*mp_mutex);
	}
	//	boost::lock_guard<boost::recursive_mutex> guardEndpoint(*p_endpoint->getMutex());
	delete(p_endpoint);
	return true;
}


ResourceEvent& RTPSParticipantImpl::getEventResource()
{
	return *this->mp_event_thr;
}

void RTPSParticipantImpl::sendSync(CDRMessage_t* msg, Endpoint *pend, const Locator_t& destination_loc)
{
	for (auto sit = pend->m_att.outLocatorList.begin(); sit != pend->m_att.outLocatorList.end(); ++sit){
		for (auto it = m_senderResource.begin(); it != m_senderResource.end(); ++it){
			if ((*it).SupportsLocator((*sit))){
				(*it).Send(msg->buffer, msg->length, destination_loc);
			}
		}
	}
}

void RTPSParticipantImpl::announceRTPSParticipantState()
{
	return mp_builtinProtocols->announceRTPSParticipantState();
}

void RTPSParticipantImpl::stopRTPSParticipantAnnouncement()
{
	return mp_builtinProtocols->stopRTPSParticipantAnnouncement();
}

void RTPSParticipantImpl::resetRTPSParticipantAnnouncement()
{
	return mp_builtinProtocols->resetRTPSParticipantAnnouncement();
}

void RTPSParticipantImpl::loose_next_change()
{
	//NOTE: This is replaced by the test transport
	//this->mp_send_thr->loose_next_change();
}


bool RTPSParticipantImpl::newRemoteEndpointDiscovered(const GUID_t& pguid, int16_t userDefinedId,EndpointKind_t kind)
{
	//const char* const METHOD_NAME = "newRemoteEndpointDiscovered";
	if(m_att.builtin.use_STATIC_EndpointDiscoveryProtocol == false)
	{
		//logWarning(RTPS_PARTICIPANT,"Remote Endpoints can only be activated with static discovery protocol");
		return false;
	}
	return mp_builtinProtocols->mp_PDP->newRemoteEndpointStaticallyDiscovered(pguid,userDefinedId,kind);
}

void RTPSParticipantImpl::ResourceSemaphorePost()
{
	if(mp_ResourceSemaphore != nullptr)
	{
		mp_ResourceSemaphore->post();
	}
}

void RTPSParticipantImpl::ResourceSemaphoreWait()
{
	if (mp_ResourceSemaphore != nullptr)
	{
		mp_ResourceSemaphore->wait();
	}

}

void RTPSParticipantImpl::assertRemoteRTPSParticipantLiveliness(const GuidPrefix_t& guidP)
{
	this->mp_builtinProtocols->mp_PDP->assertRemoteParticipantLiveliness(guidP);
}

}
} /* namespace rtps */
} /* namespace eprosima */


