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
 * @file ReceivedChanges.cpp
 *
 */

#include "ReceivedChanges.h"

namespace eprosima {
namespace fastrtps {
namespace rtps {

ReceivedChanges::ReceivedChanges() {
	// TODO Auto-generated constructor stub
m_missingChanges.reserve(1000);
}

ReceivedChanges::~ReceivedChanges() {
	// TODO Auto-generated destructor stub
}

bool ReceivedChanges::add(SequenceNumber_t& s)
{
	if(s > this->m_largestSequenceNumberReceived)
	{
		return insertMissingSequenceNumbers(s);
	}
	else if(s == this->m_largestSequenceNumberReceived)
	{
		return false;
	}
	else if(s < this->m_largestSequenceNumberReceived)
	{
		return missingSequenceNumberReceived(s);
	}
	return false;
}

bool ReceivedChanges::insertMissingSequenceNumbers(SequenceNumber_t& s)
{
	int i = 1;
	while(m_largestSequenceNumberReceived+i < s)
	{
		m_missingChanges.push_back(m_largestSequenceNumberReceived+i);
		++i;
	}
	m_largestSequenceNumberReceived = s;
	return true;
}

bool ReceivedChanges::missingSequenceNumberReceived(SequenceNumber_t& s)
{
	std::vector<SequenceNumber_t>::iterator it = m_missingChanges.end();
	while(it > m_missingChanges.begin())
	{
		--it;
		if(*it == s)
		{
			m_missingChanges.erase(it);
			return true;
		}
		if(*it< s)
			return false;
	}
	return false;
}

}
} /* namespace rtps */
} /* namespace eprosima */
