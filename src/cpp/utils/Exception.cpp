/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * FASTCDR_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#include "eprosimartps/utils/Exception.h"

namespace eprosima
{
	Exception::Exception(const std::string &message) : m_message(message)
	{
	}

//	Exception::Exception(std::string&& message) : m_message(std::move(message))
//	{
//	}

	Exception::Exception(const Exception &ex) : m_message(ex.m_message)
	{
	}

//	Exception::Exception(Exception&& ex) : m_message(std::move(ex.m_message))
//	{
//	}

	Exception& Exception::operator=(const Exception &ex)
	{
		m_message = ex.m_message;
		return *this;
	}

//	Exception& Exception::operator=(Exception&&)
//	{
//		m_message = std::move(m_message);
//		return *this;
//	}

	Exception::~Exception() throw()
	{
	}

	const char* Exception::what() const throw()
	{
		return m_message.c_str();
	}
} // namespace eprosima
