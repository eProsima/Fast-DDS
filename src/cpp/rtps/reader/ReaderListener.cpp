/******

Implementation of functions for InfectableReaderListener

*********/

#include <fastrtps/include/rtps/reader/ReaderListener.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{


void InfectableReaderListener::attachListener(ReaderListener *secondary_listener){
	attached_listener = secondary_listener;
}

void InfectableReaderListener::detachListener(){
	attached_listener = nullptr;
}

bool InfectableReaderListener::hasReaderAttached(){
	if(attached_listener != nullprt)
		return true;
	return false;
}

