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

#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>

#include <cmath>

#include "remap_plugin_faces/plugin_faces.hpp"

namespace remap
{
namespace plugins
{
PluginFaces::PluginFaces()
: SemanticPlugin() {}

PluginFaces::PluginFaces(
  std::shared_ptr<map_handler::SemanticMapHandler> & semantic_map,
  std::shared_ptr<remap::regions_register::RegionsRegister> & regions_register)
: SemanticPlugin(semantic_map, regions_register) {}

PluginFaces::~PluginFaces()
{
  semantic_map_.reset();
  regions_register_.reset();
}

void PluginFaces::representGaze(
  const geometry_msgs::msg::TransformStamped & gaze_transform,
  const std::string & gaze_id)
{
  auto gaze_translation = gaze_transform.transform.translation;
  auto gaze_rotation = gaze_transform.transform.rotation;

  tf2::Quaternion q_A_to_B(
    gaze_rotation.x,
    gaze_rotation.y,
    gaze_rotation.z,
    gaze_rotation.w);
  q_A_to_B.normalize();

  tf2::Vector3 axis_in_B(0.0, 0.0, 1.0);
  auto axis_in_A = tf2::quatRotate(q_A_to_B, axis_in_B);

  openvdb::Vec3d gaze_direction(
    axis_in_A.getX(),
    axis_in_A.getY(),
    axis_in_A.getZ());
  openvdb::Vec3d gaze_origin(
    gaze_translation.x,
    gaze_translation.y,
    gaze_translation.z);
  semantic_map_->insertSemanticCone(
    0.7,
    2.0,
    gaze_direction,
    std::string("gaze_") + gaze_id,
    *regions_register_,
    gaze_origin);
}

void PluginFaces::initialize()
{
  RCLCPP_INFO(node_ptr_->get_logger(), "PluginFaces initializing");

  hri_executor_ = rclcpp::executors::MultiThreadedExecutor::make_shared();
  hri_node_ = rclcpp::Node::make_shared("hri_node_remap_plugin_faces");
  hri_executor_->add_node(hri_node_);
  hri_listener_ = hri::HRIListener::create(hri_node_);
  hri_listener_->setReferenceFrame(semantic_map_->getFixedFrame());
}

void PluginFaces::run()
{
  hri_executor_->spin_all(std::chrono::milliseconds(50));

  std::vector<std::string> ids;
  auto faces = hri_listener_->getFaces();

  for (const auto & old_face : old_faces_) {
    semantic_map_->removeRegion(old_face, *regions_register_);
  }
  for (const auto & old_gaze : old_gazes_) {
    semantic_map_->removeRegion(old_gaze, *regions_register_);
  }

  for (const auto & face : faces) {
    if (face.second->valid()) {
      auto face_transform = face.second->transform();
      auto gaze_transform = face.second->gazeTransform();
      if (face_transform) {
        auto face_translation = face_transform->transform.translation;
        semantic_map_->insertSemanticSphere(
          0.2,
          std::string("face_") + face.first,
          *regions_register_,
          openvdb::Vec3d(face_translation.x, face_translation.y, face_translation.z));
        old_faces_.push_back(std::string("face_") + face.first);
      } else {
        RCLCPP_WARN(
          node_ptr_->get_logger(),
          "Face transform for face %s is not valid", face.first.c_str());
      }
      if (gaze_transform) {
        representGaze(*gaze_transform, face.first);
        old_gazes_.push_back(std::string("gaze_") + face.first);
      }
    }
  }
}

void PluginFaces::storeEntitiesRelationships(
  std::map<std::string, std::map<std::string, std::string>> relationships_matrix)
{
  (void) relationships_matrix;

  std::vector<std::string> new_facts;
  std::vector<std::string> old_facts;

  std::vector<std::string> & gazes = old_gazes_;  // alias, for clarity

  for (const auto & gaze : gazes) {
    auto in_fov_entities = regions_register_->getCoexistentEntities(gaze);

    if (in_fov_entities_.find(gaze) != in_fov_entities_.end()) {
      // we iterate over the objects in the field of view of this specific gaze
      for (const auto & object : in_fov_entities_[gaze]) {
        if (std::find(
            in_fov_entities.begin(), in_fov_entities.end(),
            object) == in_fov_entities.end())
        {
          // the object is not in the field of view anymore
          old_facts.push_back(object + " isInFoV " + gaze);
        }
      }
      // we iterate over the objects that are in the field of view
      // to see which one have just been detected
      for (const auto & object : in_fov_entities) {
        if (std::find(
            in_fov_entities_[gaze].begin(),
            in_fov_entities_[gaze].end(),
            object) == in_fov_entities_[gaze].end())
        {
          // the object is new
          new_facts.push_back(object + " isInFoV " + gaze);
        }
      }
    } else {
      // this gaze is new, so we add all the objects in the field of view
      for (const auto & object : in_fov_entities) {
        new_facts.push_back(object + " isInFoV " + gaze);
      }
    }
    in_fov_entities_[gaze] = in_fov_entities;
  }

  if (new_facts.size() > 0) {
    this->revisePushFacts(new_facts);
  }
  if (old_facts.size() > 0) {
    this->reviseRemoveFacts(old_facts);
  }
}
}  // namespace plugins
}  // namespace remap

#include <pluginlib/class_list_macros.hpp>

PLUGINLIB_EXPORT_CLASS(remap::plugins::PluginFaces, remap::plugins::PluginBase)
