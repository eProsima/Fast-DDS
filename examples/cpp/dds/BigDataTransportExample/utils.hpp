// Copyright 2022 Open Source Robotics Foundation, Inc.
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

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <algorithm>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>
#include <typeinfo>
#include <cstdint>

#include "datatypes.h"

namespace dds_types_bigdata
{

// Modified from:
// https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
std::string random_string(size_t length)
{
  auto randchar = []() -> char
    {
      const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "`~!@#$%^&*()_+"
        "[]\\;',./{}|:\"<>?";
      const size_t max_index = (sizeof(charset) - 1);
      return charset[rand() % max_index];
    };
  std::string str(length, 0);
  std::generate_n(str.begin(), length, randchar);
  return str;
}

template<typename T,
  typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
T random_number(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dist(min, max);

  return dist(gen);
}

template<typename T,
  typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
std::vector<T> random_number_vector(
  size_t length,
  T min = std::numeric_limits<T>::min(),
  T max = std::numeric_limits<T>::max())
{
  auto num = static_cast<T>(random_number<T>(min, max));
  auto randnum = [&]()
    {
      return num;
    };

  std::vector<T> out(length);
  std::generate(out.begin(), out.end(), randnum);

  return out;
}

bool random_bool()
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(0, 1);

  return dist(gen) == 1;
}


// MSG GEN =========================================================================================
Header random_header(size_t len = 16)
{
  Header header_msg;
  header_msg.sec(random_number<int32_t>());
  header_msg.nanosec(random_number<uint32_t>());
  header_msg.frame_id(random_string(len));

  return header_msg;
}

/**
Quaternion random_quaternion()
{
  Quaternion q_msg;

  q_msg.x(dds_types_bigdata::random_number<double>());
  q_msg.y(dds_types_bigdata::random_number<double>());
  q_msg.z(dds_types_bigdata::random_number<double>());
  q_msg.w(dds_types_bigdata::random_number<double>());

  return q_msg;
}

Point random_point()
{
  Point p_msg;

  p_msg.x(dds_types_bigdata::random_number<double>());
  p_msg.y(dds_types_bigdata::random_number<double>());
  p_msg.z(dds_types_bigdata::random_number<double>());

  return p_msg;
}

Pose random_pose()
{
  Pose pose_msg;

  pose_msg.position(dds_types_bigdata::random_point());
  pose_msg.orientation(dds_types_bigdata::random_quaternion());

  return pose_msg;
}

Vector3 random_vector3()
{
  Vector3 vec_msg;

  vec_msg.x(dds_types_bigdata::random_number<double>());
  vec_msg.y(dds_types_bigdata::random_number<double>());
  vec_msg.z(dds_types_bigdata::random_number<double>());

  return vec_msg;
}

Vector3Stamped random_vector3stamped()
{
  Vector3Stamped vec_msg;

  vec_msg.header_value(dds_types_bigdata::random_header());
  vec_msg.vector(dds_types_bigdata::random_vector3());

  return vec_msg;
}

Twist random_twist()
{
  Twist twist_msg;

  twist_msg.linear(dds_types_bigdata::random_vector3());
  twist_msg.angular(dds_types_bigdata::random_vector3());

  return twist_msg;
}

TwistWithCovariance random_twistwithcovariance(size_t len = 36)
{
  TwistWithCovariance twist_msg;

  twist_msg.twist_value(dds_types_bigdata::random_twist());
  twist_msg.covariance(dds_types_bigdata::random_number_vector<double>(len));

  return twist_msg;
}

TwistWithCovarianceStamped random_twistwithcovariancestamped(size_t len = 36)
{
  TwistWithCovarianceStamped twist_msg;

  twist_msg.header_value(dds_types_bigdata::random_header());
  twist_msg.twist_tc(dds_types_bigdata::random_twistwithcovariance(len));

  return twist_msg;
}

Wrench random_wrench()
{
  Wrench wrench_msg;

  wrench_msg.force(dds_types_bigdata::random_vector3());
  wrench_msg.torque(dds_types_bigdata::random_vector3());

  return wrench_msg;
}

WrenchStamped random_wrenchstamped()
{
  WrenchStamped wrench_msg;

  wrench_msg.header_value(dds_types_bigdata::random_header());
  wrench_msg.wrench_value(dds_types_bigdata::random_wrench());

  return wrench_msg;
}
**/

Image random_image(size_t len = 0)  // 1920 * 1080 * 3
{
  Image image_msg;

  image_msg.header_value(dds_types_bigdata::random_header());

  image_msg.height(dds_types_bigdata::random_number<uint32_t>());
  image_msg.width(dds_types_bigdata::random_number<uint32_t>());

  image_msg.encoding(dds_types_bigdata::random_string(32));
  image_msg.is_bigendian(dds_types_bigdata::random_number<int>(0, 1));
  image_msg.step(dds_types_bigdata::random_number<uint32_t>());

  image_msg.data(dds_types_bigdata::random_number_vector<uint8_t>(len));

  return image_msg;
}

PointField random_pointfield(size_t len = 32)
{
  PointField pt_msg;

  pt_msg.name(dds_types_bigdata::random_string(len));
  pt_msg.offset(dds_types_bigdata::random_number<uint32_t>());
  pt_msg.datatype(dds_types_bigdata::random_number<uint8_t>());
  pt_msg.count(dds_types_bigdata::random_number<uint32_t>(0, 1));

  return pt_msg;
}

PointCloud2 random_pointcloud(size_t len = 0)  // 4 * 4 * 4 * 1280 * 960
{
  PointCloud2 pc_msg;

  pc_msg.header_value(dds_types_bigdata::random_header());

  pc_msg.height(dds_types_bigdata::random_number<uint32_t>());
  pc_msg.width(dds_types_bigdata::random_number<uint32_t>());

  std::vector<PointField> pts(3);
  for (int i = 0; i < 3; ++i) {
    pts.push_back(random_pointfield());
  }

  pc_msg.fields(pts);

  pc_msg.is_bigendian(dds_types_bigdata::random_number<int>(0, 1));
  pc_msg.point_step(dds_types_bigdata::random_number<uint32_t>());
  pc_msg.row_step(dds_types_bigdata::random_number<uint32_t>());

  pc_msg.data(dds_types_bigdata::random_number_vector<uint8_t>(len));

  pc_msg.is_dense(dds_types_bigdata::random_number<int>(0, 1));

  return pc_msg;
}

/**
LaserScan random_laserscan(size_t len = 1024)
{
  LaserScan scan_msg;

  scan_msg.header_value(dds_types_bigdata::random_header());

  scan_msg.angle_min(dds_types_bigdata::random_number<float>());
  scan_msg.angle_max(dds_types_bigdata::random_number<float>());
  scan_msg.angle_increment(dds_types_bigdata::random_number<float>());
  scan_msg.time_increment(dds_types_bigdata::random_number<float>());
  scan_msg.scan_time(dds_types_bigdata::random_number<float>());
  scan_msg.range_min(dds_types_bigdata::random_number<float>());
  scan_msg.range_max(dds_types_bigdata::random_number<float>());

  scan_msg.ranges(dds_types_bigdata::random_number_vector<float>(len));
  scan_msg.intensities(dds_types_bigdata::random_number_vector<float>(len));

  return scan_msg;
}

BasicTypes random_basictypes()
{
  BasicTypes data;

  data.bool_value(random_bool());
  data.int32_value(random_number<int32_t>());
  data.uint32_value(random_number<uint32_t>());
  data.int64_value(random_number<int64_t>());
  data.uint64_value(random_number<uint64_t>());
  data.float_value(random_number<float>());
  data.double_value(random_number<double>());
  data.string_value(random_string(128));

  return data;
}
**/

BigData random_bigdata(bool include_image_and_pointcloud = true)
{
  BigData data;

  std::cout << "Generating small values" << std::endl;
  data.timestamp_value();

  if (include_image_and_pointcloud) {
    std::cout << "Generating image" << std::endl;
    data.image_value(random_image(10000)); // Insert size of data in bytes
    std::cout << "Generating point cloud" << std::endl;
    data.point_cloud_value(random_pointcloud(10));
  } else {
    data.image_value(random_image(0));
    data.point_cloud_value(random_pointcloud(0));
  }

  return data;
}

void set_bigdata_timestamp_to_now(BigData &big_data)
{
  auto time_since_epoch = std::chrono::high_resolution_clock::now().time_since_epoch();

  big_data.timestamp_value().sec(
    std::chrono::duration_cast<std::chrono::seconds>(time_since_epoch).count());
  big_data.timestamp_value().nanosec(
    std::chrono::duration_cast<std::chrono::nanoseconds>(time_since_epoch).count() % 1000000000);
}

}  // namespace dds_types_bigdata

#endif  // UTILS_HPP_
