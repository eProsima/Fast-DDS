// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*!
 * @file SharedSecretHandle.h (test mock)
 *
 * Drop-in replacement for the production SharedSecretHandle.h. The only
 * difference is the SharedSecretHandle typedef: it instantiates HandleImpl
 * with MockAuthenticationPlugin instead of PKIDH, so it matches the dynamic
 * type of the SharedSecret-bearing handles created by the test (see
 * test/mock/rtps/SecurityPluginFactory/rtps/security/MockAuthenticationPlugin.h).
 *
 * Without this substitution the production translation unit narrows to a
 * different HandleImpl<SharedSecret, *> instantiation than the test creates;
 * the two share layout but have distinct vtables, so the next virtual call
 * (e.g. nil()) trips ubsan with a vptr/type mismatch.
 */
#ifndef _FASTDDS_RTPS_SECURITY_COMMON_SHAREDSECRETHANDLE_H_
#define _FASTDDS_RTPS_SECURITY_COMMON_SHAREDSECRETHANDLE_H_

#include <rtps/security/common/Handle.h>

#include <cstdint>
#include <vector>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class SharedSecret
{
public:

    class BinaryData
    {
    public:

        BinaryData()
        {
        }

        BinaryData(
                const BinaryData& data)
            : name_(data.name_)
            , value_(data.value_)
        {
        }

        BinaryData(
                BinaryData&& data)
            : name_(std::move(data.name_))
            , value_(std::move(data.value_))
        {
        }

        BinaryData(
                const std::string& name,
                const std::vector<uint8_t>& value)
            : name_(name)
            , value_(value)
        {
        }

        BinaryData(
                std::string&& name,
                std::vector<uint8_t>&& value)
            : name_(std::move(name))
            , value_(std::move(value))
        {
        }

        BinaryData& operator =(
                const BinaryData& data)
        {
            name_ = data.name_;
            value_ = data.value_;
            return *this;
        }

        BinaryData& operator =(
                BinaryData&& data)
        {
            name_ = std::move(data.name_);
            value_ = std::move(data.value_);
            return *this;
        }

        void name(
                const std::string& name)
        {
            name_ = name;
        }

        void name(
                std::string&& name)
        {
            name_ = std::move(name);
        }

        const std::string& name() const
        {
            return name_;
        }

        std::string& name()
        {
            return name_;
        }

        void value(
                const std::vector<uint8_t>& value)
        {
            value_ = value;
        }

        void value(
                std::vector<uint8_t>&& value)
        {
            value_ = std::move(value);
        }

        const std::vector<uint8_t>& value() const
        {
            return value_;
        }

        std::vector<uint8_t>& value()
        {
            return value_;
        }

    private:

        std::string name_;

        std::vector<uint8_t> value_;
    };

    static const char* const class_id_;

    std::vector<BinaryData> data_;
};

// Forward declaration is enough. The friend declaration inside HandleImpl
// only requires the elaborated name.
class MockAuthenticationPlugin;

typedef HandleImpl<SharedSecret, MockAuthenticationPlugin> SharedSecretHandle;

class SharedSecretHelper
{
public:

    static const std::vector<uint8_t>* find_data_value(
            const SecretHandle& sharedsecret,
            const std::string& name);
};

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // _FASTDDS_RTPS_SECURITY_COMMON_SHAREDSECRETHANDLE_H_
