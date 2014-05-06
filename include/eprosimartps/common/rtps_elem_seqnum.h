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
#include <algorithm>
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
	SequenceNumber_t& operator+=(int inc)
	{
		if(this->low+inc>pow(2.0,32))
		{
			int module = (int)floor((inc+this->low)/pow(2.0,32));
			this->high+=module;
			this->low +=inc-((uint32_t)pow(2.0,32)*module);
		}
		else
		{	this->low+=inc;
		}

		return *this;
	}

	bool operator>(SequenceNumber_t& seq2){
		return this->to64long() > seq2.to64long();
	}
	bool operator<(SequenceNumber_t& seq2){
		return this->to64long() < seq2.to64long();
	}
	bool operator>=(SequenceNumber_t& seq2){
		return this->to64long() >= seq2.to64long();
	}
	bool operator<=(SequenceNumber_t& seq2){
		return this->to64long()<= seq2.to64long();
	}
} SequenceNumber_t;

inline SequenceNumber_t operator-(SequenceNumber_t seq,uint32_t inc)
{
	//FIXME: repare function for when inc is greater than pow 2, 32
	if(seq.low-inc < 0)
	{
		seq.high--;
		seq.low = (uint32_t)(pow(2.0,32)-(inc-seq.low));
	}
	else
		seq.low-=(uint32_t)inc;
	return seq;
}
inline SequenceNumber_t operator+(SequenceNumber_t seq,uint64_t inc){

	if(seq.low+inc>pow(2.0,32))
	{
		int module = (int)floor((inc+seq.low)/pow(2.0,32));
		seq.high+=module;
		seq.low +=(uint32_t)(inc-(pow(2.0,32)*module));
	}
	else
		seq.low+=(uint32_t)inc;
	return seq;
}


#define SEQUENCENUMBER_UNKOWN(sq) {sq.high=-1;sq.low=0;}

inline bool sort_seqNum (SequenceNumber_t s1,SequenceNumber_t s2)
{
	return(s1.to64long() < s2.to64long());
}

//!Structure SequenceNumberSet_t, contains a group of sequencenumbers.
typedef class SequenceNumberSet_t{
public:
	SequenceNumber_t base;
	SequenceNumberSet_t& operator=(const SequenceNumberSet_t& set2)
	{
		base = set2.base;
		set = set2.set;
		return *this;
	}
	bool add(SequenceNumber_t& in)
	{
		uint64_t base64 = base.to64long();
		uint64_t in64 = in.to64long();
		if(in64 >= base64 && in64<=base64+255 )
			set.push_back(in);
		else
			return false;
		return true;
	}
	SequenceNumber_t get_maxSeqNum()
	{
		return *std::max_element(set.begin(),set.end(),sort_seqNum);
	}
	bool isSetEmpty()
		{
			return set.empty();
		}
	std::vector<SequenceNumber_t>::iterator get_begin()
	{
		return set.begin();
	}
	std::vector<SequenceNumber_t>::iterator get_end()
	{
		return set.end();
	}
	size_t get_size()
	{
		return set.size();
	}
	std::vector<SequenceNumber_t> get_set()
		{
		return set;
		}
private:
	std::vector<SequenceNumber_t> set;
}SequenceNumberSet_t;

}
}



#endif /* RPTS_ELEM_SEQNUM_H_ */
