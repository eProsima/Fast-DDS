# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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
