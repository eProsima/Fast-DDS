#ifndef _RTPS_FLOWCONTROL_FLOWCONTROLLERIMPL_HPP_
#define _RTPS_FLOWCONTROL_FLOWCONTROLLERIMPL_HPP_

#include <fastdds/rtps/flowcontrol/FlowController.hpp>
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/writer/RTPSWriter.h>

#include <map>
#include <thread>
#include <mutex>
#include <cassert>
#include <condition_variable>
#include <chrono>

namespace eprosima {
namespace fastdds {
namespace rtps {

/** Auxiliary classes **/

struct FlowQueue
{
    FlowQueue()
    {
        head_new_interested.writer_info.next = &tail_new_interested;
        tail_new_interested.writer_info.previous = &head_new_interested;
        head_old_interested.writer_info.next = &tail_old_interested;
        tail_old_interested.writer_info.previous = &head_old_interested;
        head_new_ones.writer_info.next = &tail_new_ones;
        tail_new_ones.writer_info.previous = &head_new_ones;
        head_old_ones.writer_info.next = &tail_old_ones;
        tail_old_ones.writer_info.previous = &head_old_ones;
    }

    FlowQueue(
            FlowQueue&& old)
    {
        if (old.head_new_interested.writer_info.next == &old.tail_new_interested)
        {
            assert(old.tail_new_interested.writer_info.previous == &old.head_new_interested);
            head_new_interested.writer_info.next = &tail_new_interested;
            tail_new_interested.writer_info.previous = &head_new_interested;
        }
        else
        {
            assert(old.tail_new_interested.writer_info.previous != &old.head_new_interested);
            head_new_interested.writer_info.next = old.head_new_interested.writer_info.next;
            tail_new_interested.writer_info.previous = old.tail_new_interested.writer_info.previous;
            old.head_new_interested.writer_info.next = &old.tail_new_interested;
            old.tail_new_interested.writer_info.previous = &old.head_new_interested;
            head_new_interested.writer_info.next->writer_info.previous = &head_new_interested;
            tail_new_interested.writer_info.previous->writer_info.next = &tail_new_interested;
        }

        if (old.head_old_interested.writer_info.next == &old.tail_old_interested)
        {
            assert(old.tail_old_interested.writer_info.previous == &old.head_old_interested);
            head_old_interested.writer_info.next = &tail_old_interested;
            tail_old_interested.writer_info.previous = &head_old_interested;
        }
        else
        {
            assert(old.tail_old_interested.writer_info.previous != &old.head_old_interested);
            head_old_interested.writer_info.next = old.head_old_interested.writer_info.next;
            tail_old_interested.writer_info.previous = old.tail_old_interested.writer_info.previous;
            old.head_old_interested.writer_info.next = &old.tail_old_interested;
            old.tail_old_interested.writer_info.previous = &old.head_old_interested;
            head_old_interested.writer_info.next->writer_info.previous = &head_old_interested;
            tail_old_interested.writer_info.previous->writer_info.next = &tail_old_interested;
        }


        if (old.head_new_ones.writer_info.next == &old.tail_new_ones)
        {
            assert(old.tail_new_ones.writer_info.previous == &old.head_new_ones);
            head_new_ones.writer_info.next = &tail_new_ones;
            tail_new_ones.writer_info.previous = &head_new_ones;
        }
        else
        {
            assert(old.tail_new_ones.writer_info.previous != &old.head_new_ones);
            head_new_ones.writer_info.next = old.head_new_ones.writer_info.next;
            tail_new_ones.writer_info.previous = old.tail_new_ones.writer_info.previous;
            old.head_new_ones.writer_info.next = &old.tail_new_ones;
            old.tail_new_ones.writer_info.previous = &old.head_new_ones;
            head_new_ones.writer_info.next->writer_info.previous = &head_new_ones;
            tail_new_ones.writer_info.previous->writer_info.next = &tail_new_ones;
        }


        if (old.head_old_ones.writer_info.next == &old.tail_old_ones)
        {
            assert(old.tail_old_ones.writer_info.previous == &old.head_old_ones);
            head_old_ones.writer_info.next = &tail_old_ones;
            tail_old_ones.writer_info.previous = &head_old_ones;
        }
        else
        {
            assert(old.tail_old_ones.writer_info.previous != &old.head_old_ones);
            head_old_ones.writer_info.next = old.head_old_ones.writer_info.next;
            tail_old_ones.writer_info.previous = old.tail_old_ones.writer_info.previous;
            old.head_old_ones.writer_info.next = &old.tail_old_ones;
            old.tail_old_ones.writer_info.previous = &old.head_old_ones;
            head_old_ones.writer_info.next->writer_info.previous = &head_old_ones;
            tail_old_ones.writer_info.previous->writer_info.next = &tail_old_ones;
        }
    }

    ~FlowQueue()
    {
        assert(&tail_new_interested == head_new_interested.writer_info.next);
        assert(&tail_old_interested == head_old_interested.writer_info.next);
    }

    bool is_empty() const
    {
        assert(((&tail_new_ones == head_new_ones.writer_info.next &&
                &head_new_ones == tail_new_ones.writer_info.previous) ||
                (&tail_new_ones != head_new_ones.writer_info.next &&
                &head_new_ones != tail_new_ones.writer_info.previous)) &&
                ((&tail_old_ones == head_old_ones.writer_info.next &&
                &head_old_ones == tail_old_ones.writer_info.previous) ||
                (&tail_old_ones != head_old_ones.writer_info.next &&
                &head_old_ones != tail_old_ones.writer_info.previous)));

        return &tail_new_ones == head_new_ones.writer_info.next &&
               &tail_old_ones == head_old_ones.writer_info.next;
    }

    void add_new_sample(
            fastrtps::rtps::CacheChange_t* change)
    {
        change->writer_info.previous = tail_new_interested.writer_info.previous;
        change->writer_info.previous->writer_info.next = change;
        tail_new_interested.writer_info.previous = change;
        change->writer_info.next = &tail_new_interested;

    }

    void add_old_sample(
            fastrtps::rtps::CacheChange_t* change)
    {
        change->writer_info.previous = tail_old_interested.writer_info.previous;
        change->writer_info.previous->writer_info.next = change;
        tail_old_interested.writer_info.previous = change;
        change->writer_info.next = &tail_old_interested;
    }

    fastrtps::rtps::CacheChange_t* get_next_change()
    {
        if (!is_empty())
        {
            return &tail_new_ones !=
                   head_new_ones.writer_info.next ?
                   head_new_ones.writer_info.next : head_old_ones.writer_info.next;
        }

        return nullptr;
    }

    void add_interested_changes_to_queue()
    {
        // This function should be called with mutex_  and interested_lock locked, because the queue is changed.
        assert(((&tail_new_interested == head_new_interested.writer_info.next &&
                &head_new_interested == tail_new_interested.writer_info.previous) ||
                (&tail_new_interested != head_new_interested.writer_info.next &&
                &head_new_interested != tail_new_interested.writer_info.previous)) &&
                ((&tail_old_interested == head_old_interested.writer_info.next &&
                &head_old_interested == tail_old_interested.writer_info.previous) ||
                (&tail_old_interested != head_old_interested.writer_info.next &&
                &head_old_interested != tail_old_interested.writer_info.previous)));

        fastrtps::rtps::CacheChange_t* interested_it = head_new_interested.writer_info.next;
        fastrtps::rtps::CacheChange_t* next_it = nullptr;
        while (&tail_new_interested != interested_it)
        {
            next_it = interested_it->writer_info.next;
            interested_it->writer_info.previous->writer_info.next = interested_it->writer_info.next;
            interested_it->writer_info.next->writer_info.previous = interested_it->writer_info.previous;
            interested_it->writer_info.previous = tail_new_ones.writer_info.previous;
            interested_it->writer_info.previous->writer_info.next = interested_it;
            tail_new_ones.writer_info.previous = interested_it;
            interested_it->writer_info.next = &tail_new_ones;

            interested_it = next_it;
        }

        interested_it = head_old_interested.writer_info.next;
        next_it = nullptr;
        while (&tail_old_interested != interested_it)
        {
            next_it = interested_it->writer_info.next;
            interested_it->writer_info.previous->writer_info.next = interested_it->writer_info.next;
            interested_it->writer_info.next->writer_info.previous = interested_it->writer_info.previous;
            interested_it->writer_info.previous = tail_old_ones.writer_info.previous;
            interested_it->writer_info.previous->writer_info.next = interested_it;
            tail_old_ones.writer_info.previous = interested_it;
            interested_it->writer_info.next = &tail_old_ones;

            interested_it = next_it;
        }
    }

    //! Head element of interested new changes list to be included.
    //! Should be protected with changes_interested_mutex.
    fastrtps::rtps::CacheChange_t head_new_interested;

    //! Tail element of interested new changes list to be included.
    //! Should be protected with changes_interested_mutex.
    fastrtps::rtps::CacheChange_t tail_new_interested;

    //! Head element of interested old changes list to be included.
    //! Should be protected with changes_interested_mutex.
    fastrtps::rtps::CacheChange_t head_old_interested;

    //! Tail element of interested changes list to be included.
    //! Should be protected with old changes_interested_mutex.
    fastrtps::rtps::CacheChange_t tail_old_interested;

    //! Head element on the queue.
    //! Should be protected with mutex_.
    fastrtps::rtps::CacheChange_t head_new_ones;

    //! Tail element on the queue.
    //! Should be protected with mutex_.
    fastrtps::rtps::CacheChange_t tail_new_ones;

    //! Head element on the queue.
    //! Should be protected with mutex_.
    fastrtps::rtps::CacheChange_t head_old_ones;

    //! Tail element on the queue.
    //! Should be protected with mutex_.
    fastrtps::rtps::CacheChange_t tail_old_ones;
};

/** Classes used to specify FlowController's publication model **/

//! Only sends new samples synchronously. There is no mechanism to send old ones.
struct FlowControllerPureSyncPublishMode
{

    FlowControllerPureSyncPublishMode(
            fastrtps::rtps::RTPSParticipantImpl*,
            const FlowControllerDescriptor*)
    {
    }

};

//! Sends new samples asynchronously. Old samples are sent also asynchronously */
struct FlowControllerAsyncPublishMode
{
    FlowControllerAsyncPublishMode(
            fastrtps::rtps::RTPSParticipantImpl* participant,
            const FlowControllerDescriptor*)
        : group(participant)
    {
    }

    virtual ~FlowControllerAsyncPublishMode()
    {
        if (running)
        {
            {
                std::unique_lock<std::mutex> lock(changes_interested_mutex);
                running = false;
                cv.notify_one();
            }
            thread.join();
        }
    }

    bool fast_check_is_there_slot_for_change(
            fastrtps::rtps::CacheChange_t*) const
    {
        return true;
    }

    void wait(
            std::unique_lock<std::mutex>& lock)
    {
        cv.wait(lock);
    }

    bool force_wait() const
    {
        return false;
    }

    void process_deliver_retcode(
            const fastrtps::rtps::RTPSWriter::DeliveryRetCode&)
    {
    }

    std::thread thread;

    bool running = false;

    std::condition_variable cv;

    fastrtps::rtps::RTPSMessageGroup group;

    //! Mutex for interested samples to be added.
    std::mutex changes_interested_mutex;

    //! Used to warning async thread a writer wants to remove a sample.
    std::atomic<uint32_t> writers_interested_in_remove = {0};
};

//! Sends new samples synchronously. Old samples are sent asynchronously */
struct FlowControllerSyncPublishMode : public FlowControllerPureSyncPublishMode, FlowControllerAsyncPublishMode
{

    FlowControllerSyncPublishMode(
            fastrtps::rtps::RTPSParticipantImpl* participant,
            const FlowControllerDescriptor* descriptor)
        : FlowControllerPureSyncPublishMode(participant, descriptor)
        , FlowControllerAsyncPublishMode(participant, descriptor)
    {
    }

    bool fast_check_is_there_slot_for_change(
            fastrtps::rtps::CacheChange_t*) const
    {
        return true;
    }

};

//! Sends all samples asynchronously but with bandwidth limitation.
struct FlowControllerLimitedAsyncPublishMode : public FlowControllerAsyncPublishMode
{
    FlowControllerLimitedAsyncPublishMode(
            fastrtps::rtps::RTPSParticipantImpl* participant,
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
            fastrtps::rtps::CacheChange_t* change)
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

    void wait(
            std::unique_lock<std::mutex>& lock)
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
    }

    bool force_wait() const
    {
        return force_wait_;
    }

    void process_deliver_retcode(
            const fastrtps::rtps::RTPSWriter::DeliveryRetCode& ret_value)
    {
        if (fastrtps::rtps::RTPSWriter::DeliveryRetCode::EXCEEDED_LIMIT == ret_value)
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
            fastrtps::rtps::RTPSWriter*)
    {
    }

    void unregister_writer(
            fastrtps::rtps::RTPSWriter*)
    {
    }

    bool is_empty() const
    {
        return queue_.is_empty();
    }

    void add_new_sample(
            fastrtps::rtps::RTPSWriter*,
            fastrtps::rtps::CacheChange_t* change)
    {
        queue_.add_new_sample(change);
    }

    void add_old_sample(
            fastrtps::rtps::RTPSWriter*,
            fastrtps::rtps::CacheChange_t* change)
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
    fastrtps::rtps::CacheChange_t* get_next_change_nts()
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

private:

    //! Scheduler queue. FIFO scheduler only has one queue.
    FlowQueue queue_;
};

//! Round Robin scheduling
struct FlowControllerRoundRobinSchedule
{
    FlowControllerRoundRobinSchedule()
    {
        next_writer_ = writers_queue_.begin();
    }

    void register_writer(
            fastrtps::rtps::RTPSWriter* writer)
    {
        fastrtps::rtps::GUID_t current_guid = fastrtps::rtps::GUID_t::unknown();

        if (writers_queue_.end() != next_writer_)
        {
            current_guid = next_writer_->first;
        }

        assert(writers_queue_.end() == writers_queue_.find(writer->getGuid()));
        writers_queue_.emplace( writer->getGuid(), FlowQueue());

        if (fastrtps::rtps::GUID_t::unknown() == current_guid)
        {
            next_writer_ = writers_queue_.begin();
        }
        else
        {
            next_writer_ = writers_queue_.find(current_guid);
        }
    }

    void unregister_writer(
            fastrtps::rtps::RTPSWriter* writer)
    {
        fastrtps::rtps::GUID_t current_guid = fastrtps::rtps::GUID_t::unknown();

        if (writers_queue_.end() != next_writer_)
        {
            current_guid = next_writer_->first;
        }

        auto it = writers_queue_.find(writer->getGuid());
        assert(it != writers_queue_.end());
        assert(it->second.is_empty());
        writers_queue_.erase(it);

        if (fastrtps::rtps::GUID_t::unknown() == current_guid ||
                writer->getGuid() == current_guid)
        {
            next_writer_ = writers_queue_.begin();
        }
        else
        {
            next_writer_ = writers_queue_.find(current_guid);
        }
    }

    bool is_empty() const
    {
        bool ret_value = true;

        for (auto& queue : writers_queue_)
        {
            if (!(ret_value &= queue.second.is_empty()))
            {
                break;
            }
        }

        return ret_value;
    }

    void add_new_sample(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change)
    {
        auto it = writers_queue_.find(writer->getGuid());
        assert(it != writers_queue_.end());
        it->second.add_new_sample(change);
    }

    void add_old_sample(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change)
    {
        auto it = writers_queue_.find(writer->getGuid());
        assert(it != writers_queue_.end());
        it->second.add_old_sample(change);
    }

    fastrtps::rtps::CacheChange_t* get_next_change_nts()
    {
        fastrtps::rtps::CacheChange_t* ret_change = nullptr;

        if (0 < writers_queue_.size())
        {
            auto starting_it = next_writer_; // For avoid loops.

            while (nullptr == ret_change)
            {
                if (writers_queue_.end() == next_writer_)
                {
                    next_writer_ = writers_queue_.begin();
                    continue;
                }

                ret_change = next_writer_->second.get_next_change();
                ++next_writer_;

                if (starting_it == next_writer_)
                {
                    break;
                }
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

private:

    std::map<fastrtps::rtps::GUID_t, FlowQueue> writers_queue_;
    std::map<fastrtps::rtps::GUID_t, FlowQueue>::iterator next_writer_;

};

//! High priority scheduling
struct FlowControllerHighPrioritySchedule {};

//! Priority with reservation scheduling
struct FlowControllerPriorityWithReservation {};

template<typename PublishMode, typename SampleScheduling>
class FlowControllerImpl : public FlowController
{
    using publish_mode = PublishMode;
    using scheduler = SampleScheduling;

public:

    FlowControllerImpl(
            fastrtps::rtps::RTPSParticipantImpl* participant,
            const FlowControllerDescriptor* descriptor
            )
        : participant_(participant)
        , async_mode(participant, descriptor)
    {
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
            fastrtps::rtps::RTPSWriter* writer) override
    {
        std::unique_lock<std::mutex> lock(mutex_);
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
            fastrtps::rtps::RTPSWriter* writer) override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        writers_.erase(writer->getGuid());
        unregister_writer_impl(writer);
    }

    /*
     * Adds the CacheChange_t to be managed by this object.
     * The CacheChange_t has to be a new one, that is, it has to be added to the writer's history before this call.
     * This function should be called by RTPSWriter::unsent_change_added_to_history().
     * This function has two specializations depending on template parameter PublishMode.
     *
     * @param Pointer to the writer which the added CacheChante_t is responsable. Cannot be nullptr.
     * @param change Pointer to the new CacheChange_t to be managed by this object. Cannot be nullptr.
     * @return true if sample could be added. false in other case.
     */
    bool add_new_sample(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change,
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
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change) override
    {
        return add_old_sample_impl(writer, change,
                       std::chrono::steady_clock::now() + std::chrono::hours(24));
    }

    /*!
     * If currently the CacheChange_t is managed by this object, remove it.
     * This funcion should be called when a CacheChange_t is removed from the writer's history.
     *
     * @param Pointer to the change which should be removed if it is currently managed by this object.
     */
    void remove_change(
            fastrtps::rtps::CacheChange_t* change) override
    {
        assert(nullptr != change);
        remove_change_impl(change);
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
        if (false == async_mode.running)
        {
            // Code for initializing the asynchronous thread.
            async_mode.running = true;
            async_mode.thread = std::thread(&FlowControllerImpl::run, this);
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
            fastrtps::rtps::RTPSWriter* writer)
    {
        std::unique_lock<std::mutex> in_lock(async_mode.changes_interested_mutex);
        sched.register_writer(writer);
    }

    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    register_writer_impl(
            fastrtps::rtps::RTPSWriter*)
    {
        // Do nothing. Fail.
    }

    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    unregister_writer_impl(
            fastrtps::rtps::RTPSWriter* writer)
    {
        std::unique_lock<std::mutex> in_lock(async_mode.changes_interested_mutex);
        sched.unregister_writer(writer);
    }

    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    unregister_writer_impl(
            fastrtps::rtps::RTPSWriter*)
    {
        // Do nothing. Fail.
    }

    /*!
     * This function store internally the sample and wake up the async thread.
     *
     * @note Before calling this function, the change's writer mutex have to be locked.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    enqueue_new_sample_impl(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& /* TODO max_blocking_time*/)
    {
        assert(nullptr == change->writer_info.previous &&
                nullptr == change->writer_info.next);
        // Sync delivery failes. Try to store for asynchronous delivery.
        std::unique_lock<std::mutex> lock(async_mode.changes_interested_mutex);
        sched.add_new_sample(writer, change);
        async_mode.cv.notify_one();

        return true;
    }

    /*! This function is used when PublishMode = FlowControllerPureSyncPublishMode.
     *  In this case there is no async mechanism.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    constexpr enqueue_new_sample_impl(
            fastrtps::rtps::RTPSWriter*,
            fastrtps::rtps::CacheChange_t*,
            const std::chrono::time_point<std::chrono::steady_clock>&) const
    {
        // Do nothing. Fail.
        return false;
    }

    /*!
     * This function tries to send the sample synchronously.
     * That is, it uses the user's thread, which is the one calling this function, to send the sample.
     * It calls new function `RTPSWriter::deliver_sample()` for sending the sample.
     * If this function fails (for example because non-blocking socket is full), this function stores internally the sample to
     * try sending it again asynchronously.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_base_of<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    add_new_sample_impl(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
    {
        // This call should be made with writer's mutex locked.
        fastrtps::rtps::RTPSWriter::LocatorSelector& locator_selector = writer->get_general_locator_selector();
        fastrtps::rtps::RTPSMessageGroup group(participant_, writer, &locator_selector);
        if (fastrtps::rtps::RTPSWriter::DeliveryRetCode::DELIVERED !=
                writer->deliver_sample_nts(change, group, locator_selector, max_blocking_time))
        {
            return enqueue_new_sample_impl(writer, change, max_blocking_time);
        }

        return true;
    }

    /*!
     * This function stores internally the sample to send it asynchronously.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_base_of<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    add_new_sample_impl(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change,
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
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& /* TODO max_blocking_time*/)
    {
        // This comparison is thread-safe, because we ensure the change to a problematic state is always protected for
        // its writer's mutex.
        // Problematic states:
        // - Being added: change both pointers from nullptr to a pointer values.
        // - Being removed: change both pointer from pointer values to nullptr.
        if (nullptr == change->writer_info.previous &&
                nullptr == change->writer_info.next)
        {
            std::unique_lock<std::mutex> lock(async_mode.changes_interested_mutex);
            sched.add_old_sample(writer, change);
            async_mode.cv.notify_one();

            return true;
        }

        return false;
    }

    /*! This function is used when PublishMode = FlowControllerPureSyncPublishMode.
     *  In this case there is no async mechanism.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, bool>::type
    constexpr add_old_sample_impl(
            fastrtps::rtps::RTPSWriter*,
            fastrtps::rtps::CacheChange_t*,
            const std::chrono::time_point<std::chrono::steady_clock>&) const
    {
        // Do nothing. Fail.
        return false;
    }

    /*!
     * This function store internally the sample and wake up the async thread.
     *
     * @note Before calling this function, the change's writer mutex have to be locked.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    remove_change_impl(
            fastrtps::rtps::CacheChange_t* change)
    {
        // This comparison is thread-safe, because we ensure the change to a problematic state is always protected for
        // its writer's mutex.
        // Problematic states:
        // - Being added: change both pointers from nullptr to a pointer values.
        // - Being removed: change both pointer from pointer values to nullptr.
        if (nullptr != change->writer_info.previous ||
                nullptr != change->writer_info.next)
        {
            ++async_mode.writers_interested_in_remove;
            std::unique_lock<std::mutex> lock(mutex_);
            std::unique_lock<std::mutex> interested_lock(async_mode.changes_interested_mutex);

            // When blocked, both pointer are different than nullptr or equal.
            assert((nullptr != change->writer_info.previous &&
                    nullptr != change->writer_info.next) ||
                    (nullptr == change->writer_info.previous &&
                    nullptr == change->writer_info.next));
            if (nullptr != change->writer_info.previous &&
                    nullptr != change->writer_info.next)
            {

                // Try to join previous node and next node.
                change->writer_info.previous->writer_info.next = change->writer_info.next;
                change->writer_info.next->writer_info.previous = change->writer_info.previous;
                change->writer_info.previous = nullptr;
                change->writer_info.next = nullptr;
            }
            --async_mode.writers_interested_in_remove;
        }
    }

    /*! This function is used when PublishMode = FlowControllerPureSyncPublishMode.
     *  In this case there is no async mechanism.
     */
    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_same<FlowControllerPureSyncPublishMode, PubMode>::value, void>::type
    remove_change_impl(
            fastrtps::rtps::CacheChange_t*) const
    {
        // Do nothing. Fail.
    }

    /*!
     * Function ran by asynchronous thread.
     */
    void run()
    {
        while (async_mode.running)
        {
            // There is writers interested in remove a sample.
            if (0 != async_mode.writers_interested_in_remove)
            {
                continue;
            }

            std::unique_lock<std::mutex> lock(mutex_);

            //Check if we have to sleep.
            {
                std::unique_lock<std::mutex> in_lock(async_mode.changes_interested_mutex);
                // Add interested changes into the queue.
                sched.add_interested_changes_to_queue_nts();

                while (async_mode.running &&
                        (sched.is_empty() || async_mode.force_wait()))
                {
                    lock.unlock();
                    async_mode.wait(in_lock);

                    in_lock.unlock();
                    lock.lock();
                    in_lock.lock();

                    sched.add_interested_changes_to_queue_nts();
                }

            }

            fastrtps::rtps::RTPSWriter* current_writer = nullptr;
            fastrtps::rtps::CacheChange_t* change_to_process = nullptr;
            while (nullptr != (change_to_process = sched.get_next_change_nts()))
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

                if (!try_lock(current_writer))
                {
                    break;
                }

                fastrtps::rtps::RTPSWriter::LocatorSelector& locator_selector =
                        current_writer->get_async_locator_selector();
                async_mode.group.change_transmitter(current_writer, &locator_selector);

                // Remove previously from queue, because deliver_sample_nts could call FlowController::remove_sample()
                // provoking a deadlock.
                fastrtps::rtps::CacheChange_t* previous = change_to_process->writer_info.previous;
                fastrtps::rtps::CacheChange_t* next = change_to_process->writer_info.next;
                previous->writer_info.next = next;
                next->writer_info.previous = previous;
                change_to_process->writer_info.previous = nullptr;
                change_to_process->writer_info.next = nullptr;

                fastrtps::rtps::RTPSWriter::DeliveryRetCode ret_delivery = current_writer->deliver_sample_nts(
                    change_to_process, async_mode.group, locator_selector,
                    std::chrono::steady_clock::now() + std::chrono::hours(24));

                if (fastrtps::rtps::RTPSWriter::DeliveryRetCode::DELIVERED != ret_delivery)
                {
                    // If delivery fails, put the change again in the queue.
                    previous->writer_info.next = change_to_process;
                    next->writer_info.previous = change_to_process;
                    change_to_process->writer_info.previous = previous;
                    change_to_process->writer_info.next = next;

                    async_mode.process_deliver_retcode(ret_delivery);

                    unlock(current_writer);
                    // Unlock mutex_ and try again.
                    break;
                }

                unlock(current_writer);

                if (0 != async_mode.writers_interested_in_remove)
                {
                    // There are writers that want to remove samples.
                    break;
                }

                // Add interested changes into the queue.
                std::unique_lock<std::mutex> in_lock(async_mode.changes_interested_mutex);
                sched.add_interested_changes_to_queue_nts();
            }

            async_mode.group.change_transmitter(nullptr, nullptr);
        }
    }

    template<typename PubMode = PublishMode>
    typename std::enable_if<std::is_same<FlowControllerLimitedAsyncPublishMode, PubMode>::value, uint32_t>::type
    get_max_payload_impl()
    {
        return async_mode.max_bytes_per_period;
    }

    template<typename PubMode = PublishMode>
    typename std::enable_if<!std::is_same<FlowControllerLimitedAsyncPublishMode, PubMode>::value, uint32_t>::type
    constexpr get_max_payload_impl() const
    {
        return std::numeric_limits<uint32_t>::max();
    }

    std::mutex mutex_;

    fastrtps::rtps::RTPSParticipantImpl* participant_ = nullptr;

    std::map<fastrtps::rtps::GUID_t, fastrtps::rtps::RTPSWriter*> writers_;

    scheduler sched;

    publish_mode async_mode;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _RTPS_FLOWCONTROL_FLOWCONTROLLERIMPL_HPP_
