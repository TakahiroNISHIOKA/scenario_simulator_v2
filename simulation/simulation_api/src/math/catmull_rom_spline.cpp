// Copyright 2015-2020 TierIV.inc. All rights reserved.
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

#include <simulation_api/math/catmull_rom_spline.hpp>

#include <vector>
#include <string>
#include <limits>
#include <utility>

namespace simulation_api
{
namespace math
{
CatmullRomSpline::CatmullRomSpline(std::vector<geometry_msgs::msg::Point> control_points)
: control_points(control_points)
{
  size_t n = control_points.size() - 1;
  if (n <= 1) {
    throw SplineInterpolationError("numbers of control points are not enough.");
  }
  for (size_t i = 0; i < n; i++) {
    if (i == 0) {
      double ax = 0;
      double bx = control_points[0].x - 2 * control_points[1].x + control_points[2].x;
      double cx = -3 * control_points[0].x + 4 * control_points[1].x - control_points[2].x;
      double dx = 2 * control_points[0].x;
      double ay = 0;
      double by = control_points[0].y - 2 * control_points[1].y + control_points[2].y;
      double cy = -3 * control_points[0].y + 4 * control_points[1].y - control_points[2].y;
      double dy = 2 * control_points[0].y;
      double az = 0;
      double bz = control_points[0].z - 2 * control_points[1].z + control_points[2].z;
      double cz = -3 * control_points[0].z + 4 * control_points[1].z - control_points[2].z;
      double dz = 2 * control_points[0].z;
      ax = ax * 0.5;
      bx = bx * 0.5;
      cx = cx * 0.5;
      dx = dx * 0.5;
      ay = ay * 0.5;
      by = by * 0.5;
      cy = cy * 0.5;
      dy = dy * 0.5;
      az = az * 0.5;
      bz = bz * 0.5;
      cz = cz * 0.5;
      dz = dz * 0.5;
      curves_.emplace_back(HermiteCurve(ax, bx, cx, dx, ay, by, cy, dy, az, bz, cz, dz));
    } else if (i == (n - 1)) {
      double ax = 0;
      double bx = control_points[i - 1].x - 2 * control_points[i].x + control_points[i + 1].x;
      double cx = -1 * control_points[i - 1].x + control_points[i + 1].x;
      double dx = 2 * control_points[i].x;
      double ay = 0;
      double by = control_points[i - 1].y - 2 * control_points[i].y + control_points[i + 1].y;
      double cy = -1 * control_points[i - 1].y + control_points[i + 1].y;
      double dy = 2 * control_points[i].y;
      double az = 0;
      double bz = control_points[i - 1].z - 2 * control_points[i].z + control_points[i + 1].z;
      double cz = -1 * control_points[i - 1].z + control_points[i + 1].z;
      double dz = 2 * control_points[i].z;
      ax = ax * 0.5;
      bx = bx * 0.5;
      cx = cx * 0.5;
      dx = dx * 0.5;
      ay = ay * 0.5;
      by = by * 0.5;
      cy = cy * 0.5;
      dy = dy * 0.5;
      az = az * 0.5;
      bz = bz * 0.5;
      cz = cz * 0.5;
      dz = dz * 0.5;
      curves_.emplace_back(HermiteCurve(ax, bx, cx, dx, ay, by, cy, dy, az, bz, cz, dz));
    } else {
      double ax = -1 * control_points[i - 1].x + 3 * control_points[i].x - 3 *
        control_points[i + 1].x + control_points[i + 2].x;
      double bx = 2 * control_points[i - 1].x - 5 * control_points[i].x + 4 *
        control_points[i + 1].x - control_points[i + 2].x;
      double cx = -control_points[i - 1].x + control_points[i + 1].x;
      double dx = 2 * control_points[i].x;
      double ay = -1 * control_points[i - 1].y + 3 * control_points[i].y - 3 *
        control_points[i + 1].y + control_points[i + 2].y;
      double by = 2 * control_points[i - 1].y - 5 * control_points[i].y + 4 *
        control_points[i + 1].y - control_points[i + 2].y;
      double cy = -control_points[i - 1].y + control_points[i + 1].y;
      double dy = 2 * control_points[i].y;
      double az = -1 * control_points[i - 1].z + 3 * control_points[i].z - 3 *
        control_points[i + 1].z + control_points[i + 2].z;
      double bz = 2 * control_points[i - 1].z - 5 * control_points[i].z + 4 *
        control_points[i + 1].z - control_points[i + 2].z;
      double cz = -control_points[i - 1].z + control_points[i + 1].z;
      double dz = 2 * control_points[i].z;
      ax = ax * 0.5;
      bx = bx * 0.5;
      cx = cx * 0.5;
      dx = dx * 0.5;
      ay = ay * 0.5;
      by = by * 0.5;
      cy = cy * 0.5;
      dy = dy * 0.5;
      az = az * 0.5;
      bz = bz * 0.5;
      cz = cz * 0.5;
      dz = dz * 0.5;
      curves_.emplace_back(HermiteCurve(ax, bx, cx, dx, ay, by, cy, dy, az, bz, cz, dz));
    }
  }
  for (const auto & curve : curves_) {
    length_list_.emplace_back(curve.getLength());
    maximum_2d_curvatures_.emplace_back(curve.getMaximu2DCurvature());
  }
  total_length_ = 0;
  for (const auto & length : length_list_) {
    total_length_ = total_length_ + length;
  }
  checkConnection();
}

std::pair<size_t, double> CatmullRomSpline::getCurveIndexAndS(double s) const
{
  if (s < 0) {
    return std::make_pair(0, s);
  }
  if (s > total_length_) {
    return std::make_pair(curves_.size() - 1, s - total_length_);
  }
  double current_s = 0;
  for (size_t i = 0; i < curves_.size(); i++) {
    double prev_s = current_s;
    current_s = current_s + length_list_[i];
    if (prev_s <= s && s < current_s) {
      return std::make_pair(i, s - prev_s);
    }
  }
  throw SplineInterpolationError("failed to calculate curve index");
}

const geometry_msgs::msg::Point CatmullRomSpline::getPoint(double s) const
{
  const auto index_and_s = getCurveIndexAndS(s);
  return curves_[index_and_s.first].getPoint(index_and_s.second);
}

double CatmullRomSpline::getMaximum2DCurventure() const
{
  if (maximum_2d_curvatures_.size() == 0) {
    throw SplineInterpolationError("maximum 2D curventure vector size is 0.");
  }
  return *std::max_element(maximum_2d_curvatures_.begin(), maximum_2d_curvatures_.end());
}

bool CatmullRomSpline::checkConnection() const
{
  if (control_points.size() != (curves_.size() + 1)) {
    throw SplineInterpolationError("number of control points and curves does not match.");
  }
  for (size_t i = 0; i < curves_.size(); i++) {
    const auto control_point0 = control_points[i];
    const auto control_point1 = control_points[i + 1];
    const auto p0 = curves_[i].getPoint(0, false);
    const auto p1 = curves_[i].getPoint(1, false);
    if (equals(control_point0, p0) && equals(control_point1, p1)) {
      continue;
    } else if (!equals(control_point0, p0)) {
      throw SplineInterpolationError("start point of the curve number " + std::to_string(
                i) + " does not match.");
    } else if (!equals(control_point1, p1)) {
      throw SplineInterpolationError("end point of the curve number " + std::to_string(
                i) + " does not match.");
    }
  }
  if (curves_.size() == 0) {
    throw SplineInterpolationError("curve size should not be zero.");
  }
  return true;
}

bool CatmullRomSpline::equals(geometry_msgs::msg::Point p0, geometry_msgs::msg::Point p1) const
{
  constexpr double e = std::numeric_limits<double>::epsilon();
  if (std::abs(p0.x - p1.x) > e) {
    return false;
  }
  if (std::abs(p0.y - p1.y) > e) {
    return false;
  }
  if (std::abs(p0.z - p1.z) > e) {
    return false;
  }
  return true;
}
}  // namespace math
}  // namespace simulation_api
