#ifndef _RTPS_FLOWCONTROL_FLOWCONTROLLER_HPP_
#define _RTPS_FLOWCONTROL_FLOWCONTROLLER_HPP_

#include <chrono>

namespace eprosima {

namespace fastrtps {
namespace rtps {
class RTPSWriter;
struct CacheChange_t;
} // namespace rtps
} // namespace fastrtps

namespace fastdds {
namespace rtps {

/*!
 * Interface used by writers to control the usage of network bandwidth.
 */
class FlowController
{
public:

    virtual ~FlowController()
    {
    }

    /*!
     * Initializes the flow controller.
     */
    virtual void init() = 0;

    /*!
     * Registers a writer.
     * This object is only be able to manage a CacheChante_t if its writer was registered previously with this function.
     *
     * @param writer Pointer to the writer to be registered. Cannot be nullptr.
     */
    virtual void register_writer(
            fastrtps::rtps::RTPSWriter* writer) = 0;

    /*!
     * Unregister a writer.
     *
     * @param writer Pointer to the writer to be unregistered. Cannot be nullptr.
     */
    virtual void unregister_writer(
            fastrtps::rtps::RTPSWriter* writer) = 0;

    /*!
     * Adds the CacheChange_t to be managed by this object.
     * The CacheChange_t has to be a new one, that is, it has to be added to the writer's history before this call.
     * This function should be called by RTPSWriter::unsent_change_added_to_history().
     *
     * @param Pointer to the writer that is responsable of the added CacheChante_t. Cannot be nullptr.
     * @param change Pointer to the new CacheChange_t to be managed by this object. Cannot be nullptr.
     * @param max_blocking_time Maximum time the funcion has to complete the task.
     * @return true if sample could be added. false in other case.
     */
    virtual bool add_new_sample(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) = 0;

    /*!
     * Adds the CacheChante_t to be managed by this object.
     * The CacheChange_t has to be an old one, that is, it is already in the writer's history and for some reason has to
     * be sent again.
     *
     * @param Pointer to the writer that is responsible of the added change. Cannot be nullptr.
     * @param change Pointer to the old change to be managed by this object. Cannot be nullptr.
     * @return true if sample could be added. false in other case.
     */
    virtual bool add_old_sample(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::CacheChange_t* change) = 0;

    /*!
     * If currently the CacheChange_t is managed by this object, remove it.
     * This funcion should be called when a CacheChange_t is removed from the writer's history.
     *
     * @param Pointer to the change which should be removed if it is currently managed by this object.
     */
    virtual void remove_change(
            fastrtps::rtps::CacheChange_t* change) = 0;

    virtual uint32_t get_max_payload() = 0;

    bool try_lock(
            fastrtps::rtps::RTPSWriter* writer);

    void unlock(
            fastrtps::rtps::RTPSWriter* writer);
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _RTPS_FLOWCONTROL_FLOWCONTROLLER_HPP_
