if(NOT ((NSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER) AND fastcdr_FOUND)
	include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)
	check_gtest()

	if(GTEST_FOUND)
		if(WIN32)
			add_definitions(-D_WIN32_WINNT=0x0501)
		endif()

		set(ROS2FEATURES_SOURCE ros2feature_test.cpp
			types/HelloWorldType.cpp
			types/HelloWorld.cpp
			)
		add_executable(ROS2FEATURETEST ${ROS2FEATURES_SOURCE})
		add_gtest(ROS2FEATURETEST ${ROS2FEATURES_SOURCE})
		target_include_directories(ROS2FEATURETEST PRIVATE ${Boost_INCLUDE_DIR} ${GTEST_INCLUDE_DIRS})
		target_link_libraries(ROS2FEATURETEST fastrtps fastcdr ${GTEST_LIBRARIES})
	endif()
endif()
