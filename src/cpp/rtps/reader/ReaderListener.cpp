/******

Implementation of functions for InfectableReaderListener

*********/

#include <fastrtps/rtps/reader/ReaderListener.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{


void InfectableReaderListener::attachListener(ReaderListener *secondary_listener){
	attached_listener_mutex.lock();
	attached_listener = secondary_listener;
	attached_listener_mutex.unlock();

}

void InfectableReaderListener::detachListener(){
	attached_listener_mutex.lock();
	attached_listener = nullptr;
	attached_listener_mutex.unlock();
}

bool InfectableReaderListener::hasReaderAttached(){
	if(attached_listener != nullptr)
		return true;
	return false;
}
ReaderListener* InfectableReaderListener::getAttachedListener(){
	if(attached_listener != nullptr)
		return attached_listener;
	return nullptr;
}	

//Namespace ends
}}}
