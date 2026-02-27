#ifndef _RTPS_FLOWCONTROL_FLOWCONTROLLERFACTORY_HPP_
#define _RTPS_FLOWCONTROL_FLOWCONTROLLERFACTORY_HPP_

#include <fastdds/rtps/flowcontrol/FlowControllerDescriptor.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include "FlowController.hpp"

#include <string>
#include <map>

namespace eprosima {

namespace fastdds {
namespace rtps {
class RTPSParticipantImpl;
class FlowController;

const char* const pure_sync_flow_controller_name = "PureSyncFlowController";
const char* const sync_flow_controller_name = "SyncFlowController";
const char* const async_flow_controller_name = "AsyncFlowController";
#ifdef FASTDDS_STATISTICS
const char* const async_statistics_flow_controller_name = "AsyncStatisticsFlowController";
#endif // ifndef FASTDDS_STATISTICS

/*!
 * Factory of flow controllers.
 *
 * @note Non-safe thread
 */
class FlowControllerFactory
{
public:

    /*!
     * Initialize the factory.
     * In charge of creating default flow controllers.
     * Call always before use it.
     *
     * @param participant Pointer to the participant owner of this object.
     */
    virtual void init(
            fastdds::rtps::RTPSParticipantImpl* participant);

    template<typename FlowControllerType, typename ... Args>
    FlowControllerType* register_flow_controller_type(
            const std::string& flow_controller_name,
            Args&&... args)
    {
        if (flow_controllers_.find(flow_controller_name) != flow_controllers_.end())
        {
            // Flow controller with that name already exists.
            return nullptr;
        }

        FlowControllerType* ret_val =
                new FlowControllerType(participant_, async_controller_index_++, std::forward<Args>(args)...);
        flow_controllers_.insert(
            decltype(flow_controllers_)::value_type(
                flow_controller_name,
                std::unique_ptr<FlowController>(ret_val)));
        return ret_val;
    }

    /*!
     * Registers a new flow controller.
     * This function should be used by the participant.
     *
     * @param flow_controller_descr FlowController descriptor.
     */
    void register_flow_controller(
            const FlowControllerDescriptor& flow_controller_descr);

    /*!
     * Get a FlowController given its name.
     *
     * @param flow_controller_name Name of the interested FlowController.
     * @return Pointer to the FlowController. nullptr if no registered FlowController with that name.
     */
    FlowController* retrieve_flow_controller(
            const std::string& flow_controller_name,
            const fastdds::rtps::WriterAttributes& writer_attributes);

protected:

    fastdds::rtps::RTPSParticipantImpl* participant_ = nullptr;

    //! Stores the created flow controllers.
    std::map<std::string, std::unique_ptr<FlowController>> flow_controllers_;

    //! Counter used for thread identification
    uint32_t async_controller_index_ = 0;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _RTPS_FLOWCONTROL_FLOWCONTROLLERFACTORY_HPP_
