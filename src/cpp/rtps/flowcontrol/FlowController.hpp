#ifndef _RTPS_FLOWCONTROL_FLOWCONTROLLER_HPP_
#define _RTPS_FLOWCONTROL_FLOWCONTROLLER_HPP_

#include <chrono>

namespace eprosima {
namespace fastdds {
namespace rtps {

class BaseWriter;
struct CacheChange_t;

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
     * This object will only manage a CacheChange_t if the corresponding writer was previously registered with this method.
     *
     * @param writer Pointer to the writer to be registered. Cannot be nullptr.
     */
    virtual void register_writer(
            BaseWriter* writer) = 0;

    /*!
     * Unregister a writer.
     *
     * @pre Writer must have removed all its CacheChange_t from this object.
     * @param writer Pointer to the writer to be unregistered. Cannot be nullptr.
     */
    virtual void unregister_writer(
            BaseWriter* writer) = 0;

    /*!
     * Adds a CacheChange_t to be managed by this object.
     * The CacheChange_t has to be a new one, that is, it should have just been added to the writer's history before this call.
     * This method should be called by BaseWriter::unsent_change_added_to_history().
     *
     * @param Pointer to the writer that owns the added CacheChange_t. Cannot be nullptr.
     * @param change Pointer to the new CacheChange_t to be managed by this object. Cannot be nullptr.
     * @param max_blocking_time Maximum time this method has to complete the task.
     * @return true if the sample could be added. false otherwise.
     */
    virtual bool add_new_sample(
            BaseWriter* writer,
            CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) = 0;

    /*!
     * Adds a CacheChange_t to be managed by this object.
     * The CacheChange_t has to be an old one, that is, it was already in the writer's history and for some reason has to
     * be sent again.
     *
     * @param Pointer to the writer that owns the added change. Cannot be nullptr.
     * @param change Pointer to the old change to be managed by this object. Cannot be nullptr.
     * @return true if the sample could be added. false otherwise.
     */
    virtual bool add_old_sample(
            BaseWriter* writer,
            CacheChange_t* change) = 0;

    /*!
     * If the CacheChange_t is currently managed by this object, remove it.
     * This method should be called whenever a CacheChange_t is removed from the writer's history.
     *
     * @param [in] change Pointer to the change which should be removed if it is currently managed by this object.
     * @param [in] max_blocking_time Maximum time this method has to complete the task.
     * @return true if the sample could be removed. false otherwise.
     */
    virtual bool remove_change(
            CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) = 0;

    /*!
     * Return the maximum number of bytes can be used by the flow controller to generate a RTPS message.
     *
     * @return Maximum number of bytes of a RTPS message.
     */
    virtual uint32_t get_max_payload() = 0;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _RTPS_FLOWCONTROL_FLOWCONTROLLER_HPP_
