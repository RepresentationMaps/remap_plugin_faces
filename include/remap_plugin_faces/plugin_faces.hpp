// Copyright 2025 PAL Robotics, S.L.
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

#ifndef REMAP_PLUGIN_FACES__PLUGIN_FACES_HPP_
#define REMAP_PLUGIN_FACES__PLUGIN_FACES_HPP_

#include <tf2_ros/buffer.hpp>
#include <tf2_ros/transform_listener.hpp>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include <rclcpp/rclcpp.hpp>

#include <remap_plugin_base/plugin_base.hpp>
#include <remap_plugin_base/semantic_plugin.hpp>

#include <hri/hri.hpp>

namespace remap
{
namespace plugins
{
class PluginFaces : public SemanticPlugin
{
private:
  std::vector<std::string> regions_;

  rclcpp::Executor::SharedPtr hri_executor_;
  rclcpp::Node::SharedPtr hri_node_;
  std::shared_ptr<hri::HRIListener> hri_listener_;

  std::vector<std::string> old_faces_;
  std::vector<std::string> old_gazes_;

  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;

  std::string robot_gaze_frame_;

  std::map<std::string, std::unordered_set<std::string>> in_fov_entities_;

  void representGaze(
    const geometry_msgs::msg::TransformStamped & gaze_transform,
    const std::string & gaze_id);

public:
  PluginFaces();
  PluginFaces(
    std::shared_ptr<map_handler::SemanticMapHandler> & semantic_map,
    std::shared_ptr<remap::regions_register::RegionsRegister> & regions_register);
  ~PluginFaces();
  void run() override;
  void initialize() override;
  void storeEntitiesRelationships(
    std::map<std::string, std::map<std::string, std::string>> relationships_matrix) override;
};
}    // namespace plugins
}  // namespace remap
#endif  // REMAP_PLUGIN_FACES__PLUGIN_FACES_HPP_
