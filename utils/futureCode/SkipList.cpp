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
 * @file SkipList.cpp
 *
 */

#include "SkipList.h"

#include <cstdlib>
#include <ctime>

namespace eprosima {
namespace fastrtps{
namespace rtps {


template<class Obj>
SkipNode<Obj>::~SkipNode()
{
	delete [] mp_fwdNodes;
}

template<class Obj>
SkipNode<Obj>::SkipNode(Obj* obj,uint32_t level):m_height(level),
mp_data(obj)
{
	mp_fwdNodes = new SkipNode<Obj>* [m_height];
	for(int x = 0;x<m_height;++x)
		mp_fwdNodes[x] = nullptr;
}

template <class Obj>
SkipNode<Obj>::SkipNode(uint32_t level):m_height(level),
mp_data(nullptr)
{
	mp_fwdNodes = new SkipNode<Obj>* [m_height];
	for(int x = 0;x<m_height;++x)
		mp_fwdNodes[x] = nullptr;
}

//SKIP LIST

template <class Obj>
SkipList<Obj>::SkipList(float probability,uint32_t maxHeight): m_maxHeight(maxHeight),
m_currHeight(0),
m_probability(probability),
mp_head(nullptr),
mp_tails(nullptr)
{
	srand (time(NULL));
	mp_head = new SkipNode<Obj>(m_maxHeight);
	mp_tails = new SkipNode<Obj>* [m_maxHeight];
	for ( int x = 0; x < m_maxHeight; x++ )
	{
		mp_head->mp_fwdNodes[x] = nullptr;
		mp_tails[x] = mp_head;
	}

}

template <class Obj>
SkipList<Obj>::~SkipList() {
	// Walk 0 level nodes and delete all
	SkipNode<Obj>* tmp;
	SkipNode<Obj>* nxt;
	tmp = mp_head;
	while ( tmp )
	{
		nxt = tmp->mp_fwdNodes[0];
		delete tmp;
		tmp = nxt;
	}
	delete [] mp_tails;
}

template <class Obj>
bool SkipList<Obj>::insert_at_end(Obj* obj)
{
	SkipNode<Obj>* tmp = new SkipNode<Obj>(obj,newLevel());
	for(int x = 0;x<tmp->m_height;++x)
	{
		mp_tails[x]->mp_fwdNodes[x] = tmp;
		mp_tails[x] = tmp;
	}
	return true;
}

template <class Obj>
bool SkipList<Obj>::insert(Obj* obj)
{
	int lvl = 0, h = 0;
	SkipNode<Obj>** updateVec =	new SkipNode<Obj>* [m_maxHeight];
	SkipNode<Obj>* tmp = mp_head;
	// Figure out where new node goes
	for ( h = m_currHeight; h >= 0; h-- )
	{
		while (tmp->mp_fwdNodes[h] != nullptr && *tmp->mp_fwdNodes[h]->mp_data < *obj )
		{
			tmp = tmp->fwdNodes[h];
		}
		updateVec[h] = tmp;
	}
	tmp = tmp->fwdNodes[0]; //Element just before the one we are inserting.
	// If dup, return false
	if ( *tmp->mp_data == *obj )
	{
		return false;
	}
	else
	{
		// Perform an insert
		lvl = newLevel();
		if ( lvl > m_currHeight )
		{
			for ( int i = m_currHeight; i <= lvl; i++ )
				updateVec[i] = mp_head;
			m_currHeight = lvl;
		}
		// Insert new element
		tmp = new SkipNode<Obj>(obj, lvl);
		for ( int i = 0; i < lvl; i++ )
		{
			tmp->fwdNodes[i] = updateVec[i]->fwdNodes[i];
			updateVec[i]->fwdNodes[i] = tmp;
		}
	}
	return true;
}


template <class Obj>
bool SkipList<Obj>::remove(Obj* obj)
{

}



template <class Obj>
uint32_t SkipList<Obj>::newLevel()
{
	uint32_t tmpLvl = 1;
	// Develop a random number between 1 and maxLvl (node height).
	while ((rand01() < m_probability) &&
			(tmpLvl < m_maxHeight))
		tmpLvl++;

	return tmpLvl;
}

template <class Obj>
float SkipList<Obj>::rand01()
{
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

}
} /* namespace rtps */
} /* namespace eprosima */
