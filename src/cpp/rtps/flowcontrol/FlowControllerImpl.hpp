#ifndef _RTPS_FLOWCONTROL_FLOWCONTROLLERIMPL_HPP_
#define _RTPS_FLOWCONTROL_FLOWCONTROLLERIMPL_HPP_

#include <atomic>
#include <cassert>
#include <chrono>
#include <map>
#include <unordered_map>

#include "FlowController.hpp"
#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/utils/TimedConditionVariable.hpp>
#include <fastdds/utils/TimedMutex.hpp>

#include <rtps/messages/RTPSMessageGroup.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/writer/BaseWriter.hpp>
#include <utils/thread.hpp>
#include <utils/threading.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/** Auxiliary classes **/

struct FlowQueue
{
    FlowQueue() noexcept = default;

    ~FlowQueue() noexcept
    {
        assert(new_interested_.is_empty());
        assert(old_interested_.is_empty());
    }

    FlowQueue(
            FlowQueue&& old) noexcept
    {
        swap(std::move(old));
    }

    FlowQueue& operator =(
            FlowQueue&& old) noexcept
    {
        swap(std::move(old));
        return *this;
    }

    void swap(
            FlowQueue&& old) noexcept
    {
        new_interested_.swap(old.new_interested_);
        old_interested_.swap(old.old_interested_);

        new_ones_.swap(old.new_ones_);
        old_ones_.swap(old.old_ones_);
    }

    bool is_empty() const noexcept
    {
        return new_ones_.is_empty() && old_ones_.is_empty();
    }

    void add_new_sample(
            CacheChange_t* change) noexcept
    {
        new_interested_.add_change(change);
    }

    void add_old_sample(
            CacheChange_t* change) noexcept
    {
        old_interested_.add_change(change);
    }

    CacheChange_t* get_next_change() noexcept
    {
        if (!is_empty())
        {
            return !new_ones_.is_empty() ?
                   new_ones_.head.writer_info.next : old_ones_.head.writer_info.next;
        }

        return nullptr;
    }

    void add_interested_changes_to_queue() noexcept
    {
        // This function should be called with mutex_  and interested_lock locked, because the queue is changed.
        new_ones_.add_list(new_interested_);
        old_ones_.add_list(old_interested_);
    }

private:

    struct ListInfo
    {
        ListInfo() noexcept
        {
            clear();
        }

        void swap(
                ListInfo& other) noexcept
        {
            if (other.is_empty())
            {
                clear();
            }
            else
            {
                head.writer_info.next = other.head.writer_info.next;
                tail.writer_info.previous = other.tail.writer_info.previous;
                other.clear();
                head.writer_info.next->writer_info.previous = &head;
                tail.writer_info.previous->writer_info.next = &tail;
            }
        }

        void clear() noexcept
        {
            head.writer_info.next = &tail;
            tail.writer_info.previous = &head;
        }

        bool is_empty() const noexcept
        {
            assert((&tail == head.writer_info.next && &head == tail.writer_info.previous) ||
                    (&tail != head.writer_info.next && &head != tail.writer_info.previous));
            return &tail == head.writer_info.next;
        }

        void add_change(
                CacheChange_t* change) noexcept
        {
            bool expected = false;
            if (change->writer_info.is_linked.compare_exchange_strong(expected, true))
            {
                change->writer_info.previous = tail.writer_info.previous;
                change->writer_info.previous->writer_info.next = change;
                tail.writer_info.previous = change;
                change->writer_info.next = &tail;
            }
        }

        void add_list(
                ListInfo& list) noexcept
        {
            if (!list.is_empty())
            {
                CacheChange_t* first = list.head.writer_info.next;
                CacheChange_t* last = list.tail.writer_info.previous;

                first->writer_info.previous = tail.writer_info.previous;
                first->writer_info.previous->writer_info.next = first;
                last->writer_info.next = &tail;
                tail.writer_info.previous = last;

                list.clear();
            }
        }

        CacheChange_t head;
        CacheChange_t tail;
    };

    //! List of interested new changes to be included.
    //! Should be protected with changes_interested_mutex.
    ListInfo new_interested_;

    //! List of interested old changes to be included.
    //! Should be protected with changes_interested_mutex.
    ListInfo old_interested_;

    //! List of new changes
    //! Should be protected with mutex_.
    ListInfo new_ones_;

    //! List of old changes
    //! Should be protected with mutex_.
    ListInfo old_ones_;
};

/** Classes used to specify FlowController's publication model **/

//! Only sends new samples synchronously. There is no mechanism to send old ones.
struct FlowControllerPureSyncPublishMode
{

    FlowControllerPureSyncPublishMode(
            RTPSParticipantImpl*,
            const FlowControllerDescriptor*)
    {
    }

};

//! Sends new samples asynchronously. Old samples are sent also asynchronously */
struct FlowControllerAsyncPublishMode
{
    FlowControllerAsyncPublishMode(
            RTPSParticipantImpl* participant,
            const FlowControllerDescriptor*)
        : group(participant, true)
    {
    }

    virtual ~FlowControllerAsyncPublishMode()
    {
        if (running)
        {
            {
                std::unique_lock<fastdds::TimedMutex> lock(changes_interested_mutex);
                running = false;
                cv.notify_one();
            }
            thread.join();
        }
    }

    bool fast_check_is_there_slot_for_change(
            CacheChange_t*) const
    {
        return true;
    }

    bool wait(
            std::unique_lock<fastdds::TimedMutex>& lock)
    {
        cv.wait(lock);
        return false;
    }

    bool force_wait() const
    {
        return false;
    }

    void process_deliver_retcode(
            const DeliveryRetCode&)
    {
    }

    eprosima::thread thread;

    std::atomic_bool running {false};

    fastdds::TimedConditionVariable cv;

    RTPSMessageGroup group;

    //! Mutex for interested samples to be added.
    fastdds::TimedMutex changes_interested_mutex;

    //! Used to warning async thread a writer wants to remove a sample.
    std::atomic<uint32_t> writers_interested_in_remove = {0};
};

//! Sends new samples synchronously. Old samples are sent asynchronously */
struct FlowControllerSyncPublishMode : public FlowControllerPureSyncPublishMode, public FlowControllerAsyncPublishMode
{

    FlowControllerSyncPublishMode(
            RTPSParticipantImpl* participant,
            const FlowControllerDescriptor* descriptor)
        : FlowControllerPureSyncPublishMode(participant, descriptor)
        , FlowControllerAsyncPublishMode(participant, descriptor)
    {
    }

};

//! Sends all samples asynchronously but with bandwidth limitation.
struct FlowControllerLimitedAsyncPublishMode : public FlowControllerAsyncPublishMode
{
    FlowControllerLimitedAsyncPublishMode(
            RTPSParticipantImpl* participant,
            const FlowControllerDescriptor* descriptor)
        : FlowControllerAsyncPublishMode(participant, descriptor)
    {
        assert(nullptr != descriptor);
        assert(0 < descriptor->max_bytes_per_period);

        max_bytes_per_period = descriptor->max_bytes_per_period;
        period_ms = std::chrono::milliseconds(descriptor->period_ms);
        group.set_sent_bytes_limitation(static_cast<uint32_t>(max_bytes_per_period));
    }

    bool fast_check_is_there_slot_for_change(
            CacheChange_t* change)
    {
        // Not fragmented sample, the fast check is if the serialized payload fit.
        uint32_t size_to_check = change->serializedPayload.length;

        if (0 != change->getFragmentCount())
        {
            // For fragmented sample, the fast check is the minor fragments fit.
            size_to_check = change->serializedPayload.length % change->getFragmentSize();

            if (0 == size_to_check)
            {
                size_to_check = change->getFragmentSize();
            }


        }

        bool ret = (max_bytes_per_period - group.get_current_bytes_processed()) > size_to_check;

        if (!ret)
        {
            force_wait_ = true;
        }

        return ret;
    }

    /*!
     * Wait until there is a new change added (notified by other thread) or there is a timeout (period was excedded and
     * the bandwidth limitation has to be reset.
     *
     * @return false if the condition_variable was awaken because a new change was added. true if the condition_variable was awaken because the bandwidth limitation has to be reset.
     */
    bool wait(
            std::unique_lock<fastdds::TimedMutex>& lock)
    {
        auto lapse = std::chrono::steady_clock::now() - last_period_;
        bool reset_limit = true;

        if (lapse < period_ms)
        {
            if (std::cv_status::no_timeout == cv.wait_for(lock, period_ms - lapse))
            {
                reset_limit = false;
            }
        }

        if (reset_limit)
        {
            last_period_ = std::chrono::steady_clock::now();
            force_wait_ = false;
            group.reset_current_bytes_processed();
        }

        return reset_limit;
    }

    bool force_wait() const
    {
        return force_wait_;
    }

    void process_deliver_retcode(
            const DeliveryRetCode& ret_value)
    {
        if (DeliveryRetCode::EXCEEDED_LIMIT == ret_value)
        {
            force_wait_ = true;
        }
    }

    int32_t max_bytes_per_period = 0;

    std::chrono::milliseconds period_ms;

private:

    bool force_wait_ = false;

    std::chrono::steady_clock::time_point last_period_ = std::chrono::steady_clock::now();
};


/** Classes used to specify FlowController's sample scheduling **/

//! Fifo scheduling
struct FlowControllerFifoSchedule
{
    void register_writer(
            BaseWriter*) const
    {
    }

    void unregister_writer(
            BaseWriter*) const
    {
    }

    void work_done() const
    {
        // Do nothing
    }

    void add_new_sample(
            BaseWriter*,
            CacheChange_t* change)
    {
        queue_.add_new_sample(change);
    }

    void add_old_sample(
            BaseWriter*,
            CacheChange_t* change)
    {
        queue_.add_old_sample(change);
    }

    /*!
     * Returns the first sample in the queue.
     * Default behaviour.
     * Expects the queue is ordered.
     *
     * @return Pointer to next change to be sent. nullptr implies there is no sample to be sent or is forbidden due to
     * bandwidth exceeded.
     */
    CacheChange_t* get_next_change_nts()
    {
        return queue_.get_next_change();
    }

    /*!
     * Store the sample at the end of the list.
     *
     * @return true if there is added changes.
     */
    void add_interested_changes_to_queue_nts()
    {
        // This function should be called with mutex_  and interested_lock locked, because the queue is changed.
        queue_.add_interested_changes_to_queue();
    }

    void set_bandwith_limitation(
            uint32_t) const
    {
    }

    void trigger_bandwidth_limit_reset() const
    {
    }

private:

    //! Scheduler queue. FIFO scheduler only has one queue.
    FlowQueue queue_;
};

//! Round Robin scheduling
struct FlowControllerRoundRobinSchedule
{
    using element = std::tuple<BaseWriter*, FlowQueue>;
    using container = std::vector<element>;
    using iterator = container::iterator;

    FlowControllerRoundRobinSchedule()
    {
        next_writer_ = writers_queue_.begin();
    }

    void register_writer(
            BaseWriter* writer)
    {
        BaseWriter* current_writer = nullptr;

        if (writers_queue_.end() != next_writer_)
        {
            current_writer = std::get<0>(*next_writer_);
        }

        assert(writers_queue_.end() == find(writer));
        writers_queue_.emplace_back(writer, FlowQueue());

        if (nullptr == current_writer)
        {
            next_writer_ = writers_queue_.begin();
        }
        else
        {
            next_writer_ = find(current_writer);
        }
    }

    void unregister_writer(
            BaseWriter* writer)
    {
        // Queue cannot be empty, as writer should be present
        assert(writers_queue_.end() != next_writer_);
        BaseWriter* current_writer = std::get<0>(*next_writer_);
        assert(nullptr != current_writer);

        auto it = find(writer);
        assert(it != writers_queue_.end());
        assert(std::get<1>(*it).is_empty());

        // Go to the next writer when unregistering the current one
        if (it == next_writer_)
        {
            set_next_writer();
            current_writer = std::get<0>(*next_writer_);
        }

        writers_queue_.erase(it);
        if (writer == current_writer)
        {
            next_writer_ = writers_queue_.begin();
        }
        else
        {
            next_writer_ = find(current_writer);
        }
    }

    void work_done()
    {
        assert(0 < writers_queue_.size());
        assert(writers_queue_.end() != next_writer_);
        set_next_writer();
    }

    iterator set_next_writer()
    {
        iterator next = std::next(next_writer_);
        next_writer_ = writers_queue_.end() == next ? writers_queue_.begin() : next;
        return next_writer_;
    }

    void add_new_sample(
            BaseWriter* writer,
            CacheChange_t* change)
    {
        auto it = find(writer);
        assert(it != writers_queue_.end());
        std::get<1>(*it).add_new_sample(change);
    }

    void add_old_sample(
            BaseWriter* writer,
            CacheChange_t* change)
    {
        auto it = find(writer);
        assert(it != writers_queue_.end());
        std::get<1>(*it).add_old_sample(change);
    }

    CacheChange_t* get_next_change_nts()
    {
        CacheChange_t* ret_change = nullptr;

        if (0 < writers_queue_.size())
        {
            auto starting_it = next_writer_;     // For avoid loops.

            do
            {
                ret_change = std::get<1>(*next_writer_).get_next_change();
            } while (nullptr == ret_change && starting_it != set_next_writer());
        }

        return ret_change;
    }

    void add_interested_changes_to_queue_nts()
    {
        // This function should be called with mutex_  and interested_lock locked, because the queue is changed.
        for (auto& queue : writers_queue_)
        {
            std::get<1>(queue).add_interested_changes_to_queue();
        }
    }

    void set_bandwith_limitation(
            uint32_t) const
    {
    }

    void trigger_bandwidth_limit_reset() const
    {
    }

private:

    iterator find(
            const BaseWriter* writer)
    {
        return std::find_if(writers_queue_.begin(), writers_queue_.end(),
                       [writer](const element& current_writer) -> bool
                       {
                           return writer == std::get<0>(current_writer);
                       });
    }

    container writers_queue_;
    iterator next_writer_;

};

//! High priority scheduling
struct FlowControllerHighPrioritySchedule
{
    void register_writer(
            BaseWriter* writer)
    {
        assert(nullptr != writer);
        int32_t priority = 10;
        auto property = PropertyPolicyHelper::find_property(
            writer->getAttributes().properties, "fastdds.sfc.priority");

        if (nullptr != property)
        {
            char* ptr = nullptr;
            priority = strtol(property->c_str(), &ptr, 10);

            if (property->c_str() != ptr)     // A valid integer was read.
            {
                if (-10 > priority || 10 < priority)
                {
                    priority = 10;
                    EPROSIMA_LOG_ERROR(RTPS_WRITER,
                            "Wrong value for fastdds.sfc.priority property. Range is [-10, 10]. Priority set to lowest (10)");
                }
            }
            else
            {
                priority = 10;
                EPROSIMA_LOG_ERROR(RTPS_WRITER,
                        "Not numerical value for fastdds.sfc.priority property. Priority set to lowest (10)");
            }
        }

        auto ret = priorities_.insert({writer, priority});
        (void)ret;
        assert(ret.second);

        // Ensure the priority is created.
        FlowQueue& queue = writers_queue_[priority];
        (void)queue;
    }

    void unregister_writer(
            BaseWriter* writer)
    {
        auto it = priorities_.find(writer);
        assert(it != priorities_.end());
        priorities_.erase(it);
    }

    void work_done() const
    {
        // Do nothing
    }

    void add_new_sample(
            BaseWriter* writer,
            CacheChange_t* change)
    {
        find_queue(writer).add_new_sample(change);
    }

    void add_old_sample(
            BaseWriter* writer,
            CacheChange_t* change)
    {
        find_queue(writer).add_old_sample(change);
    }

    CacheChange_t* get_next_change_nts()
    {
        CacheChange_t* ret_change = nullptr;

        if (0 < writers_queue_.size())
        {
            for (auto it = writers_queue_.begin(); nullptr == ret_change && it != writers_queue_.end(); ++it)
            {
                ret_change = it->second.get_next_change();
            }
        }

        return ret_change;
    }

    void add_interested_changes_to_queue_nts()
    {
        // This function should be called with mutex_  and interested_lock locked, because the queue is changed.
        for (auto& queue : writers_queue_)
        {
            queue.second.add_interested_changes_to_queue();
        }
    }

    void set_bandwith_limitation(
            uint32_t) const
    {
    }

    void trigger_bandwidth_limit_reset() const
    {
    }

private:

    FlowQueue& find_queue(
            BaseWriter* writer)
    {
        // Find priority.
        auto priority_it = priorities_.find(writer);
        assert(priority_it != priorities_.end());
        auto queue_it = writers_queue_.find(priority_it->second);
        assert(queue_it != writers_queue_.end());
        return queue_it->second;
    }

    std::map<int32_t, FlowQueue> writers_queue_;

    std::unordered_map<BaseWriter*, int32_t> priorities_;
};

//! Priority with reservation scheduling
struct FlowControllerPriorityWithReservationSchedule
{
    void register_writer(
            BaseWriter* writer)
    {
        assert(nullptr != writer);
        int32_t priority = 10;
        auto property = PropertyPolicyHelper::find_property(
            writer->getAttributes().properties, "fastdds.sfc.priority");

        if (nullptr != property)
        {
            char* ptr = nullptr;
            priority = strtol(property->c_str(), &ptr, 10);

            if (property->c_str() != ptr)     // A valid integer was read.
            {
                if (-10 > priority || 10 < priority)
                {
                    priority = 10;
                    EPROSIMA_LOG_ERROR(RTPS_WRITER,
                            "Wrong value for fastdds.sfc.priority property. Range is [-10, 10]. Priority set to lowest (10)");
                }
            }
            else
            {
                priority = 10;
                EPROSIMA_LOG_ERROR(RTPS_WRITER,
                        "Not numerical value for fastdds.sfc.priority property. Priority set to lowest (10)");
            }
        }

        uint32_t reservation = 0;
        property = PropertyPolicyHelper::find_property(
            writer->getAttributes().properties, "fastdds.sfc.bandwidth_reservation");

        if (nullptr != property)
        {
            char* ptr = nullptr;
            reservation = strtoul(property->c_str(), &ptr, 10);

            if (property->c_str() != ptr)     // A valid integer was read.
            {
                if (100 < reservation)
                {
                    reservation = 0;
                    EPROSIMA_LOG_ERROR(RTPS_WRITER,
                            "Wrong value for fastdds.sfc.bandwidth_reservation property. Range is [0, 100]. Reservation set to lowest (0)");
                }
            }
            else
            {
                reservation = 0;
                EPROSIMA_LOG_ERROR(RTPS_WRITER,
                        "Not numerical value for fastdds.sfc.bandwidth_reservation property. Reservation set to lowest (0)");
            }
        }

        // Calculate reservation in bytes.
        uint32_t reservation_bytes = (0 == bandwidth_limit_? 0 :
                ((bandwidth_limit_ * reservation) / 100));

        auto ret = writers_queue_.emplace(writer, std::make_tuple(FlowQueue(), priority, reservation_bytes, 0u));
        (void)ret;
        assert(ret.second);

        priorities_[priority].push_back(writer);
    }

    void unregister_writer(
            BaseWriter* writer)
    {
        auto it = writers_queue_.find(writer);
        assert(it != writers_queue_.end());
        int32_t priority = std::get<1>(it->second);
        writers_queue_.erase(it);
        auto priority_it = priorities_.find(priority);
        assert(priority_it != priorities_.end());
        auto writer_it = std::find(priority_it->second.begin(), priority_it->second.end(), writer);
        assert(writer_it != priority_it->second.end());
        priority_it->second.erase(writer_it);
    }

    void work_done()
    {
        if (nullptr != writer_being_processed_)
        {
            assert(0 != size_being_processed_);
            auto writer = writers_queue_.find(writer_being_processed_);
            std::get<3>(writer->second) += size_being_processed_;
            writer_being_processed_ = nullptr;
            size_being_processed_ = 0;
        }
    }

    void add_new_sample(
            BaseWriter* writer,
            CacheChange_t* change)
    {
        // Find writer queue..
        auto it = writers_queue_.find(writer);
        assert(it != writers_queue_.end());
        std::get<0>(it->second).add_new_sample(change);
    }

    void add_old_sample(
            BaseWriter* writer,
            CacheChange_t* change)
    {
        // Find writer queue..
        auto it = writers_queue_.find(writer);
        assert(it != writers_queue_.end());
        std::get<0>(it->second).add_old_sample(change);
    }

    CacheChange_t* get_next_change_nts()
    {
        CacheChange_t* highest_priority = nullptr;
        CacheChange_t* ret_change = nullptr;

        if (0 < writers_queue_.size())
        {
            for (auto& priority : priorities_)
            {
                for (auto writer_it : priority.second)
                {
                    auto writer = writers_queue_.find(writer_it);
                    CacheChange_t* change = std::get<0>(writer->second).get_next_change();

                    if (nullptr == highest_priority)
                    {
                        highest_priority = change;
                    }

                    if (nullptr != change)
                    {
                        // Check if writer's next change can be processed because the writer's bandwidth reservation is
                        // enough.
                        uint32_t size_to_check = change->serializedPayload.length;
                        if (0 != change->getFragmentCount())
                        {
                            size_to_check = change->getFragmentSize();
                        }

                        if (std::get<2>(writer->second) > (std::get<3>(writer->second) + size_to_check))
                        {
                            ret_change = change;
                            writer_being_processed_ = writer_it;
                            size_being_processed_ = size_to_check;
                            break;
                        }
                    }
                }

                if (nullptr != ret_change)
                {
                    break;
                }
            }
        }

        return (nullptr != ret_change ? ret_change : highest_priority);
    }

    void add_interested_changes_to_queue_nts()
    {
        // This function should be called with mutex_  and interested_lock locked, because the queue is changed.
        for (auto& queue : writers_queue_)
        {
            std::get<0>(queue.second).add_interested_changes_to_queue();
        }
    }

    void set_bandwith_limitation(
            uint32_t limit)
    {
        bandwidth_limit_ = limit;
    }

    void trigger_bandwidth_limit_reset()
    {
        for (auto& writer : writers_queue_)
        {
            std::get<3>(writer.second) = 0;
        }
    }

private:

    using map_writers = std::unordered_map<BaseWriter*, std::tuple<FlowQueue, int32_t, uint32_t,
                    uint32_t>>;

    using map_priorities = std::map<int32_t, std::vector<BaseWriter*>>;

    map_writers writers_queue_;

    map_priorities priorities_;

    uint32_t bandwidth_limit_ = 0;

    BaseWriter* writer_being_processed_ = nullptr;

    uint32_t size_being_processed_ = 0;
};

template<typename PublishMode, typename SampleScheduling>
class FlowControllerImpl : public FlowController
{
    using publish_mode = PublishMode;
    using scheduler = SampleScheduling;

public:

    FlowControllerImpl(
            RTPSParticipantImpl* participant,
            const FlowControllerDescriptor* descriptor,
            uint32_t async_index,
            ThreadSettings thread_settings)
        : participant_(participant)
        , async_mode(participant, descriptor)
        , participant_id_(0)
        , async_index_(async_index)
        , thread_settings_(thread_settings)
    {
        if (nullptr != participant)
        {
            participant_id_ = static_cast<uint32_t>(participant->get_attributes().participantID);
        }

        uint32_t limitation = get_max_payload();

        if ((std::numeric_limits<uint32_t>::max)() != limitation)
        {
            sched.set_bandwith_limitation(limitation);
        }
    }

    virtual ~FlowControllerImpl() noexcept
    {
    }

    /*!
     * Initializes the flow controller.
     */
    void init() override
    {
        initialize_async_thread();
    }

    /*!
     * Registers a writer.
     * This object is only be able to manage a CacheChante_t if its writer was registered previously with this function.
     *
     * @param writer Pointer to the writer to be registered. Cannot be nullptr.
     */
    void register_writer(
            BaseWriter* writer) override
    {
        std::unique_lock<fastdds::TimedMutex> lock(mutex_);
        auto ret = writers_.insert({ writer->getGuid(), writer});
        (void)ret;
        assert(ret.second);
        register_writer_impl(writer);
    }

    /*!
     * Unregister a writer.
     *
     * @param writer Pointer to the writer to be unregistered. Cannot be nullptr.
     */
    void unregister_writer(
            BaseWriter* writer) override
    {
        std::unique_lock<fastdds::TimedMutex> lock(mutex_);
        writers_.erase(writer->getGuid());
        unregister_writer_impl(writer);
    }

    /*
     * Adds the CacheChange_t to be managed by this object.
     * The CacheChange_t has to be a new one, that is, it has to be added to the writer's history before this call.
     * This function should be called by BaseWriter::unsent_change_added_to_history().
     * This function has two specializations depending on template parameter PublishMode.
     *
     * @param Pointer to the writer which the added CacheChante_t is responsable. Cannot be nullptr.
     * @param change Pointer to the new CacheChange_t to be managed by this object. Cannot be nullptr.
     * @return true if sample could be added. false in other case.
     */
    bool add_new_sample(
            BaseWriter* writer,
            CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) override
    {
        return add_new_sample_impl(writer, change, max_blocking_time);
    }

    /*!
     * Adds the CacheChante_t to be managed by this object.
     * The CacheChange_t has to be an old one, that is, it is already in the writer's history and for some reason has to
     * be sent again.
     *
     * @param Pointer to the writer which the added change is responsable. Cannot be nullptr.
     * @param change Pointer to the old change to be managed by this object. Cannot be nullptr.
     * @return true if sample could be added. false in other case.
     */
    bool add_old_sample(
            BaseWriter* writer,
            CacheChange_t* change) override
    {
        return add_old_sample_impl(writer, change,
                       std::chrono::steady_clock::now() + std::chrono::hours(24));
    }

    /*!
     * If currently the CacheChange_t is managed by this object, remove it.
     * This funcion should be called when a CacheChange_t is removed from the writer's history.
     *
     * @param [in] change Pointer to the change which should be removed if it is currently managed by this object.
     * @param [in] max_blocking_time Maximum time this method has to complete the task.
     * @return true if the sample could be removed. false otherwise.
     */
    bool remove_change(
            CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) override
    {
        assert(nullptr != change);
        return remove_change_impl(change, max_blocking_time);
    }

    uint32_t get_max_payload() override
    {
        return get_max_payload_impl();
    }

private:

    /*!
     * Initialize asynchronous thread.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    initialize_async_thread()
    {
        bool expected = false;
        if (async_mode.running.compare_exchange_strong(expected, true))
        {
            // Code for initializing the asynchronous thread.
            async_mode.thread = create_thread([this]()
                            {
                                run();
                            }, thread_settings_, "dds.asyn.%u.%u", participant_id_, async_index_);
        }
    }

    /*! This function is used when PublishMode = FlowControllerPureSyncPublishMode.
     *  In this case the async thread doesn't need to be initialized.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    initialize_async_thread()
    {
        // Do nothing.
    }

    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    register_writer_impl(
            BaseWriter* writer)
    {
        std::unique_lock<fastdds::TimedMutex> in_lock(async_mode.changes_interested_mutex);
        sched.register_writer(writer);
    }

    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    register_writer_impl(
            BaseWriter*)
    {
        // Do nothing.
    }

    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    unregister_writer_impl(
            BaseWriter* writer)
    {
        std::unique_lock<fastdds::TimedMutex> in_lock(async_mode.changes_interested_mutex);
        sched.unregister_writer(writer);
    }

    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    unregister_writer_impl(
            BaseWriter*)
    {
        // Do nothing.
    }

    /*!
     * This function store internally the sample and wake up the async thread.
     *
     * @note Before calling this function, the change's writer mutex have to be locked.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    enqueue_new_sample_impl(
            BaseWriter* writer,
            CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
    {
        bool ret_value = false;
        // Sync delivery failed. Try to store for asynchronous delivery.
#if HAVE_STRICT_REALTIME
        std::unique_lock<fastdds::TimedMutex> lock(async_mode.changes_interested_mutex, std::defer_lock);
        if (lock.try_lock_until(max_blocking_time))
#else
        static_cast<void>(max_blocking_time);
        std::unique_lock<fastdds::TimedMutex> lock(async_mode.changes_interested_mutex);
#endif // if HAVE_STRICT_REALTIME{
        {
            sched.add_new_sample(writer, change);
            async_mode.cv.notify_one();
            ret_value = true;
        }

        return ret_value;
    }

    /*! This function is used when PublishMode = FlowControllerPureSyncPublishMode.
     *  In this case there is no async mechanism.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    constexpr enqueue_new_sample_impl(
            BaseWriter*,
            CacheChange_t*,
            const std::chrono::time_point<std::chrono::steady_clock>&) const
    {
        // Do nothing. Return false.
        return false;
    }

    /*!
     * This function tries to send the sample synchronously.
     * That is, it uses the user's thread, which is the one calling this function, to send the sample.
     * It calls new function `BaseWriter::deliver_sample_nts()` for sending the sample.
     * If this function fails (for example because non-blocking socket is full), this function stores internally the sample to
     * try sending it again asynchronously.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_base_of<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    add_new_sample_impl(
            BaseWriter* writer,
            CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
    {
        bool ret_value = false;
        // This call should be made with writer's mutex locked.
        LocatorSelectorSender& locator_selector = writer->get_general_locator_selector();
#if HAVE_STRICT_REALTIME
        std::unique_lock<LocatorSelectorSender> lock(locator_selector, std::defer_lock);
        if (lock.try_lock_until(max_blocking_time))
#else
        std::unique_lock<LocatorSelectorSender> lock(locator_selector);
#endif // if HAVE_STRICT_REALTIME{
        {
            try
            {
                RTPSMessageGroup group(participant_, writer, &locator_selector, max_blocking_time);
                ret_value = true;
                if (DeliveryRetCode::DELIVERED !=
                        writer->deliver_sample_nts(change, group, locator_selector, max_blocking_time))
                {
                    ret_value =  enqueue_new_sample_impl(writer, change, max_blocking_time);
                }
            }
            catch (RTPSMessageGroup::timeout&)
            {
            }
        }

        return ret_value;
    }

    /*!
     * This function stores internally the sample to send it asynchronously.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_base_of<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    add_new_sample_impl(
            BaseWriter* writer,
            CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
    {
        return enqueue_new_sample_impl(writer, change, max_blocking_time);
    }

    /*!
     * This function store internally the sample and wake up the async thread.
     *
     * @note Before calling this function, the change's writer mutex have to be locked.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    add_old_sample_impl(
            BaseWriter* writer,
            CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
    {
        bool ret_value = false;

        if (!change->writer_info.is_linked.load())
        {
#if HAVE_STRICT_REALTIME
            std::unique_lock<fastdds::TimedMutex> lock(async_mode.changes_interested_mutex, std::defer_lock);
            if (lock.try_lock_until(max_blocking_time))
#else
            static_cast<void>(max_blocking_time);
            std::unique_lock<fastdds::TimedMutex> lock(async_mode.changes_interested_mutex);
#endif // if HAVE_STRICT_REALTIME{
            {
                sched.add_old_sample(writer, change);
                async_mode.cv.notify_one();
                ret_value = true;
            }
        }

        return ret_value;
    }

    /*! This function is used when PublishMode = FlowControllerPureSyncPublishMode.
     *  In this case there is no async mechanism.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    constexpr add_old_sample_impl(
            BaseWriter*,
            CacheChange_t*,
            const std::chrono::time_point<std::chrono::steady_clock>&) const
    {
        return false;
    }

    /*!
     * This function store internally the sample and wake up the async thread.
     *
     * @note Before calling this function, the change's writer mutex have to be locked.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    remove_change_impl(
            CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
    {
        bool ret_value = true;
        if (change->writer_info.is_linked.load())
        {
            ++async_mode.writers_interested_in_remove;
#if HAVE_STRICT_REALTIME
            std::unique_lock<fastdds::TimedMutex> lock(mutex_, std::defer_lock);
            if (lock.try_lock_until(max_blocking_time))
#else
            static_cast<void>(max_blocking_time);
            std::unique_lock<fastdds::TimedMutex> lock(mutex_);
#endif // if HAVE_STRICT_REALTIME
            {
#if HAVE_STRICT_REALTIME
                std::unique_lock<fastdds::TimedMutex> interested_lock(async_mode.changes_interested_mutex,
                        std::defer_lock);
                if (interested_lock.try_lock_until(max_blocking_time))
#else
                std::unique_lock<fastdds::TimedMutex> interested_lock(async_mode.changes_interested_mutex);
#endif // if HAVE_STRICT_REALTIME
                {

                    // When blocked, both pointer are different than nullptr or equal.
                    assert((nullptr != change->writer_info.previous &&
                            nullptr != change->writer_info.next) ||
                            (nullptr == change->writer_info.previous &&
                            nullptr == change->writer_info.next));
                    if (change->writer_info.is_linked.load())
                    {

                        // Try to join previous node and next node.
                        change->writer_info.previous->writer_info.next = change->writer_info.next;
                        change->writer_info.next->writer_info.previous = change->writer_info.previous;
                        change->writer_info.previous = nullptr;
                        change->writer_info.next = nullptr;
                        change->writer_info.is_linked.store(false);
                    }
                }
#if HAVE_STRICT_REALTIME
                else
                {
                    ret_value = !change->writer_info.is_linked.load();
                }
#endif // if HAVE_STRICT_REALTIME
            }
#if HAVE_STRICT_REALTIME
            else
            {
                ret_value = !change->writer_info.is_linked.load();
            }
#endif // if HAVE_STRICT_REALTIME
            --async_mode.writers_interested_in_remove;
        }

        return ret_value;
    }

    /*! This function is used when PublishMode = FlowControllerPureSyncPublishMode.
     *  In this case there is no async mechanism.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    remove_change_impl(
            CacheChange_t*,
            const std::chrono::time_point<std::chrono::steady_clock>&)
    {
        // Do nothing.
        return true;
    }

    /*!
     * Function run by the asynchronous thread.
     */
    void run()
    {
        while (async_mode.running)
        {
            // There are writers interested in removing a sample.
            if (0 != async_mode.writers_interested_in_remove)
            {
                continue;
            }

            std::unique_lock<fastdds::TimedMutex> lock(mutex_);
            CacheChange_t* change_to_process = nullptr;

            //Check if we have to sleep.
            {
                std::unique_lock<fastdds::TimedMutex> in_lock(async_mode.changes_interested_mutex);
                // Add interested changes into the queue.
                sched.add_interested_changes_to_queue_nts();

                while (async_mode.running &&
                        (async_mode.force_wait() || nullptr == (change_to_process = sched.get_next_change_nts())))
                {
                    // Release main mutex to allow registering/unregistering writers while this thread is waiting.
                    lock.unlock();
                    bool ret = async_mode.wait(in_lock);

                    in_lock.unlock();
                    lock.lock();
                    in_lock.lock();

                    if (ret)
                    {
                        sched.trigger_bandwidth_limit_reset();
                    }
                    sched.add_interested_changes_to_queue_nts();
                }
            }

            BaseWriter* current_writer = nullptr;
            while (nullptr != change_to_process)
            {
                // Fast check if next change will enter.
                if (!async_mode.fast_check_is_there_slot_for_change(change_to_process))
                {
                    break;
                }

                if (nullptr == current_writer || current_writer->getGuid() != change_to_process->writerGUID)
                {
                    auto writer_it = writers_.find(change_to_process->writerGUID);
                    assert(writers_.end() != writer_it);

                    current_writer = writer_it->second;
                }

                if (!current_writer->getMutex().try_lock())
                {
                    break;
                }

                LocatorSelectorSender& locator_selector =
                        current_writer->get_async_locator_selector();
                async_mode.group.sender(current_writer, &locator_selector);
                locator_selector.lock();

                // Remove previously from queue, because deliver_sample_nts could call FlowController::remove_sample()
                // provoking a deadlock.
                CacheChange_t* previous = change_to_process->writer_info.previous;
                CacheChange_t* next = change_to_process->writer_info.next;
                previous->writer_info.next = next;
                next->writer_info.previous = previous;
                change_to_process->writer_info.previous = nullptr;
                change_to_process->writer_info.next = nullptr;
                change_to_process->writer_info.is_linked.store(false);

                DeliveryRetCode ret_delivery = current_writer->deliver_sample_nts(
                    change_to_process, async_mode.group, locator_selector,
                    std::chrono::steady_clock::now() + std::chrono::hours(24));

                if (DeliveryRetCode::DELIVERED != ret_delivery)
                {
                    // If delivery fails, put the change again in the queue.
                    change_to_process->writer_info.is_linked.store(true);
                    previous->writer_info.next = change_to_process;
                    next->writer_info.previous = change_to_process;
                    change_to_process->writer_info.previous = previous;
                    change_to_process->writer_info.next = next;

                    async_mode.process_deliver_retcode(ret_delivery);

                    locator_selector.unlock();
                    current_writer->getMutex().unlock();
                    // Unlock mutex_ and try again.
                    break;
                }

                locator_selector.unlock();
                current_writer->getMutex().unlock();

                sched.work_done();

                if (0 != async_mode.writers_interested_in_remove)
                {
                    // There are writers that want to remove samples.
                    break;
                }

                // Add interested changes into the queue.
                {
                    std::unique_lock<fastdds::TimedMutex> in_lock(async_mode.changes_interested_mutex);
                    sched.add_interested_changes_to_queue_nts();
                }

                change_to_process = sched.get_next_change_nts();
            }

            async_mode.group.sender(nullptr, nullptr);
        }
    }

    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_base_of<FlowControllerLimitedAsyncPublishMode, PubMode>::value, uint32_t>::type
    get_max_payload_impl()
    {
        return static_cast<uint32_t>(async_mode.max_bytes_per_period);
    }

    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_base_of<FlowControllerLimitedAsyncPublishMode, PubMode>::value, uint32_t>::type
    constexpr get_max_payload_impl() const
    {
        return (std::numeric_limits<uint32_t>::max)();
    }

    fastdds::TimedMutex mutex_;

    RTPSParticipantImpl* participant_ = nullptr;

    std::map<GUID_t, BaseWriter*> writers_;

    scheduler sched;

    // async_mode must be destroyed before sched.
    publish_mode async_mode;

    uint32_t participant_id_ = 0;
    uint32_t async_index_ = 0;

    //! Thread settings for the sender thread
    ThreadSettings thread_settings_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _RTPS_FLOWCONTROL_FLOWCONTROLLERIMPL_HPP_
