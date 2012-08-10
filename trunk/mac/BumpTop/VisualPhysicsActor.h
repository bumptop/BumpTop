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

#ifndef BUMPTOP_VISUALPHYSICSACTOR_H_
#define BUMPTOP_VISUALPHYSICSACTOR_H_

#include <Ogre.h>

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <string>
#include <vector>

#include "BumpTop/Clickable.h"
#include "BumpTop/BumpTopCommands.h"
#include "BumpTop/FileDropReceiver.h"
#include "BumpTop/KeyboardEventManager.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/VisualPhysicsActorId.h"

using Ogre::Vector3;
using Ogre::Node;

class BumpBoxLabel;
class Physics;
class PhysicsActor;
class RoomSurface;
class VisualPhysicsActorBuffer;
class VisualActor;

// We need to keep this enum hard-coded because
// these values are used by protocol buffers
enum VisualPhysicsActorType {
  NULL_ACTOR_TYPE = -1,
  BOX = 0,
  BUMP_BOX = 1,
  BUMP_PILE = 2,
  GRIDDED_PILE = 4,
  STICKY_NOTE = 5,
  STICKY_NOTE_PAD = 6,
  BUMP_DUMMY = 7
};

enum BumpBoxLabelColour {
  COLOURLESS = 0,
  RED = 1,
  ORANGE = 2,
  YELLOW = 3,
  GREEN = 4,
  BLUE = 5,
  PURPLE = 6,
  GREY = 7,
  RAINBOW = 8
};

#define TS_LOCAL Ogre::Node::TS_LOCAL
#define TS_PARENT Ogre::Node::TS_PARENT
#define NEGATIVE_UNIT_Z Ogre::Vector3::NEGATIVE_UNIT_Z

#define VPA_GROW_FACTOR 1.25
#define MAX_ACTOR_SIZE 300
#define MIN_ACTOR_SIZE 35
#define DEFAULT_FRICTION 1.0
#define VELOCITY_THRESHOLD_FOR_MOTION 5

#ifndef BUMPTOP_TEST
#define PHYSICS_SCALE 100.0
#else
#define PHYSICS_SCALE 1.0
#endif

class VisualPhysicsActor : public Clickable, public DropTarget {
  Q_OBJECT

 public:
  explicit VisualPhysicsActor(Ogre::SceneManager *scene_manager, Physics *physics,
                              Ogre::SceneNode *parent_ogre_scene_node, VisualPhysicsActorId unique_id = 0);
  virtual ~VisualPhysicsActor();
  virtual void init(bool physics_enabled = true);
  virtual void initAsVisualCopyOfActor(VisualPhysicsActor* actor);
  virtual VisualPhysicsActor* createVisualCopyOfSelf();
  virtual VisualPhysicsActorId unique_id();

  Ogre::SceneNode* ogre_scene_node();
  virtual Ogre::SceneNode* ogre_scene_node_for_children();
  VisualActor* visual_actor();
  PhysicsActor* physics_actor();

  virtual VisualPhysicsActor* parent();
  virtual void set_parent(VisualPhysicsActor* parent);

  // Methods concerning selction and path; these will be given default implementations, but must be overridden
  // to have "real" function
  virtual void set_selected(bool selected);
  virtual bool selected();
  virtual void set_path(QString path);
  virtual const QString& path();
  virtual QStringList pathsOfDescendants();
  virtual bool breakable();
  virtual bool pinnable();
  virtual void breakPile();
  virtual void breakPileWithoutConstrainingFinalPoses();
  virtual void stackViewAndScaleChildren(Ogre::Real scale_factor);
  virtual void addActorToPileAndUpdatePileView(VisualPhysicsActor* actor,
                                               Ogre::Vector3 offset_from_pile,
                                               Ogre::Quaternion orientation);
  virtual void breakItemsOutOfPile(QList<VisualPhysicsActorId> actor_ids);
  virtual void breakAllItemsExceptDummy();

  virtual VisualPhysicsActorList children();
  virtual VisualPhysicsActorList flattenedChildren();
  virtual QList<VisualPhysicsActorId> flattenedChildrenIds();
  virtual QList<VisualPhysicsActorId> children_ids();
  virtual QList<Ogre::Vector3> children_offsets();
  virtual QList<Ogre::Quaternion> children_orientations();
  virtual void writeToBuffer(VisualPhysicsActorBuffer* buffer);
  virtual bool serializable();
  virtual bool initFromBuffer(VisualPhysicsActorBuffer* buffer, bool physics_enabled = true);
  virtual void setPhysicsConstraintsForSurface(RoomSurface* surface);
  virtual void removePhysicsConstraints();
  virtual BumpPose getPoseForSurface(RoomSurface* surface);
  virtual void pinToSurface(RoomSurface* surface);
  virtual void set_room_surface(RoomSurface* room_surface);
  virtual void setCollisionsToOnlyWalls();
  virtual void setCollisionsToAllActors();
  virtual void setCollisionsToGriddedPiles();
  virtual bool isWall();
  virtual int16_t collision_group();
  virtual int16_t collision_mask();
  virtual void setPoseConstrainedToRoomAndNoIntersections();
  virtual BumpPose getPoseConstrainedToRoomAndNoIntersections();
  virtual VisualPhysicsActor* lowest_child_with_visual_actor();
  virtual void set_alpha(Ogre::Real alpha);
  virtual void launch();
  virtual void launch(QString app);
  virtual void closeView();
  virtual bool nameable();
  virtual Ogre::Plane plane();

  // Label Stuff
  virtual Ogre::Vector2 labelPositionForCurrentPosition();
  virtual void updateLabelPosition();
  virtual void updateLabel(Ogre::Real size_factor);
  virtual void set_label_visible(bool label_visible);
  virtual BumpBoxLabel* label();

  virtual bool label_visible();
  virtual void set_name_hidden(bool name_hidden);
  virtual bool name_hidden();
  virtual bool is_an_image_on_wall();
  virtual BumpBoxLabelColour label_colour();
  virtual void set_label_colour(BumpBoxLabelColour label_colour);

  virtual void rename(QString new_name);
  virtual QString display_name();

  virtual bool isMoving();

  virtual bool collides_with_walls_only();
  virtual bool collides_with_gridded_piles();
  virtual RoomSurface* room_surface();

  // Methods for changing properties
  virtual void setSizeForGrowOrShrink(Ogre::Vector3 size);
  virtual void set_size(Ogre::Vector3 size);

  // Methods for getting properties
  virtual Ogre::Vector3 scale();
  virtual Ogre::Vector3 size();
  virtual Ogre::Vector3 visual_size();

  // Sorting children
  virtual void sortAlphabetically();

  // Methods for changing orientation
  virtual void set_pose(BumpPose pose);
  virtual void set_orientation(const Ogre::Quaternion &q);
  virtual void setFriction(Ogre::Real friction);
  virtual void setMass(Ogre::Real mass);

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

  virtual void set_position_no_physics(const Ogre::Vector3 &pos);
  virtual void set_orientation_no_physics(const Ogre::Quaternion &q);

  virtual void translate(const Ogre::Vector3 &d, Ogre::Node::TransformSpace relativeTo = TS_PARENT);
  virtual void translate(Ogre::Real x, Ogre::Real y, Ogre::Real z, Ogre::Node::TransformSpace relativeTo = TS_PARENT);
  virtual void translate(const Ogre::Matrix3 &axes, const Ogre::Vector3 &move,
                         Ogre::Node::TransformSpace relativeTo = TS_PARENT);
  virtual void translate(const Ogre::Matrix3 &axes, Ogre::Real x, Ogre::Real y, Ogre::Real z,
                         Ogre::Node::TransformSpace relativeTo = TS_PARENT);

  // Methods for changing velocity
  virtual void set_angular_velocity(Ogre::Vector3 angular_velocity);
  virtual void set_linear_velocity(Ogre::Vector3 linear_velocity);

  // Physics interaction methods
  virtual void applyCentralImpulse(const Ogre::Vector3 &impulse);
  virtual void activatePhysics();
  virtual void set_physics_enabled(bool physics_enabled);
  virtual bool physics_enabled();

  virtual void set_material_name(const QString &material_name);
  const QString& material_name();
  virtual void setMaterialBlendFactors(Ogre::Vector3 blend_factor);

  virtual void set_visible(bool visible);
  virtual void set_render_queue_group_for_mouse_up(uint8 queue_id);
  virtual void set_render_queue_group(uint8 queue_id);
  virtual uint8 render_queue_group();

  // Getters
  virtual BumpPose pose();
  virtual Ogre::Matrix4 transform();
  virtual const Ogre::Quaternion& orientation() const;
  virtual const Ogre::Vector3& position() const;
  virtual const Ogre::Vector3 world_position();
  virtual Ogre::Vector3 linear_velocity();
  virtual Ogre::Vector3 angular_velocity();
  virtual Ogre::Vector2 getScreenPosition();
  virtual std::vector<Ogre::Vector2> getCornersInScreenSpace();
  virtual Ogre::AxisAlignedBox screenBoundingBox();
  virtual Ogre::AxisAlignedBox world_bounding_box();
  virtual Ogre::AxisAlignedBox destinationWorldBoundingBox();
  virtual Ogre::Entity* _entity();
  virtual Ogre::Real mass();
  virtual BumpPose destination_pose();
  virtual Ogre::Vector3 destinationScale();
  virtual Ogre::Real alpha();
  virtual bool is_new_items_pile();
  virtual VisualPhysicsActorId adjacent_actor(ArrowKey arrow_key);
  virtual VisualPhysicsActorId adjacentActorOfChild(VisualPhysicsActorId child_id, ArrowKey arrow_key);
  virtual void revealChild(VisualPhysicsActorId child_id);

  virtual void poseUpdatedByPhysics(const btTransform& transform);
  virtual VisualPhysicsActorType actor_type() = 0;

  virtual BumpTopCommandSet* supported_context_menu_items();
  virtual bool supportsContextMenuItem(BumpTopCommand* context_menu_option);

  virtual void set_clickable_parent(Clickable* clickable_parent);
  virtual void mouseMotionRegistered();

  virtual Ogre::Vector2 actor_screen_position_before_drag();
  virtual BumpPose actor_pose_before_drag();
  virtual RoomSurface* actor_room_surface_before_drag();
  virtual void updateActorStatusBeforeDrag();

  virtual VisualPhysicsActorId actor_parent_id_before_drag();
  virtual BumpPose actor_offset_pose_to_its_parent();
  virtual QHash<VisualPhysicsActorId, BumpPose> actor_siblings_offset_poses_to_parent();
  virtual VisualPhysicsActorType actor_parent_type_before_drag();
  virtual Ogre::Vector3 actor_parent_position_before_drag();
  virtual void updateActorParentInfoBeforeDrag();
  virtual void updateActorOffsetPoseToItsParentBeforeDrag();
  virtual void updateActorSiblingOffsetPoseToParentBeforeDrag();

  virtual BumpPose children_offset_pose(VisualPhysicsActor* actor);
  virtual void set_children_offset_pose(QHash<VisualPhysicsActorId, BumpPose> children_ids_to_offset_poses);

 public slots:  // NOLINT
  virtual void mouseDown(MouseEvent* mouse_event);
  virtual void mouseDragged(MouseEvent* mouse_event);
  virtual void mouseUp(MouseEvent* mouse_event);
  virtual void mouseMoved(MouseEvent* mouse_event);
  virtual void rightMouseDown(MouseEvent* mouse_event);
  virtual void rightMouseDragged(MouseEvent* mouse_event);
  virtual void rightMouseUp(MouseEvent* mouse_event);
  virtual void scrollWheel(MouseEvent* mouse_event);
  virtual void renderTick();
  virtual void update();

 signals:
  void onRemoved(VisualPhysicsActorId actor_id);
  void onSizeChanged(VisualPhysicsActorId actor_id);
  void onSelectedChanged(VisualPhysicsActorId actor_id);
  void onPoseChanged(VisualPhysicsActorId actor_id);
  void onStoppedMoving(VisualPhysicsActorId actor_id);
  void onMouseMotionRegistered(VisualPhysicsActor* actor);
  void onFinishedDragging(VisualPhysicsActor* actor);

 protected:
  virtual std::string meshName() = 0;
  virtual Ogre::Vector3 absoluteMeshSizeDividedBy100() = 0;
  virtual btVector3 physicsSize() = 0;
  virtual void makePhysicsActor(bool physics_enabled) = 0;
  virtual void syncBulletTransformWithOgreTransform();
  virtual void setParentNodeWhileMaintainingAbsolutePosition(Ogre::SceneNode* parent_ogre_scene_node);

  static VisualPhysicsActorId unique_id_counter_; // NOLINT
  VisualPhysicsActorId unique_id_;

  bool name_hidden_;
  bool is_moving_;
  bool pose_has_changed_;
  PhysicsActor* physics_actor_;
  VisualActor* visual_actor_;
  VisualPhysicsActor* parent_;
  Ogre::SceneNode* parent_ogre_scene_node_;
  Physics* physics_;
  Ogre::SceneManager* scene_manager_;
  RoomSurface* room_surface_;
  Ogre::Real alpha_;
};

bool compareDisplayName(VisualPhysicsActor* actor_1, VisualPhysicsActor* actor_2);

#endif  // BUMPTOP_VISUALPHYSICSACTOR_H_
