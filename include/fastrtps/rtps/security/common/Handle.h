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
#ifndef _RTPS_SECURITY_COMMON_HANDLE_H_
#define _RTPS_SECURITY_COMMON_HANDLE_H_

#include <memory>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {


class Handle
{
    public:

        const std::string& get_class_id() const
        {
            return class_id_;
        }

    protected:

        Handle(const std::string& class_id) : class_id_(class_id) {};

    private:

        std::string class_id_;
};

template<typename T>
class HandleImpl : public Handle
{
    public:

        typedef T type;

        HandleImpl() : Handle(T::class_id_), _impl(new T) {}

        static HandleImpl<T>& narrow(Handle& handle)
        {
            if(handle.get_class_id().compare(T::class_id_) == 0)
                return reinterpret_cast<HandleImpl<T>&>(handle);

            return HandleImpl<T>::nil_handle;
        }

        bool nil() const
        {
            return _impl ? false : true;
        }

        T* operator->()
        {
            return _impl.get();
        }

    private:

        explicit HandleImpl(bool) : Handle(T::class_id_) {}

        std::unique_ptr<T> _impl;

        static HandleImpl<T> nil_handle;
};
template<typename T>
HandleImpl<T> HandleImpl<T>::nil_handle(true);


// Define common handlers
template<typename T>
using IdentityHandle = Handle<T>;

template<typename T>
using PermissionsHandle = Handle<T>;

template<typename T>
using SharedSecretHandle = Handle<T>;

} //namespace eprosima
} //namespace fastrtps
} //namespace rtps
} //namespace security

#endif // _RTPS_SECURITY_COMMON_HANDLE_H_
