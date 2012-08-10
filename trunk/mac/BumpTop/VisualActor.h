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

#ifndef BUMPTOP_VISUALACTOR_H_
#define BUMPTOP_VISUALACTOR_H_

#include <Ogre.h>
#include <string>
#include <vector>

#include "BumpTop/Clickable.h"

#define TS_LOCAL Ogre::Node::TS_LOCAL
#define TS_PARENT Ogre::Node::TS_PARENT
#define NEGATIVE_UNIT_Z Ogre::Vector3::NEGATIVE_UNIT_Z
#define kOgreSceneNodeScaleFactor 100.0

class Box;

class RenderOrder {
 public:
  RenderOrder();
  virtual ~RenderOrder();
  virtual Ogre::Renderable* renderable() = 0;
  virtual Ogre::MovableObject* movable_object() = 0;

  void set_to_render_after(RenderOrder* renderable);
  void set_to_render_before(RenderOrder* renderable);
 protected:
  void set_to_render_before_or_after_helper(RenderOrder* renderable, bool after);
  QList<RenderOrder*> items_whose_render_order_is_tied_to_mine_;
  RenderOrder* item_who_im_rendering_next_to_;
};

class VisualActor : public Clickable, public RenderOrder {
  Q_OBJECT

 public:
  explicit VisualActor(Ogre::SceneManager *scene_manager, Ogre::SceneNode *parent_ogre_scene_node,
                       std::string mesh_name, bool clickable = true);
  virtual ~VisualActor();

  virtual void init();
  Ogre::SceneNode* ogre_scene_node();
  virtual void setParentNodeWhileMaintainingAbsolutePosition(Ogre::SceneNode* parent_ogre_scene_node);
  // Methods for changing properties
  virtual void set_scale(Ogre::Vector3 scale);

  // Methods for getting properties
  virtual const Ogre::Vector3& scale();

  virtual Ogre::Renderable* renderable();
  virtual Ogre::MovableObject* movable_object();

  // Methods for changing orientation
  virtual void set_orientation(const Ogre::Quaternion &q);
  virtual void reset_orientation(void);

  virtual void setDirection(Ogre::Real x, Ogre::Real y, Ogre::Real z,
                            Ogre::Node::TransformSpace relativeTo = TS_LOCAL,
                            const Ogre::Vector3 &localDirectionVector = NEGATIVE_UNIT_Z);

  virtual void setDirection(Ogre::Vector3 x, Ogre::Node::TransformSpace relativeTo = TS_LOCAL,
                            const Ogre::Vector3 &localDirectionVector = NEGATIVE_UNIT_Z);

  virtual void lookAt(const Ogre::Vector3 &targetPoint, Ogre::Node::TransformSpace relativeTo,
                      const Ogre::Vector3 &localDirectionVector = NEGATIVE_UNIT_Z);
  virtual void roll(const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo = TS_LOCAL);
  virtual void pitch(const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo = TS_LOCAL);
  virtual void yaw(const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo = TS_LOCAL);
  virtual void rotate(const Ogre::Vector3 &axis, const Ogre::Radian &angle,
                      Ogre::Node::TransformSpace relativeTo = TS_LOCAL);
  virtual void rotate(const Ogre::Quaternion &q, Ogre::Node::TransformSpace relativeTo = TS_LOCAL);

  // Methods for changing position
  virtual void set_position(const Ogre::Vector3 &pos);

  virtual void translate(const Ogre::Vector3 &d, Ogre::Node::TransformSpace relativeTo = TS_PARENT);
  virtual void translate(Ogre::Real x, Ogre::Real y, Ogre::Real z, Ogre::Node::TransformSpace relativeTo = TS_PARENT);
  virtual void translate(const Ogre::Matrix3 &axes, const Ogre::Vector3 &move,
                         Ogre::Node::TransformSpace relativeTo = TS_PARENT);
  virtual void translate(const Ogre::Matrix3 &axes, Ogre::Real x, Ogre::Real y, Ogre::Real z,
                         Ogre::Node::TransformSpace relativeTo = TS_PARENT);


  virtual Ogre::Plane plane();

  // Other setters
  virtual void set_material_name(const QString &material_name);
  virtual void set_inherit_orientation(bool inherit_orientation);
  virtual void set_inherit_scale(bool inherit_scale);
  virtual void set_visible(bool visible);

  virtual void set_render_queue_group(uint8 queue_id);
  virtual uint8 render_queue_group();

  // Getters
  virtual const Ogre::Quaternion& orientation() const;
  virtual const Ogre::Vector3& position() const;
  virtual const Ogre::Vector3 world_position();
  virtual const Ogre::Quaternion world_orientation();

  virtual Ogre::Vector2 getScreenPosition();

  virtual const Ogre::AxisAlignedBox& world_bounding_box();
  virtual Ogre::Entity* _entity();
  virtual const QString& material_name();

  virtual void set_clickable_parent(Clickable* clickable_parent);

 protected:
  Ogre::Entity* entity_;
  Ogre::SceneNode *ogre_scene_node_, *parent_ogre_scene_node_;
  Ogre::SceneManager *scene_manager_;
  QString material_name_;
  Ogre::Vector3 scale_;
  std::string mesh_name_;
  bool clickable_;
};
#endif  // BUMPTOP_VISUALACTOR_H_
