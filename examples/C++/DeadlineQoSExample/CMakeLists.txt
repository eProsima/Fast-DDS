if(NOT (MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER) AND fastcdr_FOUND)
	message(STATUS "Building DeadlineQoS example...")
	file(GLOB DLQOS_SOURCES "*.cxx")
	
	add_executable(deadlineqosexample ${DLQOS_SOURCES})
	target_include_directories(deadlineqosexample PRIVATE ${Boost_INCLUDE_DIR})
	target_link_libraries(deadlineqosexample fastrtps fastcdr ${Boost_LIBRARIES})
endif()
