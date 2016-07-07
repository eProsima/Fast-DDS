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
 * @file SerializedPayload.h 	
 */

#ifndef SERIALIZEDPAYLOAD_H_
#define SERIALIZEDPAYLOAD_H_
#include "../../fastrtps_dll.h"
#include "Types.h"
#include <cstring>
#include <new>
#include <stdexcept>
#include <stdint.h>
#include <stdlib.h>
#include <fastrtps/rtps/resources/ResourceManagement.h>

/*!
 * @brief Maximum payload is maximum of UDP packet size minus 536bytes (RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE)
 * With those 536 bytes (RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE) bytes is posible to send RTPS Header plus RTPS Data submessage plus RTPS Heartbeat submessage.
 */
#define PAYLOAD_MAX_SIZE 65000u

namespace eprosima{
    namespace fastrtps{
        namespace rtps{

//Pre define data encapsulation schemes
#define CDR_BE 0x0000
#define CDR_LE 0x0001
#define PL_CDR_BE 0x0002
#define PL_CDR_LE 0x0003


            //!@brief Structure SerializedPayload_t.
            //!@ingroup COMMON_MODULE
            struct RTPS_DllAPI SerializedPayload_t
            {
                
		//!Encapsulation of the data as suggested in the RTPS 2.1 specification chapter 10.
                uint16_t encapsulation;
                //!Actual length of the data
                uint32_t length;
                //!Pointer to the data.
                octet* data;
                //!Maximum size of the payload
                uint32_t max_size;
                //!Position when reading
                uint32_t pos;
		//!Behaviour on memory assignment
		MemoryManagementPolicy_t memoryMode;

                //!Default constructor
                SerializedPayload_t()
                : encapsulation(CDR_BE), length(0), data(nullptr), max_size(0),
                  pos(0), memoryMode(DYNAMIC_RESERVE_MEMORY_MODE) 
                {}

                /**
                 * @param len Maximum size of the payload
                 * @param allow_resize If true the payload can be resized dynamically.
                 */
                SerializedPayload_t(uint32_t len, MemoryManagementPolicy_t policy)
                : SerializedPayload_t()
                {
                    memoryMode = policy;
                    this->reserve_(len);
                }

                ~SerializedPayload_t()
                {
                    this->empty();
                }

                /*!
                 * Copy another structure (including allocating new space for the data.)
                 * @param[in] serData Pointer to the structure to copy
                 * @param with_limit if true, the function will fail when providing a payload too big
                 * @return True if correct
                 */
                bool copy(SerializedPayload_t* serData, bool with_limit = true)
                {
		    // If this payload comes initializated but however it is smalled than what is copied 
		    //if((length < serData->length) & (data != nullptr))
		    //{
		    //	free(data);
		    //}
		    length = serData->length;

                    if(serData->length > max_size)
                    {
                        if(with_limit)
                            return false;
                        else
                            this->reserve_(serData->length);
                    }
                    encapsulation = serData->encapsulation;
                    memcpy(data, serData->data, length);
                    return true;
                }

		bool set_payload(octet *target_data,uint32_t target_length)
		{
			if( (memoryMode==PREALLOCATED_MEMORY_MODE) & (target_length > max_size))
			{
				return false;
			}
			//Possible resize of the buffer only happens in (semi) dynamic mode
			if( (memoryMode!=PREALLOCATED_MEMORY_MODE) )
			{
				if(length < target_length)
				{
					//Data does not fit, resize
					if(data != nullptr)
					{
						octet * temp_pointer = (octet*)realloc(data,target_length*sizeof(octet));
						//Why this? Because if the block needs to be moved elsewhere realloc gives a pointer to the new position
						if(data!=temp_pointer)
							data = temp_pointer;
					}
					else
					{
						data = (octet *)calloc(target_length,sizeof(octet));
					}
				}
			}
			length = target_length;
			memcpy(data, target_data, length);
			return true;
		}		
	
		/*!
		* Allocate new space for fragmented data
		* @param[in] serData Pointer to the structure to copy
		* @return True if correct
		*/
		bool reserve_fragmented(SerializedPayload_t* serData)
		{
			length = serData->length;
			max_size = serData->length;
			encapsulation = serData->encapsulation;
			data = (octet*)calloc(length, sizeof(octet));
			return true;
		}

                //! Empty the payload
                void empty()
                {
                    length= 0;
                    encapsulation = CDR_BE;
                    max_size = 0;
                    if(data!=nullptr)
                        free(data);
                    data = nullptr;
                }

                void reserve(size_t new_size)
                {
                    if (memoryMode == PREALLOCATED_MEMORY_MODE) {
                        throw std::length_error("instance of SerializedPayload_t is not resizable");
                    }
                    return reserve_(new_size);
                }

                protected:
                void reserve_(size_t new_size)
                {
                    if (new_size <= this->max_size) {
                        return;
                    }
                    void * old_data = data;
                    data = (octet*)realloc(data, new_size);
                    if (!data) {
                        free(old_data);
                        throw std::bad_alloc();
                    }
                    max_size = new_size;
                }
            };
        }
    }
}

#endif /* SERIALIZEDPAYLOAD_H_ */
