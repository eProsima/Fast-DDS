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
 * @file ReaderLocator.h
*/



#ifndef READERLOCATOR_H_
#define READERLOCATOR_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <vector>
#include "../common/Locator.h"
#include "../common/Guid.h"
#include "../common/SequenceNumber.h"
#include "../messages/RTPSMessageGroup.h"


namespace eprosima {
namespace fastrtps{
namespace rtps {

struct CacheChange_t;

/**
 * Class ReaderLocator, contains information about a locator, without saving its state.
  * @ingroup WRITER_MODULE
 */
class ReaderLocator
{
public:
    ReaderLocator();
    /**
     * Constructor.
     * @param locator Locator to create the ReaderLocator.
     * @param expectsInlineQos Indicate that the ReaderLocator expects inline QOS.
     */
    ReaderLocator(Locator_t& locator, bool expectsInlineQos);
    virtual ~ReaderLocator();
    //!Address of this ReaderLocator.
    Locator_t locator;
    //!Whether the Reader expects inlineQos with its data messages.
    bool expectsInlineQos;

    std::vector<ChangeForReader_t> unsent_changes;

    //!Number of times this locator has been used (in case different readers use the same locator).
    uint32_t n_used;

    /**
     * Remove change from requested list.
     * @param cpoin Pointer to change.
     * @return True if correct
     */
    //bool remove_requested_change(const CacheChange_t* cpoin);
};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif
#endif /* READERLOCATOR_H_ */
