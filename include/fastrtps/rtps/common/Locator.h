/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Locator.h	
 */

#ifndef RTPS_ELEM_LOCATOR_H_
#define RTPS_ELEM_LOCATOR_H_
#include "../../fastrtps_dll.h"
#include "Types.h"
#include <sstream>
#include <vector>
#include <cstdint>
namespace eprosima{
namespace fastrtps{
namespace rtps{

#define LOCATOR_INVALID(loc)  {loc.kind=LOCATOR_KIND_INVALID;loc.port= LOCATOR_PORT_INVALID;LOCATOR_ADDRESS_INVALID(loc.address);}
#define LOCATOR_KIND_INVALID -1

#define LOCATOR_ADDRESS_INVALID(a) {for(uint8_t i=0;i<16;i++) a[i]=0x0;}
#define LOCATOR_PORT_INVALID 0
#define LOCATOR_KIND_RESERVED 0
#define LOCATOR_KIND_UDPv4 1
#define LOCATOR_KIND_UDPv6 2


//!@brief Class Locator_t, uniquely identifies a communication channel for a particular transport. 
//For example, an address+port combination in the case of UDP.
//!@ingroup COMMON_MODULE
class RTPS_DllAPI Locator_t{
public:

    /*!
     * @brief Specifies the locator type. Valid values are:
     * LOCATOR_KIND_UDPv4
     * LOCATOR_KIND_UDPv6
     */
	int32_t kind;
	uint32_t port;
	octet address[16];

	//!Default constructor
	Locator_t():kind(1),port(0)
    {
		LOCATOR_ADDRESS_INVALID(address);
	}

	Locator_t(Locator_t&& loc):
		kind(loc.kind),
		port(loc.port)
	{
		for(uint8_t i = 0;i<16;++i)
			address[i] = loc.address[i];
	}

	Locator_t(const Locator_t& loc) :
		kind(loc.kind),
		port(loc.port)
	{
		for(uint8_t i = 0;i<16;++i)
			address[i] = loc.address[i];
	}

	Locator_t(uint32_t portin):kind(1),port(portin)
	{
		LOCATOR_ADDRESS_INVALID(address);
	}
    
	Locator_t& operator=(const Locator_t& loc)
	{
		kind = loc.kind;
		port = loc.port;
		for(uint8_t i=0;i<16;i++)
			address[i] = loc.address[i];
		return *this;
	}

	bool set_IP4_address(octet o1,octet o2,octet o3,octet o4){
		LOCATOR_ADDRESS_INVALID(address);
		address[12] = o1;
		address[13] = o2;
		address[14] = o3;
		address[15] = o4;
		return true;
	}
	bool set_IP4_address(std::string& in_address)
	{
		std::stringstream ss(in_address);
		int a,b,c,d; //to store the 4 ints
		char ch; //to temporarily store the '.'
		ss >> a >> ch >> b >> ch >> c >> ch >> d;
		LOCATOR_ADDRESS_INVALID(address);
		address[12] = (octet)a;
		address[13] = (octet)b;
		address[14] = (octet)c;
		address[15] = (octet)d;
		return true;
	}
	std::string to_IP4_string(){
		std::stringstream ss;
		ss << (int)address[12] << "." << (int)address[13] << "." << (int)address[14]<< "." << (int)address[15];
		return ss.str();
	}
	uint32_t to_IP4_long()
	{
		uint32_t addr;
		octet* oaddr = (octet*)&addr;
#if __BIG_ENDIAN__
        oaddr[0] = address[12];oaddr[1] = address[13];
        oaddr[2] = address[14];oaddr[3] = address[15];
#else
		oaddr[0] = address[15];oaddr[1] = address[14];
		oaddr[2] = address[13];oaddr[3] = address[12];
#endif

		return addr;
	}
};


inline bool IsAddressDefined(const Locator_t& loc)
{
	if(loc.kind == LOCATOR_KIND_UDPv4)
	{
		for(uint8_t i = 12; i < 16; ++i)
		{
			if(loc.address[i] != 0)
				return true;
		}
	}
	else if (loc.kind == LOCATOR_KIND_UDPv6)
	{
		for(uint8_t i = 0; i < 16; ++i)
		{
			if(loc.address[i] != 0)
				return true;
		}
	}
	return false;
}

inline bool IsLocatorValid(const Locator_t&loc)
{
	if(loc.kind<0)
		return false;
	if(loc.port == 0)
		return false;
	return true;

}

inline bool operator==(const Locator_t&loc1,const Locator_t& loc2){
	if(loc1.kind!=loc2.kind)
		return false;
	if(loc1.port !=loc2.port)
		return false;
	for(uint8_t i =0;i<16;i++){
		if(loc1.address[i] !=loc2.address[i])
			return false;
	}
	return true;
}

inline std::ostream& operator<<(std::ostream& output,const Locator_t& loc)
{
	if(loc.kind == LOCATOR_KIND_UDPv4)
	{
		output<<(int)loc.address[12] << "." << (int)loc.address[13] << "." << (int)loc.address[14]<< "." << (int)loc.address[15]<<":"<<loc.port;
	}
	else if(loc.kind == LOCATOR_KIND_UDPv6)
	{
		for(uint8_t i =0;i<16;++i)
		{
			output<<(int)loc.address[i];
			if(i<15)
				output<<".";
		}
		output<<":"<<loc.port;
	}
	return output;
}



typedef std::vector<Locator_t>::iterator LocatorListIterator;


/**
 * Class LocatorList_t, a Locator_t vector that doesn't avoid duplicates.
 * @ingroup COMMON_MODULE
 */
class LocatorList_t{
public:
	RTPS_DllAPI LocatorList_t(){};
	RTPS_DllAPI ~LocatorList_t(){};
	RTPS_DllAPI LocatorList_t(const LocatorList_t& list)
	{
		m_locators = list.m_locators;
	}
	RTPS_DllAPI LocatorListIterator begin(){
		return m_locators.begin();
	}
	RTPS_DllAPI LocatorListIterator end(){
		return m_locators.end();
	}
	RTPS_DllAPI size_t size(){
		return m_locators.size();
	}
	RTPS_DllAPI void clear(){ return m_locators.clear();}
	RTPS_DllAPI void reserve(size_t num){ return m_locators.reserve(num);}
	RTPS_DllAPI void resize(size_t num) { return m_locators.resize(num);}
	RTPS_DllAPI void push_back(const Locator_t& loc)
	{
		bool already = false;
		for(LocatorListIterator it=this->begin(); it!=this->end(); ++it)
		{
			if(loc == *it)
			{
				already = true;
				break;
			}
		}
		if(!already)
			m_locators.push_back(loc);
	}

	RTPS_DllAPI void push_back(const LocatorList_t& locList)
	{
		for(auto it = locList.m_locators.begin(); it!=locList.m_locators.end(); ++it)
        {
			this->push_back(*it);
        }
	}

	RTPS_DllAPI bool empty(){
		return m_locators.empty();
	}

	RTPS_DllAPI bool contains(const Locator_t& loc)
	{
		for(LocatorListIterator it=this->begin();it!=this->end();++it)
		{
            if(IsAddressDefined(*it))
            {
                if(loc == *it)
                    return true;
            }
            else
            {
                if(loc.kind == (*it).kind && loc.port == (*it).port)
                    return true;
            }
		}

		return false;
	}

	RTPS_DllAPI bool isValid()
	{
		for(LocatorListIterator it=this->begin();it!=this->end();++it)
		{
			if(!IsLocatorValid(*it))
				return false;
		}
		return true;
	}
	friend std::ostream& operator <<(std::ostream& output,const LocatorList_t& loc);
private:
	std::vector<Locator_t> m_locators;

};

inline std::ostream& operator<<(std::ostream& output,const LocatorList_t& locList)
{
	for(auto it = locList.m_locators.begin();it!=locList.m_locators.end();++it)
	{
		output << *it << ",";
	}
	return output;
}


}
}
}




#endif /* RTPS_ELEM_LOCATOR_H_ */
