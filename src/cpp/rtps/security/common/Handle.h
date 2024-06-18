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

/*!
 * @file Handle.h
 */
#ifndef _FASTDDS_RTPS_SECURITY_COMMON_HANDLE_H_
#define _FASTDDS_RTPS_SECURITY_COMMON_HANDLE_H_

#include <memory>
#include <string>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class Handle : public std::enable_shared_from_this<Handle>
{
public:

    const std::string& get_class_id() const
    {
        return class_id_;
    }

    virtual bool nil() const = 0;

protected:

    Handle(
            const std::string& class_id)
        : class_id_(class_id)
    {
    }

    virtual ~Handle()
    {
    }

private:

    std::string class_id_;
};

template<typename T, typename F>
class HandleImpl : public Handle
{
    friend F;

protected:

    HandleImpl()
        : Handle(T::class_id_)
        , impl_(new T)
    {
    }

    virtual ~HandleImpl() = default;

public:

    typedef T type;

    static HandleImpl<T, F>& narrow(
            Handle& handle)
    {
        if (handle.get_class_id().compare(T::class_id_) == 0)
        {
            return reinterpret_cast<HandleImpl<T, F>&>(handle);
        }

        return HandleImpl<T, F>::nil_handle;
    }

    static const HandleImpl<T, F>& narrow(
            const Handle& handle)
    {
        if (handle.get_class_id().compare(T::class_id_) == 0)
        {
            return reinterpret_cast<const HandleImpl<T, F>&>(handle);
        }

        return HandleImpl<T, F>::nil_handle;
    }

    bool nil() const override
    {
        return impl_ ? false : true;
    }

    T* operator *()
    {
        return impl_.get();
    }

    const T* operator *() const
    {
        return impl_.get();
    }

    T* operator ->()
    {
        return impl_.get();
    }

    const T* operator ->() const
    {
        return impl_.get();
    }

    static HandleImpl<T, F> nil_handle;

private:

    explicit HandleImpl(
            bool)
        : Handle(T::class_id_)
    {
    }

    std::unique_ptr<T> impl_;
};

template<typename T, typename F>
HandleImpl<T, F> HandleImpl<T, F>::nil_handle(true);

class NilHandle : public Handle
{
public:

    NilHandle()
        : Handle("nil_handle")
    {
    }

    virtual ~NilHandle() = default;

    bool nil() const override
    {
        return true;
    }

};


// Define common handlers
typedef Handle IdentityHandle;

typedef Handle PermissionsHandle;

typedef Handle SecretHandle;

typedef Handle ParticipantCryptoHandle;
typedef Handle EntityCryptoHandle;
typedef Handle DatawriterCryptoHandle;
typedef Handle DatareaderCryptoHandle;

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // _FASTDDS_RTPS_SECURITY_COMMON_HANDLE_H_
