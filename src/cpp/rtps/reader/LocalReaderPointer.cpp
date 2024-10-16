// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file LocalReaderPointer.cpp
 */

#include <rtps/reader/BaseReader.hpp>
#include <rtps/reader/LocalReaderPointer.hpp>

#include <cassert>

#include <rtps/reader/LocalReaderView.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

WeakLocalReaderPointer::WeakLocalReaderPointer()
    : local_reader_(nullptr)
    , view_(nullptr)
{
}

WeakLocalReaderPointer::WeakLocalReaderPointer(
        BaseReader* reader,
        std::shared_ptr<LocalReaderView> view)
{
    std::lock_guard<std::mutex> lock(this->mutex_);
    local_reader_ = reader;
    view_ = view;
}

WeakLocalReaderPointer::WeakLocalReaderPointer(
        const WeakLocalReaderPointer& other)
{
    std::lock_guard<std::mutex> lock_other(other.mutex_);
    std::lock_guard<std::mutex> lock(this->mutex_);
    local_reader_ = other.local_reader_;
    view_ = other.view_;
}

BaseReader* WeakLocalReaderPointer::operator ->()
{
    assert(nullptr != local_reader_);
    return local_reader_;
}

WeakLocalReaderPointer::operator bool() const
{
    bool ret = false;

    std::lock_guard<std::mutex> lock(mutex_);
    if (nullptr != local_reader_ &&
            nullptr != view_)
    {
        ret = true;
    }

    return ret;
}

WeakLocalReaderPointer& WeakLocalReaderPointer::operator =(
        const WeakLocalReaderPointer& other)
{
    std::lock_guard<std::mutex> lock_other(other.mutex_);
    std::lock_guard<std::mutex> lock(this->mutex_);
    local_reader_ = other.local_reader_;
    view_ = other.view_;
    return *this;
}

void WeakLocalReaderPointer::reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    local_reader_ = nullptr;
    view_.reset();
}

LocalReaderPointer::LocalReaderPointer()
    : WeakLocalReaderPointer()
{
}

LocalReaderPointer::LocalReaderPointer(
        BaseReader* reader,
        std::shared_ptr<LocalReaderView> view)
    : WeakLocalReaderPointer(reader, view)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (nullptr != view_)
    {
        view_->add_reference();
    }
}

LocalReaderPointer::LocalReaderPointer(
        const LocalReaderPointer& other)
    : WeakLocalReaderPointer()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::lock_guard<std::mutex> lock_other(other.mutex_);

    local_reader_ = other.local_reader_;
    view_ = other.view_;

    if (nullptr != view_)
    {
        view_->add_reference();
    }
}

LocalReaderPointer::LocalReaderPointer(
        const WeakLocalReaderPointer& weak_local_reader_ptr)
    : WeakLocalReaderPointer()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::lock_guard<std::mutex> lock_other(weak_local_reader_ptr.mutex_);

    local_reader_ = weak_local_reader_ptr.local_reader_;
    view_ = weak_local_reader_ptr.view_;

    if (nullptr != view_)
    {
        view_->add_reference();
    }
}

LocalReaderPointer::~LocalReaderPointer()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (nullptr != view_)
    {
        view_->dereference();
    }
}

BaseReader* LocalReaderPointer::operator ->()
{
    assert(nullptr != local_reader_);
    return local_reader_;
}

LocalReaderPointer& LocalReaderPointer::operator =(
        const LocalReaderPointer& other)
{
    std::lock_guard<std::mutex> lock_other(other.mutex_);
    std::lock_guard<std::mutex> lock(this->mutex_);
    local_reader_ = other.local_reader_;
    view_ = other.view_;

    if (nullptr != view_)
    {
        view_->add_reference();
    }

    return *this;
}

LocalReaderPointer& LocalReaderPointer::operator =(
        const WeakLocalReaderPointer& other)
{
    std::lock_guard<std::mutex> lock_other(other.mutex_);
    std::lock_guard<std::mutex> lock(this->mutex_);

    local_reader_ = other.local_reader_;
    view_ = other.view_;

    if (nullptr != view_)
    {
        view_->add_reference();
    }

    return *this;
}

LocalReaderPointer::operator bool() const
{
    bool ret = false;

    std::lock_guard<std::mutex> lock(mutex_);

    if (nullptr != local_reader_ &&
            nullptr != view_ &&
            view_->get_status() != LocalReaderViewStatus::INACTIVE)
    {
        ret = true;
    }

    return ret;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
