if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER) AND fastcdr_FOUND)
	message(STATUS "Building OwnershipStrength example...")
	file(GLOB DLQOS_SOURCES "*.cxx")
	
	add_executable(ownershipstrengthtest ${DLQOS_SOURCES})
	target_include_directories(ownershipstrengthtest PRIVATE ${Boost_INCLUDE_DIR})
	target_link_libraries(ownershipstrengthtest fastrtps fastcdr ${Boost_LIBRARIES})
endif()
