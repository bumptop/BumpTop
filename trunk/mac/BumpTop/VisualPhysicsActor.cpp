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

#include "BumpTop/VisualPhysicsActor.h"
#include <string>
#include <vector>

#include "BumpTop/AnimationManager.h"
#include "BumpTop/BumpBoxLabel.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/OgreBulletConverter.h"
#include "BumpTop/Math.h"
#include "BumpTop/Physics.h"
#include "BumpTop/PhysicsActor.h"
#include "BumpTop/PhysicsBoxActor.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/RoomItemPoseConstraints.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/VisualActor.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"

VisualPhysicsActorId VisualPhysicsActor::unique_id_counter_ = 1;

VisualPhysicsActor::VisualPhysicsActor(Ogre::SceneManager *scene_manager, Physics *physics,
                                       Ogre::SceneNode *parent_ogre_scene_node, VisualPhysicsActorId unique_id)
: Clickable(),
  scene_manager_(scene_manager),
  physics_(physics),
  parent_ogre_scene_node_(parent_ogre_scene_node),
  physics_actor_(NULL),
  visual_actor_(NULL),
  parent_(NULL),
  name_hidden_(false),
  room_surface_(NULL),
  pose_has_changed_(false),
  is_moving_(false),
  alpha_(1.0) {
  if (unique_id == 0) {
    unique_id_ = unique_id_counter_;
    unique_id_counter_++;
  } else {
    unique_id_ = unique_id;
  }
}

void VisualPhysicsActor::init(bool physics_enabled) {
  // set up Ogre objects

  // meshName() is overridden by the subclass and if it is empty (""), then the VisualPhysicsActor has no entity, and
  // hence has no visual component. In this case, the VisualActor exists exclusively for it's scene node,
  // which is used for positioning and orientation of the physical actor
  visual_actor_ = new VisualActor(scene_manager_, parent_ogre_scene_node_, meshName());
  visual_actor_->init();

  assert(QObject::connect(visual_actor_, SIGNAL(onMouseDown(MouseEvent*)),  // NOLINT
                          this, SLOT(mouseDown(MouseEvent*))));  // NOLINT
  assert(QObject::connect(visual_actor_, SIGNAL(onMouseDragged(MouseEvent*)),  // NOLINT
                          this, SLOT(mouseDragged(MouseEvent*))));  // NOLINT
  assert(QObject::connect(visual_actor_, SIGNAL(onMouseUp(MouseEvent*)),  // NOLINT
                          this, SLOT(mouseUp(MouseEvent*))));  // NOLINT
  assert(QObject::connect(visual_actor_, SIGNAL(onMouseMoved(MouseEvent*)),  // NOLINT
                          this, SLOT(mouseMoved(MouseEvent*))));  // NOLINT
  assert(QObject::connect(visual_actor_, SIGNAL(onRightMouseDown(MouseEvent*)),  // NOLINT
                          this, SLOT(rightMouseDown(MouseEvent*))));  // NOLINT
  assert(QObject::connect(visual_actor_, SIGNAL(onRightMouseDragged(MouseEvent*)),  // NOLINT
                          this, SLOT(rightMouseDragged(MouseEvent*))));  // NOLINT
  assert(QObject::connect(visual_actor_, SIGNAL(onRightMouseUp(MouseEvent*)),  // NOLINT
                          this, SLOT(rightMouseUp(MouseEvent*))));  // NOLINT
  assert(QObject::connect(visual_actor_, SIGNAL(onScrollWheel(MouseEvent*)),  // NOLINT
                          this, SLOT(scrollWheel(MouseEvent*))));  // NOLINT
  assert(QObject::connect(BumpTopApp::singleton(), SIGNAL(onRender()),  // NOLINT
                          this, SLOT(renderTick())));  // NOLINT


  // makePhysicsActor() is overridden by the subclass and may set physics_actor_ to be NULL. In the case that the
  // PhysicsActor is NULL, the VisualPhysicsActor has no physical compenent. This should not be confused with the case
  // where the physics actor is set to have "physics_enabled" set to false; in this case, the VisualPhysicsActor has
  // a physical component, but it is set initially to not be interactable within the dynamics world.
  makePhysicsActor(physics_enabled);
  if (physics_actor_ != NULL) {
    physics_actor_->set_owner(this);
    physics_actor_->setFriction(DEFAULT_FRICTION);
  }
}

void VisualPhysicsActor::initAsVisualCopyOfActor(VisualPhysicsActor* actor) {
  visual_actor_ = new VisualActor(scene_manager_, parent_ogre_scene_node_, meshName());
  visual_actor_->init();
  physics_actor_ = NULL;
}

VisualPhysicsActor* VisualPhysicsActor::createVisualCopyOfSelf() {
  return NULL;
}

VisualPhysicsActor::~VisualPhysicsActor() {
  AnimationManager::singleton()->endAnimationsForActor(this, AnimationManager::MOVE_TO_FINAL_STATE);
  if (physics_actor_ != NULL) {
    delete physics_actor_;
  }
  if (visual_actor_ != NULL) {
    delete visual_actor_;
  }
  memset(this, 0, sizeof(this));
}

VisualPhysicsActorId VisualPhysicsActor::unique_id() {
  return unique_id_;
}

VisualPhysicsActor* VisualPhysicsActor::parent() {
  return parent_;
}

BumpPose VisualPhysicsActor::destination_pose() {
  QList<VisualPhysicsActorAnimation*> animations = AnimationManager::singleton()->actor_animations();
  for_each(VisualPhysicsActorAnimation* animation, animations) {
    VisualPhysicsActor* actor = animation->visual_physics_actor();
    if (actor->unique_id() == unique_id()) {
      return animation->final_pose();
    }
  }
  return pose();
}

Ogre::Vector3 VisualPhysicsActor::destinationScale() {
  QList<VisualPhysicsActorAnimation*> animations = AnimationManager::singleton()->actor_animations();
  for_each(VisualPhysicsActorAnimation* animation, animations) {
    VisualPhysicsActor* actor = animation->visual_physics_actor();
    if (actor->unique_id() == unique_id()) {
      return animation->final_scale();
    }
  }
  return scale();
}

void VisualPhysicsActor::set_parent(VisualPhysicsActor* parent) {
  // special case: if the parent is _changed_ to NULL, set our parent scene node to the Room's scene node
  // by convention, a NULL parent is the same as the Room being your parent
  if (parent_ != NULL && parent == NULL)
    setParentNodeWhileMaintainingAbsolutePosition(room_surface()->room()->ogre_scene_node());
  else if (parent != NULL)
    setParentNodeWhileMaintainingAbsolutePosition(parent->ogre_scene_node_for_children());
  parent_ = parent;
  set_clickable_parent(parent);
}

void VisualPhysicsActor::setParentNodeWhileMaintainingAbsolutePosition(Ogre::SceneNode* parent_ogre_scene_node) {
  AnimationManager::singleton()->endAnimationsForActor(this, AnimationManager::MOVE_TO_FINAL_STATE);
  visual_actor_->setParentNodeWhileMaintainingAbsolutePosition(parent_ogre_scene_node);
  // TODO: this shouldn't be needed, but... it is. I think this is related to the fact that physics
  // isn't using relative positions, so when actors have relative positions (eg. piles and gridded pies) the
  // correspondence of physics and ogre is very screwed up; you don't usually see this though, since physics
  // is disabled on those itesm
  update();
}



// Methods concerning selection; these will be given default implementations, but must be overridden
// to have function
void VisualPhysicsActor::set_selected(bool selected) {
}

bool VisualPhysicsActor::selected() {
  return false;
}

void VisualPhysicsActor::set_path(QString path) {
}

const QString& VisualPhysicsActor::path() {
  return EMPTY_QSTRING;
}

QStringList VisualPhysicsActor::pathsOfDescendants() {
  return QStringList();
}

bool VisualPhysicsActor::breakable() {
  return false;
}

bool VisualPhysicsActor::pinnable() {
  return false;
}

void VisualPhysicsActor::breakPile() {
}

void VisualPhysicsActor::stackViewAndScaleChildren(Ogre::Real scale_factor) {
}

void VisualPhysicsActor::breakPileWithoutConstrainingFinalPoses() {
}

void VisualPhysicsActor::addActorToPileAndUpdatePileView(VisualPhysicsActor* actor,
                                                         Ogre::Vector3 offset_from_pile,
                                                         Ogre::Quaternion orientation) {
}

void VisualPhysicsActor::breakItemsOutOfPile(QList<VisualPhysicsActorId> actor_ids) {
}

void VisualPhysicsActor::breakAllItemsExceptDummy() {
}

QList<VisualPhysicsActorId> VisualPhysicsActor::children_ids() {
  return QList<VisualPhysicsActorId>();
}

VisualPhysicsActorList VisualPhysicsActor::children() {
  return VisualPhysicsActorList();
}

VisualPhysicsActorList VisualPhysicsActor::flattenedChildren() {
  return VisualPhysicsActorList();
}

QList<VisualPhysicsActorId> VisualPhysicsActor::flattenedChildrenIds() {
  return QList<VisualPhysicsActorId>();
}

QList<Ogre::Vector3> VisualPhysicsActor::children_offsets() {
  return QList<Ogre::Vector3>();
}

QList<Ogre::Quaternion> VisualPhysicsActor::children_orientations() {
  return QList<Ogre::Quaternion>();
}

void VisualPhysicsActor::writeToBuffer(VisualPhysicsActorBuffer* buffer) {
}

bool VisualPhysicsActor::serializable() {
  return false;
}

bool VisualPhysicsActor::initFromBuffer(VisualPhysicsActorBuffer* buffer, bool physics_enabled) {
  assert(false && "initFromBuffer for Visual Physics actor is calling the default implementation");
  return false;
}

RoomSurface* VisualPhysicsActor::room_surface() {
  return room_surface_;
}

void VisualPhysicsActor::set_room_surface(RoomSurface* room_surface) {
  room_surface_ = room_surface;
}

void VisualPhysicsActor::setCollisionsToOnlyWalls() {
  if (physics_actor_ != NULL)
    physics_actor_->setCollisionsToOnlyWalls();
}

void VisualPhysicsActor::setCollisionsToAllActors() {
  if (physics_actor_ != NULL)
    physics_actor_->setCollisionsToAllActors();
}

void VisualPhysicsActor::setCollisionsToGriddedPiles() {
  if (physics_actor_ != NULL)
    physics_actor_->setCollisionsToGriddedPiles();
}

bool VisualPhysicsActor::isWall() {
  return false;
}

int16_t VisualPhysicsActor::collision_group() {
  if (physics_actor_ != NULL)
    return physics_actor_->collision_group();
  else
    return NULL;
}

int16_t VisualPhysicsActor::collision_mask() {
  if (physics_actor_ != NULL)
    return physics_actor_->collision_mask();
  else
    return NULL;
}

bool VisualPhysicsActor::collides_with_walls_only() {
  if (physics_actor_ != NULL)
    return physics_actor_->collides_with_walls_only();
  else
    return false;
}

bool VisualPhysicsActor::collides_with_gridded_piles() {
  if (physics_actor_ != NULL)
    return physics_actor_->collides_with_gridded_piles();
  else
    return false;
}

void VisualPhysicsActor::setPhysicsConstraintsForSurface(RoomSurface* surface) {
}

BumpPose VisualPhysicsActor::getPoseForSurface(RoomSurface* surface) {
  return BumpPose();
}

void VisualPhysicsActor::pinToSurface(RoomSurface* surface) {
}

void VisualPhysicsActor::removePhysicsConstraints() {
  if (physics_actor_ != NULL)
    physics_actor_->removeConstraints();
}

BumpPose VisualPhysicsActor::getPoseConstrainedToRoomAndNoIntersections() {
  return getActorPoseConstrainedToRoomAndNoIntersections(this, room_surface()->room());
}

void VisualPhysicsActor::setPoseConstrainedToRoomAndNoIntersections() {
  // ensure this actor belongs directly to the room
  if (parent_ == NULL) {
    if (room_surface_->is_pinnable_receiver())
      removePhysicsConstraints();

    set_pose(getPoseConstrainedToRoomAndNoIntersections());

    if (room_surface_->is_pinnable_receiver())
      pinToSurface(room_surface_);
  }
}

Ogre::Vector2 VisualPhysicsActor::labelPositionForCurrentPosition() {
  return Ogre::Vector2::ZERO;
}

void VisualPhysicsActor::updateLabelPosition() {
}

void VisualPhysicsActor::updateLabel(Ogre::Real size_factor) {
}

BumpBoxLabel* VisualPhysicsActor::label() {
  return NULL;
}

void VisualPhysicsActor::set_label_visible(bool label_visible) {
  // don't do anything (we don't implement labels at this level)
}

bool VisualPhysicsActor::label_visible() {
  return true;
}

void VisualPhysicsActor::set_name_hidden(bool name_hidden) {
  name_hidden_ = name_hidden;
}

bool VisualPhysicsActor::is_an_image_on_wall() {
  return false;
}

bool VisualPhysicsActor::name_hidden() {
  return false;
}

BumpBoxLabelColour VisualPhysicsActor::label_colour() {
  if (label() == NULL) {
    return COLOURLESS;
  }
  return label()->label_colour();
}

void VisualPhysicsActor::set_label_colour(BumpBoxLabelColour label_colour) {
  if (label() != NULL) {
    label()->set_label_colour(label_colour);
  }
}

VisualPhysicsActor* VisualPhysicsActor::lowest_child_with_visual_actor() {
  return NULL;
}

void VisualPhysicsActor::rename(QString new_name) {
}

bool VisualPhysicsActor::nameable() {
  return false;
}

QString VisualPhysicsActor::display_name() {
  return "";
}

// Methods for changing properties
void VisualPhysicsActor::set_size(Ogre::Vector3 size) {
  visual_actor_->set_scale(size);
  if (physics_actor_ != NULL)
    physics_actor_->set_scale((1/PHYSICS_SCALE)*size);
}

void VisualPhysicsActor::setSizeForGrowOrShrink(Ogre::Vector3 size) {
  set_size(size);
  BumpTopApp::singleton()->markGlobalStateAsChanged();
}

Ogre::Vector3 VisualPhysicsActor::scale() {
  return visual_actor_->scale();
}

void VisualPhysicsActor::set_alpha(Ogre::Real alpha) {
  alpha_ = alpha;
  const std::string material_name = utf8(visual_actor_->material_name());
  Ogre::MaterialPtr material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(material_name));
  if (!material.isNull()) {
    material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(Ogre::LBX_MODULATE,
                                                                                     Ogre::LBS_TEXTURE,
                                                                                     Ogre::LBS_MANUAL,
                                                                                     0.0,
                                                                                     alpha);
  }
  update();
  BumpTopApp::singleton()->markGlobalStateAsChanged();
}

void VisualPhysicsActor::setMaterialBlendFactors(Ogre::Vector3 blend_factor) {
  QString material_name = visual_actor_->material_name();
  Ogre::MaterialPtr material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(utf8(material_name)));
  if (!material.isNull()) {
    material->setAmbient(blend_factor.x, blend_factor.y, blend_factor.z);
    BumpTopApp::singleton()->markGlobalStateAsChanged();
  }
}

Ogre::Real VisualPhysicsActor::alpha() {
  return alpha_;
}

bool VisualPhysicsActor::is_new_items_pile() {
  return false;
}

VisualPhysicsActorId VisualPhysicsActor::adjacent_actor(ArrowKey arrow_key) {
  if (parent() != NULL) {
    return parent()->adjacentActorOfChild(unique_id(), arrow_key);
  }

  return room_surface()->room()->adjacentActorOfRoomActor(unique_id(), arrow_key);
}

VisualPhysicsActorId VisualPhysicsActor::adjacentActorOfChild(VisualPhysicsActorId child_id, ArrowKey arrow_key) {
  return 0;
}

void VisualPhysicsActor::revealChild(VisualPhysicsActorId child_id) {
}

void VisualPhysicsActor::launch() {
}

void VisualPhysicsActor::launch(QString app) {
}

void VisualPhysicsActor::closeView() {
}

// Methods for getting properties
Ogre::Vector3 VisualPhysicsActor::size() {
  return visual_actor_->scale() * toOgre(physicsSize());
}

Ogre::Vector3 VisualPhysicsActor::visual_size() {
  return visual_actor_->scale() * absoluteMeshSizeDividedBy100();
}

// Sorting children
void VisualPhysicsActor::sortAlphabetically() {
}

// Methods for changing orientation
void VisualPhysicsActor::set_orientation(const Ogre::Quaternion &q) {
  visual_actor_->set_orientation(q);
  update();
}

void VisualPhysicsActor::set_orientation_no_physics(const Ogre::Quaternion &q) {
  visual_actor_->set_orientation(q);
}

void VisualPhysicsActor::setFriction(Ogre::Real friction) {
  if (physics_actor_ != NULL)
    physics_actor_->setFriction(friction);
}

void VisualPhysicsActor::setMass(Ogre::Real mass) {
  if (physics_actor_ != NULL)
    physics_actor_->setMass(mass);
}

void VisualPhysicsActor::reset_orientation(void) {
  visual_actor_->reset_orientation();
  update();
}
void VisualPhysicsActor::setDirection(Ogre::Real x, Ogre::Real y, Ogre::Real z, Ogre::Node::TransformSpace relativeTo,
                       const Ogre::Vector3 &localDirectionVector) {
  visual_actor_->setDirection(x, y, z, relativeTo, localDirectionVector);
  update();
}

void VisualPhysicsActor::setDirection(Ogre::Vector3 x, Ogre::Node::TransformSpace relativeTo,
                       const Ogre::Vector3 &localDirectionVector) {
  visual_actor_->setDirection(x, relativeTo, localDirectionVector);
  update();
}

void VisualPhysicsActor::lookAt(const Ogre::Vector3 &targetPoint, Ogre::Node::TransformSpace relativeTo,
                 const Ogre::Vector3 &localDirectionVector) {
  visual_actor_->lookAt(targetPoint, relativeTo, localDirectionVector);
  update();
}

void VisualPhysicsActor::roll(const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo) {
  visual_actor_->roll(angle, relativeTo);
  update();
}

void VisualPhysicsActor::pitch(const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo) {
  visual_actor_->pitch(angle, relativeTo);
  update();
}

void VisualPhysicsActor::yaw(const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo) {
  visual_actor_->yaw(angle, relativeTo);
  update();
}

void VisualPhysicsActor::rotate(const Ogre::Vector3 &axis, const Ogre::Radian &angle,
                                Ogre::Node::TransformSpace relativeTo) {
  visual_actor_->rotate(axis, angle, relativeTo);
  update();
}

void VisualPhysicsActor::rotate(const Ogre::Quaternion &q, Ogre::Node::TransformSpace relativeTo) {
  visual_actor_->rotate(q, relativeTo);
  update();
}

// Methods for changing position
void VisualPhysicsActor::set_pose(BumpPose pose) {
  set_position(pose.position);
  set_orientation(pose.orientation);
}

void VisualPhysicsActor::set_position(const Ogre::Vector3 &pos) {
  visual_actor_->set_position(pos);
  update();
}

void VisualPhysicsActor::set_position_no_physics(const Ogre::Vector3 &pos) {
  visual_actor_->set_position(pos);
}

void VisualPhysicsActor::translate(const Ogre::Vector3 &d, Ogre::Node::TransformSpace relativeTo) {
  visual_actor_->translate(d, relativeTo);
  update();
}

void VisualPhysicsActor::translate(Ogre::Real x, Ogre::Real y, Ogre::Real z, Ogre::Node::TransformSpace relativeTo) {
  visual_actor_->translate(x, y, z, relativeTo);
  update();
}

void VisualPhysicsActor::translate(const Ogre::Matrix3 &axes, const Ogre::Vector3 &move,
                                   Ogre::Node::TransformSpace relativeTo) {
  visual_actor_->translate(axes, move, relativeTo);
  update();
}

void VisualPhysicsActor::translate(const Ogre::Matrix3 &axes, Ogre::Real x, Ogre::Real y, Ogre::Real z,
                    Ogre::Node::TransformSpace relativeTo) {
  visual_actor_->translate(axes, x, y, z, relativeTo);
  update();
}

void VisualPhysicsActor::set_angular_velocity(Ogre::Vector3 angular_velocity) {
  if (physics_actor_ != NULL)
    physics_actor_->set_angular_velocity(angular_velocity);
}

void VisualPhysicsActor::set_linear_velocity(Ogre::Vector3 linear_velocity) {
  if (physics_actor_ != NULL)
    physics_actor_->set_linear_velocity(1/(PHYSICS_SCALE)*linear_velocity);
}

void VisualPhysicsActor::set_material_name(const QString& material_name) {
  visual_actor_->set_material_name(material_name);
}

const QString& VisualPhysicsActor::material_name() {
  return visual_actor_->material_name();
}

void VisualPhysicsActor::set_render_queue_group_for_mouse_up(uint8 queue_id) {
}

void VisualPhysicsActor::set_visible(bool visible) {
  visual_actor_->set_visible(visible);
}

void VisualPhysicsActor::set_render_queue_group(uint8 queue_id) {
  visual_actor_->set_render_queue_group(queue_id);
}

uint8 VisualPhysicsActor::render_queue_group() {
  return visual_actor_->render_queue_group();
}

// Getters
BumpPose VisualPhysicsActor::pose() {
  BumpPose pose = BumpPose(position(), orientation());
  return pose;
}

const Ogre::Quaternion& VisualPhysicsActor::orientation() const {
  return visual_actor_->orientation();
}

const Ogre::Vector3& VisualPhysicsActor::position() const {
  return visual_actor_->position();
}

const Ogre::Vector3 VisualPhysicsActor::world_position() {
  if (visual_actor_ != NULL)
    return visual_actor_->world_position();
  else
    return Ogre::Vector3::ZERO;
}

Ogre::Vector3 VisualPhysicsActor::linear_velocity() {
  if (physics_actor_ != NULL)
    return PHYSICS_SCALE*physics_actor_->linear_velocity();
  else
    return Ogre::Vector3::ZERO;
}

Ogre::Vector3 VisualPhysicsActor::angular_velocity() {
  if (physics_actor_ != NULL)
    return physics_actor_->angular_velocity();
  else
    return Ogre::Vector3::ZERO;
}

Ogre::Vector2 VisualPhysicsActor::getScreenPosition() {
  return visual_actor_->getScreenPosition();
}


Ogre::AxisAlignedBox VisualPhysicsActor::screenBoundingBox() {
  std::vector<Ogre::Vector2> corners = getCornersInScreenSpace();
  Ogre::AxisAlignedBox box;
  bool first_pass = true;
  for_each(Ogre::Vector2 corner, corners) {
    if (first_pass) {
      box = Ogre::AxisAlignedBox(corner.x, corner.y, -1, corner.x, corner.y, 1);
    } else {
      if (!box.contains(Ogre::Vector3(corner.x, corner.y, 0))) {
        if (corner.x < (box.getMinimum()).x) {
          box.setMinimumX(std::max(0.0f, corner.x));
        } else if (corner.x > (box.getMaximum()).x) {
          box.setMaximumX(corner.x);
        }
        if (corner.y < (box.getMinimum()).y) {
          box.setMinimumY(std::max(0.0f, corner.y));
        } else if (corner.y > (box.getMaximum()).y) {
          box.setMaximumY(corner.y);
        }
      }
    }
    first_pass = false;
  }
  return box;
}

std::vector<Ogre::Vector2> VisualPhysicsActor::getCornersInScreenSpace() {
  std::vector<Ogre::Vector2> corners;
  Ogre::AxisAlignedBox box = world_bounding_box();
  const Ogre::Vector3* world_corners = box.getAllCorners();

  for (int i = 0; i < 8; i++) {
    corners.push_back(worldPositionToScreenPosition(world_corners[i]));
  }
  return corners;
}

Ogre::SceneNode* VisualPhysicsActor::ogre_scene_node() {
  return visual_actor_->ogre_scene_node();
}

Ogre::SceneNode* VisualPhysicsActor::ogre_scene_node_for_children() {
  return visual_actor_->ogre_scene_node();
}

VisualActor* VisualPhysicsActor::visual_actor() {
  return visual_actor_;
}

PhysicsActor* VisualPhysicsActor::physics_actor() {
  return physics_actor_;
}

// This is only an approximation, is it doesn't take into account the final
// orientation of the actor, only its final position
Ogre::AxisAlignedBox VisualPhysicsActor::destinationWorldBoundingBox() {
  Ogre::AxisAlignedBox wbb = world_bounding_box();
  Ogre::Vector3 position_delta = destination_pose().position - position();
  Ogre::Matrix4 translation;
  translation.makeTrans(position_delta);

  wbb.transform(translation);
  return wbb;
}

Ogre::AxisAlignedBox VisualPhysicsActor::world_bounding_box() {
  // By default we return the bounding box of the physics actor, however, if there is no physics actor,
  // then we look to the visual actors the bounding box, and if it has no entity we return the all-zeros box
  if (physics_actor_ != NULL) {
    Ogre::AxisAlignedBox physics_box = physics_actor_->world_bounding_box();
    return Ogre::AxisAlignedBox(physics_box.getMinimum()*PHYSICS_SCALE, physics_box.getMaximum()*PHYSICS_SCALE);
  } else if (visual_actor_->world_bounding_box() != Ogre::AxisAlignedBox::BOX_NULL) {
    return visual_actor_->world_bounding_box();
  } else {
    return Ogre::AxisAlignedBox::BOX_NULL;
  }
}

Ogre::Plane VisualPhysicsActor::plane() {
  if (visual_actor_ != NULL) {
    return visual_actor_->plane();
  }
  return Ogre::Plane();
}

Ogre::Entity* VisualPhysicsActor::_entity() {
  return visual_actor_->_entity();
}

Ogre::Real VisualPhysicsActor::mass() {
  if (physics_actor_ != NULL)
    return physics_actor_->mass();
  else
    return 0.0;
}

Ogre::Vector2 VisualPhysicsActor::actor_screen_position_before_drag() {
  return Ogre::Vector2::ZERO;
}

BumpPose VisualPhysicsActor::actor_pose_before_drag() {
  return BumpPose(Ogre::Vector3::ZERO, Ogre::Quaternion::IDENTITY);
}

RoomSurface* VisualPhysicsActor::actor_room_surface_before_drag() {
  return room_surface()->room()->getSurface(FLOOR);
}

void VisualPhysicsActor::updateActorStatusBeforeDrag() {
}

VisualPhysicsActorId VisualPhysicsActor::actor_parent_id_before_drag() {
  return 0;
}

VisualPhysicsActorType VisualPhysicsActor::actor_parent_type_before_drag() {
  return BOX;
}

Ogre::Vector3 VisualPhysicsActor::actor_parent_position_before_drag() {
  return Ogre::Vector3::ZERO;
}

BumpPose VisualPhysicsActor::actor_offset_pose_to_its_parent() {
  return BumpPose(Ogre::Vector3::ZERO, Ogre::Quaternion::IDENTITY);
}

QHash<VisualPhysicsActorId, BumpPose> VisualPhysicsActor::actor_siblings_offset_poses_to_parent() {
  return QHash<VisualPhysicsActorId, BumpPose>();
}

void VisualPhysicsActor::updateActorParentInfoBeforeDrag() {
}

void VisualPhysicsActor::updateActorOffsetPoseToItsParentBeforeDrag() {
}

void VisualPhysicsActor::updateActorSiblingOffsetPoseToParentBeforeDrag() {
}

BumpPose VisualPhysicsActor::children_offset_pose(VisualPhysicsActor* actor) {
  return BumpPose(Ogre::Vector3::ZERO, Ogre::Quaternion::IDENTITY);
}

void VisualPhysicsActor::set_children_offset_pose(QHash<VisualPhysicsActorId, BumpPose> children_ids_to_offset_poses) {
}

void VisualPhysicsActor::mouseMotionRegistered() {
  emit onMouseMotionRegistered(this);
}

// Physics interaction methods
void VisualPhysicsActor::applyCentralImpulse(const Ogre::Vector3 &impulse) {
  if (physics_actor_ != NULL)
    physics_actor_->applyCentralImpulse(impulse);
}

void VisualPhysicsActor::activatePhysics() {
  if (physics_actor_ != NULL)
    physics_actor_->activate();
}

void VisualPhysicsActor::set_physics_enabled(bool physics_enabled) {
  if (physics_actor_ != NULL) {
    syncBulletTransformWithOgreTransform();
    physics_actor_->set_physics_enabled(physics_enabled);
  }
}

Ogre::Matrix4 VisualPhysicsActor::transform() {
  Ogre::Matrix4 trans = ogre_scene_node()->_getFullTransform();
  return trans;
}

bool VisualPhysicsActor::physics_enabled() {
  if (physics_actor_ != NULL)
    return physics_actor_->physics_enabled();
  else
    return false;
}

void VisualPhysicsActor::update() {
  syncBulletTransformWithOgreTransform();
}

void VisualPhysicsActor::poseUpdatedByPhysics(const btTransform& transform) {
  set_position_no_physics(PHYSICS_SCALE*toOgre(transform.getOrigin()));
  set_orientation_no_physics(toOgre(transform.getRotation()));
  // we need to call update on the visual actor to update its transforms; otherwise, as an example,
  //    text labels lag behind objects when they're in a gridded pile
  // TODO: this might be causing performance loss; just check if that's true
  visual_actor_->ogre_scene_node()->_update(false, false);
  pose_has_changed_ = true;
}

BumpTopCommandSet* VisualPhysicsActor::supported_context_menu_items() {
  return new BumpTopCommandSet();
}

bool VisualPhysicsActor::supportsContextMenuItem(BumpTopCommand* context_menu_option) {
  return false;
}

void VisualPhysicsActor::set_clickable_parent(Clickable* clickable_parent) {
  clickable_parent_ = clickable_parent;
  visual_actor_->set_clickable_parent(clickable_parent);
}

void VisualPhysicsActor::mouseDown(MouseEvent* mouse_event) {
}

void VisualPhysicsActor::mouseDragged(MouseEvent* mouse_event) {
}

void VisualPhysicsActor::mouseMoved(MouseEvent* mouse_event) {
}

void VisualPhysicsActor::mouseUp(MouseEvent* mouse_event) {
}

void VisualPhysicsActor::rightMouseDown(MouseEvent* mouse_event) {
}

void VisualPhysicsActor::rightMouseDragged(MouseEvent* mouse_event) {
}

void VisualPhysicsActor::rightMouseUp(MouseEvent* mouse_event) {
}

void VisualPhysicsActor::scrollWheel(MouseEvent* mouse_event) {
}

void VisualPhysicsActor::syncBulletTransformWithOgreTransform() {
  if (physics_actor_ != NULL)
    // TODO: not sure why we can't use the world_orientation of the visual actor here but it
    // seems to break things might have to do with the scaling component of the nodes world Quaternion
    physics_actor_->setPose(1/(PHYSICS_SCALE)*visual_actor_->world_position(), visual_actor_->orientation());
  // we need to call update on the visual actor to update its transforms; otherwise, as an example,
  //    text labels lag behind objects when they're in a gridded pile
  // TODO: this might be causing performance loss; just check if that's true
  visual_actor_->ogre_scene_node()->_update(false, false);

  pose_has_changed_ = true;
}

bool VisualPhysicsActor::isMoving() {
  return linear_velocity().length() > VELOCITY_THRESHOLD_FOR_MOTION && physics_enabled();
}

void VisualPhysicsActor::renderTick() {
  if (pose_has_changed_) {
    pose_has_changed_ = false;
    emit onPoseChanged(unique_id());
  }

  if (is_moving_ && !isMoving() && collision_mask() == (ROOM_ACTORS | ROOM_WALLS) && physics_enabled()) {
    is_moving_ = false;
    emit onStoppedMoving(unique_id_);
  } else if (!is_moving_ && isMoving() && collision_mask() == (ROOM_ACTORS | ROOM_WALLS) && physics_enabled()) {
    is_moving_ = true;
  }
}

bool compareDisplayName(VisualPhysicsActor* actor_1, VisualPhysicsActor* actor_2) {
  return QString::compare(actor_1->display_name(), actor_2->display_name(), Qt::CaseInsensitive) < 0;
}

#include "BumpTop/moc/moc_VisualPhysicsActor.cpp"
