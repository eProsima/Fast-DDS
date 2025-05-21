// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file RTPSEntityDeleter.hpp
 */

#ifndef UTILS__RTPSENTITYDELETERHPP
#define UTILS__RTPSENTITYDELETERHPP

#include <atomic>
#include <cstdint>

namespace eprosima {
namespace fastdds {

/**
 *
*/
template<typename T>
class RTPSEntityDeleter
{
public:
   explicit RTPSEntityDeleter()
   : ptr_(nullptr)
   {
       // Default constructor
   }
   explicit RTPSEntityDeleter(
          T* ptr)
      : ptr_(ptr)
      {
          assert(ptr != nullptr);
      }

   RTPSEntityDeleter(const RTPSEntityDeleter& other)
   : ptr_(other.ptr_)
   {
       other.ptr_ = nullptr;
   }

   RTPSEntityDeleter(RTPSEntityDeleter&& other)
   : ptr_(other.ptr_)
   {
       other.ptr_ = nullptr;
   }

   ~RTPSEntityDeleter()
   {
      if (ptr_ != nullptr)
      {
          ptr_->local_actions_on_entity_removed();
          delete ptr_;
          ptr_ = nullptr;
      }
   }

   /**
    * @brief operator to check if the pointer is valid.
    */
   operator bool() const
   {
       return nullptr != ptr_;
   }

   /**
    * @brief operator to call the T methods.
    */
   T& operator =(const RTPSEntityDeleter& other) const
   {
       ptr_ = other.ptr_;
       other.ptr_ = nullptr;
       return *ptr_;
   }

   bool operator==(
           const RTPSEntityDeleter& other) const
   {
       return ptr_ == other.ptr_;
   }

   /**
    * @brief operator to call the T methods.
    */
   T* operator ->() const
   {
       assert(nullptr != ptr_);
       return ptr_;
   }

   /**
    * @brief operator to call the T methods.
    */
   T& operator *() const
   {
       assert(nullptr != ptr_);
       return *ptr_;
   }

   template <class Q>
   operator Q*() const { return static_cast<Q*>(this->ptr_); }

   /**
    * Pointer to the managed object.
    */
   mutable T* ptr_;
};
}  // namespace fastdds
}  // namespace eprosima
#endif // UTILS__RTPSENTITYDELETERHPP
