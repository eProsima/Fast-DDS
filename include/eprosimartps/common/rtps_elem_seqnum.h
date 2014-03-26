/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file rtps_elem_seqnum.h
 *	SequenceNumber definition.
 *  Created on: Feb 28, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef RPTS_ELEM_SEQNUM_H_
#define RPTS_ELEM_SEQNUM_H_

namespace eprosima{
namespace rtps{


//!@brief Structure SequenceNumber_t, different for each change in the same writer.
typedef struct SequenceNumber_t{
	int32_t high;
	uint32_t low;
	SequenceNumber_t(){
		high = 0;
		low = 0;
	}
	//!Convert the number to 64 bit.
	uint64_t to64long(){
		return ((uint64_t)high *(uint64_t)pow(2.0,32) + (uint64_t)low);
	}
	SequenceNumber_t& operator=(const SequenceNumber_t& seq)
	{
		high = seq.high;
		low = seq.low;
		return *this;
	}
	//!Compares two SequenceNumber_t.
	bool operator==(SequenceNumber_t& sn){
		if(high != sn.high)
			return false;
		if(low != sn.low)
			return false;
		return true;
	}
	//!Increase SequenceNumber in 1.
	SequenceNumber_t& operator++(){
		if(low == pow(2.0,32))
		{high++;low = 0;}
		else
			low++;
		return *this;
	}
	SequenceNumber_t& operator++(int unused){
		if(low == pow(2.0,32))
		{high++;low = 0;}
		else
			low++;
		return *this;
	}
	SequenceNumber_t& operator+=(int inc){
		if(low+inc>pow(2,32))
		{high++;low +=inc-(pow(2,32)-low);}
		else
			low+=inc;
		return *this;
	}
	SequenceNumber_t& operator-(int inc){
		if(low-inc < 0)
		{
			high--;
			low = pow(2,32)-(inc-low);
		}
		else
			low-=inc;
		return *this;
	}
	bool operator>(SequenceNumber_t& seq2){
		return islarger(seq2);
	}
	bool operator<(SequenceNumber_t& seq2){
		return islarger(seq2);
	}
	bool operator>=(SequenceNumber_t& seq){
		if(islarger(seq) || *this==seq)
			return true;
		else
			return false;
	}
	bool operator<=(SequenceNumber_t& seq){
		if(!islarger(seq) || *this==seq)
			return true;
		else
			return false;
	}
	bool islarger(SequenceNumber_t& seq2){
		if(this->to64long() > seq2.to64long())
			return true;
		else
			return false;
	}
} SequenceNumber_t;

#define SEQUENCENUMBER_UNKOWN(sq) {sq.high=-1;sq.low=0;}

//!Structure SequenceNumberSet_t, contains a group of sequencenumbers.
typedef struct SequenceNumberSet_t{
	SequenceNumber_t base;
	std::vector<SequenceNumber_t> set;
	bool add(SequenceNumber_t& in){
		uint64_t base64 = base.to64long();
		uint64_t in64 = in.to64long();
		if(in64 >= base64 && in64<=base64+255 )
			set.push_back(in);
		else
			return false;
		return true;
	}
}SequenceNumberSet_t;

}
}



#endif /* RPTS_ELEM_SEQNUM_H_ */
