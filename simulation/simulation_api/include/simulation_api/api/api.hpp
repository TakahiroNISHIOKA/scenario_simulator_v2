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

#ifndef SIMULATION_API__API__API_HPP_
#define SIMULATION_API__API__API_HPP_

#include <simulation_api/entity/entity_manager.hpp>
#include <simulation_api/helper/helper.hpp>
#include <simulation_api/traffic_lights/traffic_light.hpp>
#include <simulation_api/metrics/metrics_manager.hpp>

#include <awapi_accessor/accessor.hpp>

#include <openscenario_msgs/msg/driver_model.hpp>

#include <autoware_auto_msgs/msg/vehicle_control_command.hpp>
#include <autoware_auto_msgs/msg/vehicle_state_command.hpp>
#include <rclcpp/rclcpp.hpp>
#include <xmlrpcpp/XmlRpcClient.h>
#include <xmlrpcpp/XmlRpcValue.h>
#include <xmlrpcpp/XmlRpcException.h>

#include <memory>
#include <string>
#include <utility>

namespace scenario_simulator
{
class XmlRpcRuntimeError : public std::runtime_error
{
public:
  XmlRpcRuntimeError(const char * message, int result)
  : runtime_error(message), error_info_(result) {}

private:
  int error_info_;
};

class ExecutionFailedError : public std::runtime_error
{
public:
  explicit ExecutionFailedError(XmlRpc::XmlRpcValue value)
  : runtime_error(value["message"]) {}
  explicit ExecutionFailedError(const char * message)
  : runtime_error(message) {}
};

class API
{
  using EntityManager = simulation_api::entity::EntityManager;

  std::unique_ptr<autoware_api::Accessor> access_rights_;

public:
  template<class NodeT, class AllocatorT = std::allocator<void>>
  explicit API(
    NodeT && node, const std::string & map_path = "",
    bool verbose = false, const std::string metrics_logfile_path = "/tmp/metrics.json",
    const rclcpp::SubscriptionOptionsWithAllocator<AllocatorT> & options =
    rclcpp::SubscriptionOptionsWithAllocator<AllocatorT>())
  : access_rights_(std::make_unique<decltype(access_rights_)::element_type>(node)),
    metrics_manager_(verbose, metrics_logfile_path)
  {
    std::string address = "127.0.0.1";

    int port = 8080;

    node->declare_parameter("port", port);
    node->get_parameter("port", port);
    node->undeclare_parameter("port");
    auto cmd_cb = std::bind(&API::vehicleControlCommandCallback, this, std::placeholders::_1);
    cmd_sub_ = rclcpp::create_subscription<autoware_auto_msgs::msg::VehicleControlCommand>(
      node,
      "input/vehicle_control_command",
      rclcpp::QoS(10),
      std::move(cmd_cb),
      options);
    auto state_cmd_cb = std::bind(&API::vehicleStateCommandCallback, this, std::placeholders::_1);
    state_cmd_sub_ = rclcpp::create_subscription<autoware_auto_msgs::msg::VehicleStateCommand>(
      node,
      "input/vehicle_state_command",
      rclcpp::QoS(10),
      std::move(state_cmd_cb),
      options);
    entity_manager_ptr_ = std::make_shared<EntityManager>(node, map_path);
    metrics_manager_.setEntityManager(entity_manager_ptr_);
    client_ptr_ =
      std::shared_ptr<XmlRpc::XmlRpcClient>(new XmlRpc::XmlRpcClient(address.c_str(), port));
    setVerbose(verbose);
  }
  template<typename T, typename ... Ts>
  void addMetric(std::string name, Ts && ... xs)
  {
    metrics_manager_.addMetric<T>(name, std::forward<Ts>(xs)...);
  }
  boost::optional<double> getLinearJerk(std::string name);
  void setDriverModel(std::string name, const openscenario_msgs::msg::DriverModel & model);
  void setVerbose(bool verbose);

  // (1) Basis
  bool spawn(
    bool is_ego,
    const std::string & name,
    const std::string & catalog_xml);

  // (2) => (1)
  bool spawn(
    bool is_ego,
    const std::string & name,
    const simulation_api::entity::VehicleParameters & params);

  // (3) => (1)
  bool spawn(
    bool is_ego,
    const std::string & name,
    const simulation_api::entity::PedestrianParameters & params);

  template
  <
    typename ... Ts
  >
  decltype(auto) spawn(
    bool is_ego,
    const std::string & name,
    const std::string & catalog_xml,
    Ts && ... xs)
  {
    return
      spawn(is_ego, name, catalog_xml) &&
      setEntityStatus(name, std::forward<decltype(xs)>(xs)...);
  }

  template
  <
    typename Parameters,  // Maybe, VehicleParameters or PedestrianParameters
    typename ... Ts
  >
  decltype(auto) spawn(
    bool is_ego,
    const std::string & name,
    const Parameters & params,
    Ts && ... xs)
  {
    return
      spawn(is_ego, name, params) &&
      setEntityStatus(name, std::forward<decltype(xs)>(xs)...);
  }

  openscenario_msgs::msg::EntityStatus getEntityStatus(std::string name);
  bool setEntityStatus(
    std::string name, const geometry_msgs::msg::Pose & map_pose,
    const openscenario_msgs::msg::ActionStatus & action_status);
  bool setEntityStatus(
    std::string name, const openscenario_msgs::msg::LaneletPose & lanelet_pose,
    const openscenario_msgs::msg::ActionStatus & action_status);
  bool setEntityStatus(std::string name, const openscenario_msgs::msg::EntityStatus & status);
  bool setEntityStatus(
    std::string name, std::string reference_entity_name,
    const geometry_msgs::msg::Pose relative_pose,
    const openscenario_msgs::msg::ActionStatus action_status);
  bool setEntityStatus(
    std::string name, std::string reference_entity_name,
    const geometry_msgs::msg::Point relative_position,
    const geometry_msgs::msg::Vector3 relative_rpy,
    const openscenario_msgs::msg::ActionStatus action_status);

  boost::optional<double> getLongitudinalDistance(std::string from, std::string to);
  boost::optional<double> getTimeHeadway(std::string from, std::string to);
  void requestAcquirePosition(std::string name, openscenario_msgs::msg::LaneletPose lanelet_pose);
  void requestLaneChange(std::string name, std::int64_t to_lanelet_id);
  void requestLaneChange(std::string name, simulation_api::entity::Direction direction);
  bool isInLanelet(std::string name, std::int64_t lanelet_id, double tolerance);
  void setTargetSpeed(std::string name, double target_speed, bool continuous);
  geometry_msgs::msg::Pose getRelativePose(std::string from, std::string to);
  geometry_msgs::msg::Pose getRelativePose(geometry_msgs::msg::Pose from, std::string to);
  geometry_msgs::msg::Pose getRelativePose(std::string from, geometry_msgs::msg::Pose to);
  geometry_msgs::msg::Pose getRelativePose(
    geometry_msgs::msg::Pose from,
    geometry_msgs::msg::Pose to);
  bool reachPosition(std::string name, geometry_msgs::msg::Pose target_pose, double tolerance);
  bool reachPosition(
    std::string name, openscenario_msgs::msg::LaneletPose target_pose,
    double tolerance);
  bool reachPosition(std::string name, std::string target_name, double tolerance) const;
  boost::optional<double> getStandStillDuration(std::string name) const;
  bool checkCollision(std::string name0, std::string name1);
  XmlRpc::XmlRpcValue initialize(double realtime_factor, double step_time);
  XmlRpc::XmlRpcValue updateFrame();
  double getCurrentTime() const {return current_time_;}
  const boost::optional<openscenario_msgs::msg::LaneletPose> toLaneletPose(
    geometry_msgs::msg::Pose pose) const;
  const geometry_msgs::msg::Pose toMapPose(const openscenario_msgs::msg::LaneletPose lanelet_pose)
  const;
  bool despawnEntity(std::string name);
  bool entityExists(std::string name);
  template<typename ... Ts>
  void setrafficLightColorPhase(Ts && ... xs)
  {
    entity_manager_ptr_->setTrafficLightColorPhase(std::forward<Ts>(xs)...);
  }
  template<typename ... Ts>
  void setTrafficLightArrowPhase(Ts && ... xs)
  {
    entity_manager_ptr_->setTrafficLightArrowPhase(std::forward<Ts>(xs)...);
  }
  template<typename ... Ts>
  void setTrafficLightColor(Ts && ... xs)
  {
    entity_manager_ptr_->setTrafficLightColor(std::forward<Ts>(xs)...);
  }
  template<typename ... Ts>
  void setTrafficLightArrow(Ts && ... xs)
  {
    entity_manager_ptr_->setTrafficLightArrow(std::forward<Ts>(xs)...);
  }
  template<typename ... Ts>
  simulation_api::TrafficLightColor getTrafficLightColor(Ts && ... xs)
  {
    return entity_manager_ptr_->getTrafficLightColor(std::forward<Ts>(xs)...);
  }
  template<typename ... Ts>
  simulation_api::TrafficLightArrow getTrafficLightArrow(Ts && ... xs)
  {
    return entity_manager_ptr_->getTrafficLightArrow(std::forward<Ts>(xs)...);
  }

private:
  bool spawn(
    bool is_ego,
    const std::string & catalog_xml,
    const openscenario_msgs::msg::EntityStatus & status);
  bool spawn(
    bool is_ego,
    const simulation_api::entity::PedestrianParameters & params,
    const openscenario_msgs::msg::EntityStatus & status);
  bool spawn(
    bool is_ego,
    const simulation_api::entity::VehicleParameters & params,
    const openscenario_msgs::msg::EntityStatus & status);

  std::shared_ptr<XmlRpc::XmlRpcClient> client_ptr_;
  std::shared_ptr<simulation_api::entity::EntityManager> entity_manager_ptr_;
  double step_time_;
  double current_time_;
  openscenario_msgs::msg::EntityStatus toStatus(XmlRpc::XmlRpcValue param);
  XmlRpc::XmlRpcValue toValue(openscenario_msgs::msg::EntityStatus status);
  void vehicleControlCommandCallback(autoware_auto_msgs::msg::VehicleControlCommand::SharedPtr msg);
  boost::optional<autoware_auto_msgs::msg::VehicleControlCommand> current_cmd_;
  rclcpp::Subscription<autoware_auto_msgs::msg::VehicleControlCommand>::SharedPtr cmd_sub_;
  void vehicleStateCommandCallback(autoware_auto_msgs::msg::VehicleStateCommand::SharedPtr msg);
  boost::optional<autoware_auto_msgs::msg::VehicleStateCommand> current_state_cmd_;
  rclcpp::Subscription<autoware_auto_msgs::msg::VehicleStateCommand>::SharedPtr state_cmd_sub_;
  metrics::MetricsManager metrics_manager_;
};
}  // namespace scenario_simulator

#endif  // SIMULATION_API__API__API_HPP_
