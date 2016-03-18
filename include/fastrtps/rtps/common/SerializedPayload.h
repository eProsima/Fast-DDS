/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SerializedPayload.h 	
 */

#ifndef SERIALIZEDPAYLOAD_H_
#define SERIALIZEDPAYLOAD_H_
#include "../../fastrtps_dll.h"
#include "Types.h"
#include <cstring>
#include <stdint.h>
#include <stdlib.h>

/*!
 * @brief Maximum payload is maximum of UDP packet size minus 100bytes.
 * With those 100 bytes is posible to send RTPS Header plus RTPS Data submessage plus RTPS Heartbeat submessage.
 */
#define PAYLOAD_MAX_SIZE 65430u

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

                //!Default constructor
                SerializedPayload_t()
                {
                    length = 0;
                    data = nullptr;
                    encapsulation = CDR_BE;
                    max_size = 0;
                    pos = 0;
                }

                /**
                 * @param len Maximum size of the payload
                 */
                SerializedPayload_t(uint32_t len)
                {
                    encapsulation = CDR_BE;
                    length = 0;
                    data = (octet*)calloc(len, sizeof(octet));
                    max_size = len;
                    pos = 0;
                }

                ~SerializedPayload_t()
                {
                    this->empty();
                }

                /*!
                 * Copy another structure (including allocating new space for the data.)
                 * @param[in] serData Pointer to the structure to copy
                 * @return True if correct
                 */
                bool copy(SerializedPayload_t* serData, bool with_limit = true)
                {
                    length = serData->length;

                    if(serData->length > max_size)
                    {
                        if(with_limit)
                            return false;
                        else
                            length = max_size;
                    }
                    encapsulation = serData->encapsulation;
                    if(data == nullptr)
                        data = (octet*)calloc(length, sizeof(octet));
                    memcpy(data,serData->data,length);
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
            };
        }
    }
}

#endif /* SERIALIZEDPAYLOAD_H_ */
