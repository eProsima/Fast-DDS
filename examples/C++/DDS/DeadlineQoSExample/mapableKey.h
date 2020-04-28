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

#ifndef MAPABLEKEY_H_
#define MAPABLEKEY_H_

#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/attributes/SubscriberAttributes.h>

#include <fastrtps/Domain.h>

typedef struct ex_mapableKey{

	eprosima::fastrtps::rtps::octet value[16];
	
	ex_mapableKey& operator=(const eprosima::fastrtps::rtps::InstanceHandle_t& ihandle){
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
