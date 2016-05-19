#ifndef MAPABLEKEY_H_
#define MAPABLEKEY_H_

#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/attributes/SubscriberAttributes.h>

#include <fastrtps/Domain.h>

typedef struct ex_mapableKey{

	octet value[16];
	
	ex_mapableKey& operator=(const InstanceHandle_t& ihandle){
		for(uint8_t i=0;i<16;i++)
		{
			value[i] = ihandle.value[i];
		}
		return *this;
	}

}mapable_key;

inline bool operator<(const ex_mapableKey& ex_mapableKey1,const ex_mapableKey& ex_mapableKey2)
{
	for(uint8_t i=0;i<16;++i){
		if(ex_mapableKey1.value[i] < ex_mapableKey2.value[i])
			return true;
	}
	return false;
}



#endif