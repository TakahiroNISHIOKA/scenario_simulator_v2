// Copyright 2015-2020 Tier IV, Inc. All rights reserved.
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

#include <quaternion_operation/quaternion_operation.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

#include <traffic_simulator/math/transform.hpp>

namespace traffic_simulator
{
namespace math
{
const geometry_msgs::msg::Pose getRelativePose(
  const geometry_msgs::msg::Pose & from, const geometry_msgs::msg::Pose & to)
{
  geometry_msgs::msg::Transform from_translation;
  {
    from_translation.translation.x = from.position.x;
    from_translation.translation.y = from.position.y;
    from_translation.translation.z = from.position.z;
    from_translation.rotation = from.orientation;
  }

  tf2::Transform from_tf;
  {
    tf2::fromMsg(from_translation, from_tf);
  }

  geometry_msgs::msg::Transform to_translation;
  {
    to_translation.translation.x = to.position.x;
    to_translation.translation.y = to.position.y;
    to_translation.translation.z = to.position.z;
    to_translation.rotation = to.orientation;
  }

  tf2::Transform to_tf;
  {
    tf2::fromMsg(to_translation, to_tf);
  }

  tf2::Transform tf_delta = from_tf.inverse() * to_tf;

  geometry_msgs::msg::Pose ret;
  {
    tf2::toMsg(tf_delta, ret);
  }

  return ret;
}

const geometry_msgs::msg::Point transformPoint(
  const geometry_msgs::msg::Pose & pose, const geometry_msgs::msg::Point & point)
{
  auto mat = quaternion_operation::getRotationMatrix(pose.orientation);
  Eigen::VectorXd v(3);
  v(0) = point.x;
  v(1) = point.y;
  v(2) = point.z;
  v = mat * v;
  v(0) = v(0) + pose.position.x;
  v(1) = v(1) + pose.position.y;
  v(2) = v(2) + pose.position.z;
  geometry_msgs::msg::Point transformed;
  transformed.x = v(0);
  transformed.y = v(1);
  transformed.z = v(2);
  return transformed;
}

std::vector<geometry_msgs::msg::Point> transformPoints(
  const geometry_msgs::msg::Pose & pose, const std::vector<geometry_msgs::msg::Point> & points)
{
  std::vector<geometry_msgs::msg::Point> ret;
  for (const auto & point : points) {
    ret.emplace_back(transformPoint(pose, point));
  }
  return ret;
}
}  // namespace math
}  // namespace traffic_simulator
