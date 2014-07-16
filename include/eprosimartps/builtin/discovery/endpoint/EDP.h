/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDP.h
 *
 */

#ifndef EDP_H_
#define EDP_H_

namespace eprosima {
namespace rtps {

class PDPSimple;

class EDP {
public:
	EDP(PDPSimple* p);
	virtual ~EDP();

	/**
	 * Abstract method to initialize the EDP.
	 * @param attributes DiscoveryAttributes structure.
	 * @return True if correct.
	 */
	virtual bool initEDP(DiscoveryAttributes& attributes)=0;


private:


	PDPSimple* mp_PDP;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* EDP_H_ */
