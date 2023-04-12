// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef MEMBER_ID_H
#define MEMBER_ID_H

#include <cstdint>
#include <istream>
#include <iterator>
#include <limits>
#include <ostream>
#include <string>
#include <type_traits>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

template<typename base>
class member_id
{
    // value_ is invalid if is higher than
    static const base invalid = std::numeric_limits<base>::max();

    std::size_t value_ = invalid;

public:

    using integer_type = std::size_t;
    static const unsigned int serialized_size = sizeof(base);

    // trivial construction
    member_id() = default;

    member_id(const member_id& i) = default;

    explicit member_id(unsigned long long i)
        : value_(static_cast<integer_type>(i))
    {}

    // support for signed construction
    template<typename type,
             typename std::enable_if<std::is_integral<type>::value &&
                                     std::is_signed<type>::value, bool>::type = 0>
    explicit member_id(type i)
        : value_(i < 0 ? 0 : i)
    {}

    // interaction with integers, note invalid values propagate
    member_id& operator=(const integer_type& i)
    {
        value_ = i;
        return *this;
    }

    member_id& operator+=(const integer_type& i)
    {
        value_ += i;
        return *this;
    }

    member_id& operator-=(const integer_type& i)
    {
        if(value_ < invalid)
        {   // only decrement valid values
            value_ -= i;
        }
        return *this;
    }

    // trivial assignation
    member_id& operator=(const member_id& i) = default;

    member_id& operator+=(const member_id& i)
    {
        return this->operator+=(i.value_);
    }

    member_id& operator-=(const member_id& i)
    {
        return this->operator-=(i.value_);
    }

    // aritmethic
    member_id operator+(const member_id& i) const
    {
        member_id t(*this);
        t += i;
        return t;
    }

    member_id operator-(const member_id& i) const
    {
        member_id t(*this);
        t -= i;
        return t;
    }

    // integer aritmethic
    member_id operator+(const integer_type& i) const
    {
        member_id t(*this);
        t += i;
        return t;
    }

    member_id operator-(const integer_type& i) const
    {
        member_id t(*this);
        t -= i;
        return t;
    }

    // comparison with integers
    bool operator==(const integer_type& i) const
    {
        bool a_invalid = value_ >= invalid;
        bool b_invalid = i >= invalid;

        return a_invalid || b_invalid ? a_invalid && b_invalid : value_ == i;
    }

    bool operator!=(const integer_type& i) const
    {
        return !this->operator==(i);
    }

    bool operator<(const integer_type& i) const
    {
        return !(value_ >= i);
    }

    bool operator>(const integer_type& i) const
    {
        return !(value_ <= i);
    }

    bool operator<=(const integer_type& i) const
    {
        return *this == i ? true : value_ < i;
    }

    bool operator>=(const integer_type& i) const
    {
        return *this == i ? true : value_ > i;
    }

    // comparison
    bool operator==(const member_id& i) const
    {
        return this->operator==(i.value_);
    }

    bool operator!=(const member_id& i) const
    {
        return !this->operator==(i);
    }

    bool operator<(const member_id& i) const
    {
        return this->operator<(i.value_);
    }

    bool operator>(const member_id& i) const
    {
        return this->operator>(i.value_);
    }

    bool operator<=(const member_id& i) const
    {
        return this->operator<=(i.value_);
    }

    bool operator>=(const member_id& i) const
    {
        return this->operator>=(i.value_);
    }

    // prefix increment
    member_id& operator++()
    {
        ++value_;
        return *this;
    }

    // postfix increment
    member_id operator++(int)
    {
        member_id old = *this;
        operator++();
        return old;
    }

    // prefix decrement
    member_id& operator--()
    {
        if(value_ < invalid)
        {   // only decrement valid values
            --value_;
        }
        return *this;
    }

    // postfix decrement
    member_id operator--(int)
    {
        member_id old = *this;
        operator--();
        return old;
    }

    // return integer value
    base operator*() const
    {
        return !*this ? invalid : static_cast<base>(value_);
    }

    // if() support
    bool operator!() const
    {
        return value_ >= invalid;
    }

    // CDR input/output support (avoid dependency using templates)

    template<class Cdr>
    Cdr& serialize(Cdr& buf) const
    {
        return buf << **this;
    }

    template<class Cdr>
    Cdr& deserialize(Cdr& buf)
    {
        base tmp;
        buf >> tmp;
        value_ = tmp;
        return buf;
    }
};

// addition
template<class base>
member_id<base> operator+(const member_id<base>& x, const member_id<base>& y)
{
    member_id<base> mx(x);
    return mx += y;
}

template<class base>
member_id<base> operator+(const typename member_id<base>::integer_type& x, const member_id<base>& y)
{
    member_id<base> mx(x);
    return mx += y;
}

// substraction
template<class base>
member_id<base> operator-(const member_id<base>& x, const member_id<base>& y)
{
    member_id<base> mx(x);
    return mx -= y;
}

template<class base>
member_id<base> operator-(const typename member_id<base>::integer_type& x, const member_id<base>& y)
{
    member_id<base> mx(x);
    return mx -= y;
}

// comparison

template<class base>
bool operator==(const typename member_id<base>::integer_type& x, const member_id<base>& y)
{
    return y == x;
}

template<class base>
bool operator!=(const typename member_id<base>::integer_type& x, const member_id<base>& y)
{
    return y != x;
}

template<class base>
bool operator<(const typename member_id<base>::integer_type& x, const member_id<base>& y)
{
    return y >= x;
}

template<class base>
bool operator>(const typename member_id<base>::integer_type& x, const member_id<base>& y)
{
    return y <= x;
}

template<class base>
bool operator<=(const typename member_id<base>::integer_type& x, const member_id<base>& y)
{
    return y > x;
}

template<class base>
bool operator>=(const typename member_id<base>::integer_type& x, const member_id<base>& y)
{
    return y < x;
}

// STL input/output support

namespace detail {

// ancillary strings for STL input/output
template <class CharT, class Traits>
struct member_id_invalid
{
    static const std::basic_string<CharT, Traits> get();
};

template<>
inline const std::string member_id_invalid<std::string::value_type, std::string::traits_type>::get()
{
    return "MEMBER_ID_INVALID";
}

template<>
inline const std::wstring member_id_invalid<std::wstring::value_type, std::wstring::traits_type>::get()
{
    return L"MEMBER_ID_INVALID";
}

} // detail namespace

template <class base, class CharT, class Traits>
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& istr, member_id<base>& x)
{
    base tmp;
    if (istr >> tmp)
    {
        x = tmp;
    }
    else // check if is MEMBER_ID_INVALID
    {
        istr.clear();
        const auto & val = detail::member_id_invalid<CharT, Traits>::get();
        auto res = std::mismatch(val.begin(), val.end(), std::istream_iterator<CharT, CharT, Traits>(istr));
        if (val.end() == res.first)
        {
            x = member_id<base>{}; // invalid
            istr.unget();
        }
        // on error undefined
    }
    return istr;
}

template <class base, class CharT, class Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& ostr, const member_id<base>& x)
{
    if(!x)
    {
        return ostr << detail::member_id_invalid<CharT, Traits>::get();
    }
    return ostr << *x;
}

// specialization to use
using MemberId = member_id<uint32_t>;

const MemberId MEMBER_ID_INVALID;

// literals
inline namespace literals
{

inline MemberId operator"" _id(unsigned long long x)
{
    return MemberId(x);
}

} // namespace literals

} // namespace v1_3

// Make the literals available on types
using namespace v1_3::literals;

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // MEMBER_ID_H
