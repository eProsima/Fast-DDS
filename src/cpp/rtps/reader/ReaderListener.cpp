/******

Implementation of functions for InfectableReaderListener

*********/

#include <fastrtps/rtps/reader/ReaderListener.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{


void CompoundReaderListener::attachListener(ReaderListener *secondary_listener){
	attached_listener_mutex.lock();
	attached_listener = secondary_listener;
	attached_listener_mutex.unlock();

}

void CompoundReaderListener::detachListener(){
	attached_listener_mutex.lock();
	attached_listener = nullptr;
	attached_listener_mutex.unlock();
}

bool CompoundReaderListener::hasReaderAttached(){
	if(attached_listener != nullptr)
		return true;
	return false;
}
ReaderListener* CompoundReaderListener::getAttachedListener(){
	if(attached_listener != nullptr)
		return attached_listener;
	return nullptr;
}	

//Namespace ends
}}}
