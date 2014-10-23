/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TimeConversion.h
 *
 */

#ifndef TIMECONVERSION_H_
#define TIMECONVERSION_H_

#include <cstdint>
#include "eprosimartps/common/types/Time_t.h"

namespace eprosima {
namespace rtps {

namespace TimeConv{
inline double Time_t2SecondsDouble(const Time_t& t)
{
	return (double)t.seconds + (double)(t.fraction/pow(2.0,32));
}
inline int64_t Time_t2MicroSecondsInt64(const Time_t& t)
{
	return (int64_t)(t.fraction/pow(2.0,32)*pow(10.0,6))+t.seconds*(int64_t)pow(10.0,6);
}
inline double Time_t2MicroSecondsDouble(const Time_t& t)
{
	return ((double)t.fraction/pow(2.0,32)*pow(10.0,6))+(double)t.seconds*pow(10.0,6);
}
inline int64_t Time_t2MilliSecondsInt64(const Time_t& t)
{
	return (int64_t)(t.fraction/pow(2.0,32)*pow(10.0,3))+t.seconds*(int64_t)pow(10.0,3);
}
inline double Time_t2MilliSecondsDouble(const Time_t& t)
{
	return ((double)t.fraction/pow(2.0,32)*pow(10.0,3))+(double)t.seconds*pow(10.0,3);
}
inline Time_t MilliSeconds2Time_t(double millisec)
{
	Time_t t;
	t.seconds = (int32_t)(millisec/pow(10.0,3));
	t.fraction = (uint32_t)((millisec-(double)t.seconds*pow(10.0,3))/pow(10.0,3)*pow(2.0,32));
	return t;
}
inline Time_t MicroSeconds2Time_t(double microsec)
{
	Time_t t;
	t.seconds = (int32_t)(microsec/pow(10.0,6));
	t.fraction = (uint32_t)((microsec-(double)t.seconds*pow(10.0,6))/pow(10.0,6)*pow(2.0,32));
	return t;
}
inline Time_t Seconds2Time_t(double seconds)
{
	Time_t t;
	t.seconds = (int32_t)seconds;
	t.fraction = (uint32_t)((seconds-(double)t.seconds)*pow(2.0,32));
	return t;
}

inline double Time_tAbsDiff2DoubleMillisec(const Time_t& t1,const Time_t& t2)
{
	double result = 0;
	result +=(double)abs((t2.seconds-t1.seconds)*1000);
	result +=(double)abs((t2.fraction-t1.fraction)/pow(2.0,32)*1000);
	return result;
}

//! Create a random Time_t that is millisec + [-randoff,randoff]
inline Time_t MilliSecondsWithRandOffset2Time_t(double millisec, double randoff)
{
	randoff = abs(randoff);
	millisec = millisec + (-randoff) + static_cast <double> (rand()) /( static_cast <double> (RAND_MAX/(2*randoff)));
	return MilliSeconds2Time_t(millisec);
}
//! Create a random Time_t that is microsec + [-randoff,randoff]
inline Time_t MicroSecondsWithRandOffset2Time_t(double microsec, double randoff)
{
	randoff = abs(randoff);
	microsec = microsec + (-randoff) + static_cast <double> (rand()) /( static_cast <double> (RAND_MAX/(2*randoff)));
	return MicroSeconds2Time_t(microsec);
}
//! Create a random Time_t that is sec + [-randoff,randoff]
inline Time_t SecondsWithRandOffset2Time_t(double sec, double randoff)
{
	randoff = abs(randoff);
	sec = sec + (-randoff) + static_cast <double> (rand()) /( static_cast <double> (RAND_MAX/(2*randoff)));
	return Seconds2Time_t(sec);
}



};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* TIMECONVERSION_H_ */
