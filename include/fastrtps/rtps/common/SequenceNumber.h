/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SequenceNumber.h
 */

#ifndef RPTS_ELEM_SEQNUM_H_
#define RPTS_ELEM_SEQNUM_H_
#include "fastrtps/config/fastrtps_dll.h"
#include "fastrtps/rtps/common/Types.h"

#include <vector>
#include <cmath>
#include <algorithm>
#include <sstream>
namespace eprosima{
namespace fastrtps{
namespace rtps{


//!@brief Structure SequenceNumber_t, different for each change in the same writer.
struct RTPS_DllAPI SequenceNumber_t{
	int32_t high;
	uint32_t low;
	SequenceNumber_t(){
		high = 0;
		low = 0;
	}
	SequenceNumber_t(int32_t hi,uint32_t lo):
		high(hi),low(lo)
	{

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


};


//!Compares two SequenceNumber_t.
inline bool operator==(const SequenceNumber_t& sn1,const SequenceNumber_t& sn2)
								{
	if(sn1.high != sn2.high)
		return false;
	if(sn1.low != sn2.low)
		return false;
	return true;
								}
inline bool operator!=(const SequenceNumber_t& sn1,const SequenceNumber_t& sn2)
								{
	if(sn1.high != sn2.high)
		return true;
	if(sn1.low != sn2.low)
		return true;
	return false;
								}
inline bool operator>(const SequenceNumber_t& seq1, const SequenceNumber_t& seq2)
{
	if(seq1.high>seq2.high)
		return true;
	else if(seq1.high < seq2.high)
		return false;
	else
	{
		if(seq1.low>seq2.low)
			return true;
	}
	return false;
}
inline bool operator<(const SequenceNumber_t& seq1, const SequenceNumber_t& seq2)
{
	if(seq1.high>seq2.high)
		return false;
	else if(seq1.high < seq2.high)
		return true;
	else
	{
		if(seq1.low < seq2.low)
			return true;
	}
	return false;
}

inline bool operator>=(const SequenceNumber_t& seq1, const SequenceNumber_t& seq2)
{
	if(seq1.high>seq2.high)
		return true;
	else if(seq1.high < seq2.high)
		return false;
	else
	{
		if(seq1.low>=seq2.low)
			return true;
	}
	return false;
}

inline bool operator<=( const SequenceNumber_t& seq1, const  SequenceNumber_t& seq2)
{
	if(seq1.high>seq2.high)
			return false;
		else if(seq1.high < seq2.high)
			return true;
		else
		{
			if(seq1.low <= seq2.low)
				return true;
		}
		return false;
}



inline SequenceNumber_t operator-(SequenceNumber_t& seq,uint32_t inc)
{
	SequenceNumber_t res(seq);
	if((int64_t)res.low-(int64_t)inc < 0)
	{
		res.high--;
		res.low = (uint32_t)(pow(2.0,32)-(inc-seq.low));
	}
	else
		res.low-=(uint32_t)inc;
	return res;
}
inline SequenceNumber_t operator+(SequenceNumber_t& seqin,uint64_t inc){
	SequenceNumber_t seq(seqin);
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

const SequenceNumber_t c_SequenceNumber_Unknown(-1,0);

inline bool sort_seqNum (SequenceNumber_t& s1,SequenceNumber_t& s2)
{
	return(s1.to64long() < s2.to64long());
}

inline std::ostream& operator<<(std::ostream& output,const SequenceNumber_t& seqNum){
	return output << ((uint64_t)seqNum.high *(uint64_t)pow(2.0,32) + (uint64_t)seqNum.low);
}



//!Structure SequenceNumberSet_t, contains a group of sequencenumbers.
class SequenceNumberSet_t{
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
	std::string print()
	{
		std::stringstream ss;
		ss<<base.to64long()<<":";
		for(std::vector<SequenceNumber_t>::iterator it=set.begin();it!=set.end();++it)
			ss<<it->to64long()<<"-";
		return ss.str();
	}
private:
	std::vector<SequenceNumber_t> set;
};

inline std::ostream& operator<<(std::ostream& output, SequenceNumberSet_t& sns){
	return output<< sns.print();
}


}
}
}



#endif /* RPTS_ELEM_SEQNUM_H_ */
