/*
 *  Copyright 2012 Google Inc. All Rights Reserved.
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "BumpTop/VisualActor.h"

#include <string>
#include <vector>

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/DebugAssert.h"
#include "BumpTop/OgreBulletConverter.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/Physics.h"
#include "BumpTop/QStringHelpers.h"

RenderOrder::RenderOrder() {
  item_who_im_rendering_next_to_ = NULL;
}

RenderOrder::~RenderOrder() {
  if (item_who_im_rendering_next_to_ != NULL) {
    item_who_im_rendering_next_to_->items_whose_render_order_is_tied_to_mine_.removeOne(this);
  }
  item_who_im_rendering_next_to_ = NULL;

  for_each(RenderOrder* actor, items_whose_render_order_is_tied_to_mine_) {
    actor->set_to_render_after(NULL);
  }
}
void RenderOrder::set_to_render_before_or_after_helper(RenderOrder* other_item, bool after) {
  if (item_who_im_rendering_next_to_ != NULL) {
    item_who_im_rendering_next_to_->items_whose_render_order_is_tied_to_mine_.removeOne(this);
  }
  item_who_im_rendering_next_to_ = other_item;
  if (other_item == NULL) {
    renderable()->setToRenderAfter(NULL);
  } else {
    DEBUG_ASSERT(movable_object()->getRenderQueueGroup() == other_item->movable_object()->getRenderQueueGroup());
    if (after) {
      renderable()->setToRenderAfter(other_item->renderable());
    } else {
      renderable()->setToRenderBefore(other_item->renderable());
    }
    other_item->items_whose_render_order_is_tied_to_mine_.append(this);
  }
}

void RenderOrder::set_to_render_after(RenderOrder* other_item) {
  set_to_render_before_or_after_helper(other_item, true);
}

void RenderOrder::set_to_render_before(RenderOrder* other_item) {
  set_to_render_before_or_after_helper(other_item, false);
}


VisualActor::VisualActor(Ogre::SceneManager *scene_manager, Ogre::SceneNode *parent_ogre_scene_node,
                         std::string mesh_name, bool clickable)
: Clickable(),
  RenderOrder(),
  scene_manager_(scene_manager),
  parent_ogre_scene_node_(parent_ogre_scene_node),
  mesh_name_(mesh_name),
  clickable_(clickable) {
}

Ogre::Renderable* VisualActor::renderable() {
  assert(_entity()->getNumSubEntities() == 1);
  return _entity()->getSubEntity(0);
}

Ogre::MovableObject* VisualActor::movable_object() {
  return _entity();
}

void VisualActor::init() {
  // set up Ogre objects
  if (parent_ogre_scene_node_ == NULL)
    parent_ogre_scene_node_ = scene_manager_->getRootSceneNode();

  Ogre::String scene_node_name = "VisualActor" + addressToString(this);
  ogre_scene_node_ = parent_ogre_scene_node_->createChildSceneNode(scene_node_name);

  if (mesh_name_ != "") {
    Ogre::String entity_name = "VisualActor" + addressToString(this);
    entity_ = scene_manager_->createEntity(entity_name, mesh_name_);

    if (clickable_)
      entity_->setUserAny(Ogre::Any(static_cast<Clickable*>(this)));

    ogre_scene_node_->attachObject(entity_);
  } else {
    entity_ = NULL;
  }
}


VisualActor::~VisualActor() {
  if (entity_ != NULL) {
    entity_->detachFromParent();
    scene_manager_->destroyEntity(entity_);
  }

  ogre_scene_node_->getParent()->removeChild(ogre_scene_node_);
  scene_manager_->destroySceneNode(ogre_scene_node_);
}

// Methods for changing properties

void VisualActor::setParentNodeWhileMaintainingAbsolutePosition(Ogre::SceneNode* parent_ogre_scene_node) {
  // We switch the items parent scene node to the new parent's scene node
  // TODO: find away to change coordinate systems without changing anything else
  Ogre::Matrix4 current_world_transform, new_parent_transform;
  current_world_transform = ogre_scene_node()->_getFullTransform();
  new_parent_transform = parent_ogre_scene_node->_getFullTransform();

  ogre_scene_node()->getParent()->removeChild(ogre_scene_node());
  parent_ogre_scene_node->addChild(ogre_scene_node());

  current_world_transform =  new_parent_transform.inverseAffine()*current_world_transform;
  set_position(current_world_transform.getTrans());
  set_orientation(current_world_transform.extractQuaternion());
}

// TODO: deal with this whole size shambles
void VisualActor::set_scale(Ogre::Vector3 scale) {
  scale_ = scale;
  ogre_scene_node_->setScale(scale_ / kOgreSceneNodeScaleFactor);
}

// Methods for getting properties
const Ogre::Vector3& VisualActor::scale() {
  return scale_;
}

// Methods for changing orientation
void VisualActor::set_orientation(const Ogre::Quaternion &q) {
  ogre_scene_node_->setOrientation(q);
}

void VisualActor::reset_orientation(void) {
  ogre_scene_node_->resetOrientation();
}
void VisualActor::setDirection(Ogre::Real x, Ogre::Real y, Ogre::Real z, Ogre::Node::TransformSpace relativeTo,
                       const Ogre::Vector3 &localDirectionVector) {
  ogre_scene_node_->setDirection(x, y, z, relativeTo, localDirectionVector);
}

void VisualActor::setDirection(Ogre::Vector3 x, Ogre::Node::TransformSpace relativeTo,
                       const Ogre::Vector3 &localDirectionVector) {
  ogre_scene_node_->setDirection(x, relativeTo, localDirectionVector);
}

void VisualActor::lookAt(const Ogre::Vector3 &targetPoint, Ogre::Node::TransformSpace relativeTo,
                 const Ogre::Vector3 &localDirectionVector) {
  ogre_scene_node_->lookAt(targetPoint, relativeTo, localDirectionVector);
}

void VisualActor::roll(const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo) {
  ogre_scene_node_->roll(angle, relativeTo);
}

void VisualActor::pitch(const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo) {
  ogre_scene_node_->pitch(angle, relativeTo);
}

void VisualActor::yaw(const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo) {
  ogre_scene_node_->yaw(angle, relativeTo);
}

void VisualActor::rotate(const Ogre::Vector3 &axis, const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo) {
  ogre_scene_node_->rotate(axis, angle, relativeTo);
}

void VisualActor::rotate(const Ogre::Quaternion &q, Ogre::Node::TransformSpace relativeTo) {
  ogre_scene_node_->rotate(q, relativeTo);
}

// Methods for changing position

void VisualActor::set_position(const Ogre::Vector3 &pos) {
  ogre_scene_node_->setPosition(pos);
}

void VisualActor::translate(const Ogre::Vector3 &d, Ogre::Node::TransformSpace relativeTo) {
  ogre_scene_node_->translate(d, relativeTo);
}

void VisualActor::translate(Ogre::Real x, Ogre::Real y, Ogre::Real z, Ogre::Node::TransformSpace relativeTo) {
  ogre_scene_node_->translate(x, y, z, relativeTo);
}

void VisualActor::translate(const Ogre::Matrix3 &axes, const Ogre::Vector3 &move,
                            Ogre::Node::TransformSpace relativeTo) {
  ogre_scene_node_->translate(axes, move, relativeTo);
}

void VisualActor::translate(const Ogre::Matrix3 &axes, Ogre::Real x, Ogre::Real y, Ogre::Real z,
                    Ogre::Node::TransformSpace relativeTo) {
  ogre_scene_node_->translate(axes, x, y, z, relativeTo);
}


Ogre::Plane VisualActor::plane() {
  Ogre::Matrix4 transform = ogre_scene_node_->_getFullTransform();

  Ogre::Vector3 pt1 = transform*Ogre::Vector3(-50, 0, -50);
  Ogre::Vector3 pt2 = transform*Ogre::Vector3(50, 0, 50);
  Ogre::Vector3 pt3 = transform*Ogre::Vector3(-50, 0, 50);

  return Ogre::Plane(pt1, pt3, pt2);
}

// Other setters
void VisualActor::set_material_name(const QString& material_name) {
  material_name_ = material_name;
  if (entity_ != NULL) {
    entity_->setMaterialName(utf8(material_name_));
    BumpTopApp::singleton()->markGlobalStateAsChanged();
  }
}

void VisualActor::set_inherit_orientation(bool inherit_orientation) {
  ogre_scene_node_->setInheritOrientation(inherit_orientation);
}

void VisualActor::set_inherit_scale(bool inherit_scale) {
  ogre_scene_node_->setInheritScale(inherit_scale);
}

void VisualActor::set_visible(bool visible) {
  ogre_scene_node_->setVisible(visible);
  BumpTopApp::singleton()->markGlobalStateAsChanged();
}

void VisualActor::set_render_queue_group(uint8 queue_id) {
  if (entity_ != NULL) {
    entity_->setRenderQueueGroup(queue_id);
    BumpTopApp::singleton()->markGlobalStateAsChanged();
  }
}

uint8 VisualActor::render_queue_group() {
  return entity_->getRenderQueueGroup();
}

// Getters
const Ogre::Quaternion& VisualActor::orientation() const {
  return ogre_scene_node_->getOrientation();
}

const Ogre::Vector3& VisualActor::position() const {
  return ogre_scene_node_->getPosition();
}

const Ogre::Vector3 VisualActor::world_position() {
  Ogre::Matrix4 world_transform = ogre_scene_node_->_getFullTransform();
  return world_transform.getTrans();
}

const Ogre::Quaternion VisualActor::world_orientation() {
  Ogre::Matrix4 world_transform = ogre_scene_node_->_getFullTransform();
  return world_transform.extractQuaternion();
}

Ogre::Vector2 VisualActor::getScreenPosition() {
  return worldPositionToScreenPosition(ogre_scene_node_->getPosition());
}

Ogre::SceneNode* VisualActor::ogre_scene_node() {
  return ogre_scene_node_;
}

const Ogre::AxisAlignedBox& VisualActor::world_bounding_box() {
  if (entity_ != NULL)
    return entity_->getWorldBoundingBox();
  else
    return Ogre::AxisAlignedBox::BOX_NULL;
}

Ogre::Entity* VisualActor::_entity() {
  return entity_;
}

const QString& VisualActor::material_name() {
  return material_name_;
}

void VisualActor::set_clickable_parent(Clickable* clickable_parent) {
  clickable_parent_ = clickable_parent;
}

#include "BumpTop/moc/moc_VisualActor.cpp"

