/********/
Internal ePRosima doc - New features in this branch
/********/

--- Query the number of Publishers and Subscribers listening to a topic

Participant has 2 new API functions, get_no_publishers and get_no_subscribers
These functions a target_topic as argument and return the number of pubs/subs
that exist on a participant that are assigned to the specified topic

--- Possibility to attach a secondary ReaderListener to SimpleEDP RTPSReaders

A new Participant API function, getEDPReaders, allows the user to get pointer
to the two stateful readers the EDP spawns. From here it is possible to access
the ReaderListener, which has been modified.

The ReaderListeners used in the EDP Readers inherit from a class that allows a
slave ReaderListener to be attached. This way a user can attach his own
ReaderListener so that its callback is executed whenever the default EDP
callback is invoked. 

The result of this is that there is a path from user land to the EDP
RTPSReaders, then to the modified ReaderListener and from there we can
attach a user defined ReaderListener with its own callbacks.

--- Examples:

	---Attach a slave ReaderListener:
	- The target slave ReaderListener inherits from ReaderListener

class gettopicnamesandtypesReaderListener:public ReaderListener
{
	public:
	std::mutex mapmutex;
	std::map<std::string,std::set<std::string>> topicNtypes;
	void onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change_in){
		CacheChange_t* change = (CacheChange_t*) change_in;
		if(change->kind == ALIVE){
			WriterProxyData proxyData;
			CDRMessage_t tempMsg;
			tempMsg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND:LITTLEEND;
			tempMsg.length = change->serializedPayload.length;
			memcpy(tempMsg.buffer,change->serializedPayload.data,tempMsg.length);
			if(proxyData.readFromCDRMessage(&tempMsg)){
				mapmutex.lock();
				topicNtypes[proxyData.m_topicName].insert(proxyData.m_typeName);		
				mapmutex.unlock();
			}
		}
	}
};

	- Insertion  from within application code:

std::pair<StatefulReader*,StatefulReader*> EDP_Readers = my_participant->getEDPReaders();
	InfectableReaderListener* target = dynamic_cast<InfectableReaderListener*>(EDP_Readers.second->getListener());
	target->attachListener(slave_listener);
	result = target->hasReaderAttached();
	ASSERT_EQ(result,true);
	slave_target =  dynamic_cast<gettopicnamesandtypesReaderListener*>(target->getAttachedListener());


	--- Poll the no of Pubs and Subs on topic "TEST_NAME"

std::string my_topicName("TEST_NAME");
int no_pubs = my_participant->get_no_publishers(my_topicName.cstr());

