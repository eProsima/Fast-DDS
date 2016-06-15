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
