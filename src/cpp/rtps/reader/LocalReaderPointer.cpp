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

#include <rtps/reader/LocalReaderPointer.hpp>

#include <rtps/reader/LocalReaderView.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {


LocalReaderPointer::LocalReaderPointer()
    : local_reader_(nullptr)
    , already_referenced_(false)
{
}

LocalReaderPointer::LocalReaderPointer(
        BaseReader* reader,
        std::shared_ptr<LocalReaderView> view)
    : local_reader_(reader)
    , view_(view)
    , already_referenced_(false)
{

}

LocalReaderPointer::~LocalReaderPointer()
{
    auto view = view_.lock();
    if (already_referenced_ && view)
    {
        view->dereference();
    }
}

BaseReader* LocalReaderPointer::reader()
{
    BaseReader* ret = nullptr;
    auto view = view_.lock();
    if (!already_referenced_ && view)
    {
        view->add_reference();
        ret = local_reader_;
        already_referenced_ = true;
    }

    return ret;
}

BaseReader* LocalReaderPointer::operator ->()
{
    return reader();
}

bool LocalReaderPointer::is_valid()
{
    bool ret = false;

    if (local_reader_)
    {
        auto view = view_.lock();

        if (view && view->get_status() != LocalReaderViewStatus::INACTIVE)
        {
            ret = true;
        }
    }

    return ret;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
