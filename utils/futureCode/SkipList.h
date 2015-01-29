/*************************************************************************
 * Copyright (c) 2015 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SkipList.h
 *
 */

#ifndef SKIPLIST_H_
#define SKIPLIST_H_

namespace eprosima {
namespace fastrtps{
namespace rtps {

template <class Obj>
class SkipNode
{
public:
	SkipNode(Obj* obj,uint32_t level);
	SkipNode(uint32_t level);
	~SkipNode();
	uint32_t m_height;
	Obj* mp_data;
	SkipNode** mp_fwdNodes;

};


template <class Obj>
class SkipList {
public:
	SkipList(float probability, uint32_t maxHeight);
	virtual ~SkipList();

	bool insert_at_end(Obj* obj);
	bool remove(Obj* obj);
	bool insert(Obj* obj);


private:
	uint32_t newLevel();
	float rand01();
	uint32_t m_maxHeight;
	uint32_t m_currHeight;
	float m_probability;
	SkipNode<Obj>* mp_head;
	SkipNode<Obj>** mp_tails;

};

}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* SKIPLIST_H_ */
