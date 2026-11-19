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

#ifndef FASTDDS_TEST_MOCK__MOCKCOMPANION_HPP
#define FASTDDS_TEST_MOCK__MOCKCOMPANION_HPP

#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>

namespace eprosima {
namespace fastdds {
namespace mock {

/**
 * Trait mapping a real class to the gmock class holding its expectations.
 * A link-seam mock specializes it next to its gmock class, e.g.
 * `template<> struct MockOf<rtps::TimedEvent> { using type = TimedEventMock; };`
 */
template<class Real>
struct MockOf;

/**
 * Per-instance gmock companions of a real class whose implementation is replaced at link time.
 *
 * The mock implementation of the real class attaches a companion in its constructors, detaches it in its destructor
 * and forwards to it the methods tests need to control. Tests reach the companion of an instance with mock_of(), so
 * they can set expectations on objects created by the code under test. Detaching destroys the companion, which
 * verifies its expectations.
 */
template<class Real>
class MockCompanion
{
public:

    using Mock = typename MockOf<Real>::type;

    static void attach(
            const Real* real)
    {
        std::unique_ptr<Mock> mock(new Mock());
        {
            std::lock_guard<std::mutex> lock(mutex());
            mocks()[real].swap(mock);
        }
        // A companion replaced here is destroyed outside the lock, as in detach().
    }

    static void detach(
            const Real* real)
    {
        std::unique_ptr<Mock> mock;
        {
            std::lock_guard<std::mutex> lock(mutex());
            auto it = mocks().find(real);
            if (it != mocks().end())
            {
                mock = std::move(it->second);
                mocks().erase(it);
            }
        }
        // The companion is destroyed outside the lock, as verifying its expectations may report failures.
    }

    static Mock& get(
            const Real* real)
    {
        Mock* mock = find(real);
        if (nullptr == mock)
        {
            throw std::logic_error("No mock companion attached to this instance");
        }
        return *mock;
    }

    static Mock* find(
            const Real* real)
    {
        std::lock_guard<std::mutex> lock(mutex());
        auto it = mocks().find(real);
        return it == mocks().end() ? nullptr : it->second.get();
    }

private:

    // Never destroyed: instances owned by singletons are destroyed at exit, maybe after these would be.
    static std::map<const Real*, std::unique_ptr<Mock>>& mocks()
    {
        static auto* mocks = new std::map<const Real*, std::unique_ptr<Mock>>();
        return *mocks;
    }

    static std::mutex& mutex()
    {
        static auto* mutex = new std::mutex();
        return *mutex;
    }

};

/**
 * Get the gmock companion of an instance of a real class whose implementation is replaced at link time.
 *
 * @param real Instance created with the mock implementation.
 * @return The companion holding the expectations of @c real.
 * @throw std::logic_error if @c real has no companion attached.
 */
template<class Real>
typename MockOf<Real>::type& mock_of(
        const Real* real)
{
    return MockCompanion<Real>::get(real);
}

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK__MOCKCOMPANION_HPP
