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

#include "BumpTop/BumpBox.h"

#include <utility>
#include <string>
#include <vector>

#include "BumpTop/AppSettings.h"
#include "BumpTop/BumpBoxLabel.h"
#include "BumpTop/BumpFlatSquare.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopCommands.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/DampedSpringMouseHandler.h"
#include "BumpTop/OSX/EventModifierFlags.h"
#include "BumpTop/FileItem.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/HighlightActor.h"
#include "BumpTop/MaterialLoader.h"
#include "BumpTop/MouseEventManager.h"
#include "BumpTop/OgreBulletConverter.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/OSX/ContextMenu.h"
#include "BumpTop/PersistenceManager.h"
#include "BumpTop/protoc/AllMessages.pb.h"
#include "BumpTop/ProtocolBufferHelpers.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomItemPoseConstraints.h"
#include "BumpTop/Shape.h"
#include "BumpTop/PhysicsBoxActor.h"
#include "BumpTop/VisualActor.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"
#include "BumpTop/VisualPhysicsActorList.h"

BumpTopCommandSet* BumpBox::context_menu_items_set = MakeQSet(18,  // count, must keep this updated
                                                               Open::singleton(),
                                                               OpenWith::singleton(),
                                                               CreatePile::singleton(),
                                                               MoveToTrash::singleton(),
                                                               ShowOriginal::singleton(),
                                                               Eject::singleton(),
                                                               GetInfo::singleton(),
                                                               Duplicate::singleton(),
                                                               MakeAlias::singleton(),
                                                               Grow::singleton(),
                                                               Shrink::singleton(),
                                                               Compress::singleton(),
                                                               PileByTypeForSelectedActors::singleton(),
                                                               GridView::singleton(),
                                                               Copy::singleton(),
                                                               HideFilename::singleton(),
                                                               ShowFilename::singleton(),
                                                               ChangeLabelColour::singleton());

BumpTopCommandSet* BumpBox::supported_context_menu_items() {
  return context_menu_items_set;
}

bool BumpBox::supportsContextMenuItem(BumpTopCommand* context_menu_item) {
  return context_menu_items_set->contains(context_menu_item);
}

BumpBox::BumpBox(Ogre::SceneManager *scene_manager, Physics* physics, Room *room, VisualPhysicsActorId unique_id)
: VisualPhysicsActor(scene_manager, physics, room->ogre_scene_node(), unique_id),
  room_(room),
  is_selected_(false),
  label_(NULL),
  highlight_(NULL),
  file_item_(new FileItem("")),
  mouse_handler_(NULL),
  label_visible_(true),
  is_material_dirty_(false),
  material_loader_(NULL),
  is_dir_(false),
  label_visible_from_camera_position_(true),
  actor_parent_id_before_drag_(0) {
}

BumpBox::~BumpBox() {
  if (label_ != NULL) {
    delete label_;
  }

  if (file_item_ != NULL) {
    delete file_item_;
  }

  if (highlight_ != NULL) {
    delete highlight_;
  }

  if (mouse_handler_ != NULL) {
    delete mouse_handler_;
  }

  clearMaterialLoader();
}

// Implementing abstract members of VisualPhysicsActor
std::string BumpBox::meshName() {
  return "Prefab_Cube";
}

btVector3 BumpBox::physicsSize() {
  return btVector3(1.0, 1.0, 1.0);
}

Ogre::Vector3 BumpBox::absoluteMeshSizeDividedBy100() {
  return Ogre::Vector3(1.0, 1.0, 1.0);
}

void BumpBox::makePhysicsActor(bool physics_enabled) {
  physics_actor_ = new PhysicsBoxActor(physics_, 1.0, toOgre(physicsSize()), physics_enabled);
}

VisualPhysicsActorType BumpBox::actor_type() {
  return BUMP_BOX;
}
// end: Implementing abstract members of VisualPhysicsActor

void BumpBox::init() {
  VisualPhysicsActor::init();
  set_room_surface(room_->getSurface(FLOOR));
  setPhysicsConstraintsForSurface(room_surface_);

  // We first set a default icon material which has already been loaded
  set_material_name(AppSettings::singleton()->global_material_name(DEFAULT_ICON));

  assert(QObject::connect(visual_actor_, SIGNAL(onDraggingEntered(MouseEvent*)),  // NOLINT
                          this, SLOT(draggingEntered(MouseEvent*))));  // NOLINT
  assert(QObject::connect(visual_actor_, SIGNAL(onDraggingUpdated(MouseEvent*)),  // NOLINT
                          this, SLOT(draggingUpdated(MouseEvent*))));  // NOLINT

  mouse_handler_ = new DampedSpringMouseHandler(this, room_);
  drop_receiver_ = new FileDropReceiver(this);
  assert(QObject::connect(drop_receiver_, SIGNAL(onPerformDragOperation()),  // NOLINT
                          this, SLOT(performDragOperation())));  // NOLINT

  Ogre::String entity_name = "BumpBoxHighlight" + addressToString(this);
  highlight_ = new HighlightActor(scene_manager_, visual_actor_->ogre_scene_node(), visual_size().y);
  highlight_->set_to_render_before(visual_actor());
}

void BumpBox::initWithPath(QString file_path, bool physics_enabled) {
  init();
  set_path(file_path);
  if (FileManager::getFileKind(file_path) == ALIAS) {
    QString original_path = QFileInfo(file_path).readLink();
    is_dir_ = QFileInfo(original_path).isDir();
  } else {
    is_dir_ = QFileInfo(file_path).isDir();
  }

  updateIcon();
  updateLabel(1);

  room_->addActor(this);
}

void BumpBox::initAsVisualCopyOfActor(VisualPhysicsActor* actor) {
  VisualPhysicsActor::initAsVisualCopyOfActor(actor);

  std::string material_name = "copied material" + addressToString(this);

  Ogre::MaterialPtr source_material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(utf8(actor->visual_actor()->material_name())));  // NOLINT
  if (!source_material.isNull()) {
    if (source_material->getTechnique(0)->getPass(0)->getNumTextureUnitStates() > 0) {
      Ogre::TexturePtr texture = source_material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->_getTexturePtr();
      if (!texture.isNull()) {
        if (texture->isLoaded()) {
          Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(material_name,
                                                                                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);  // NOLINT
          Ogre::Pass *texture_pass = material->getTechnique(0)->getPass(0);
          texture_pass->createTextureUnitState(texture->getName());
          texture_pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
          texture_pass->setDepthCheckEnabled(false);
          set_material_name(QStringFromUtf8(material_name));
        }
      }
    }
  }
}

VisualPhysicsActor* BumpBox::createVisualCopyOfSelf() {
  // create a visual copy of the actor
  BumpBox* visual_copy_of_actor = constructCopyOfMyType();
  visual_copy_of_actor->initAsVisualCopyOfActor(this);
  visual_copy_of_actor->set_size(this->size());
  visual_copy_of_actor->set_position(this->world_position());
  visual_copy_of_actor->set_orientation(this->orientation());
  return visual_copy_of_actor;
}

void BumpBox::updateIcon() {
  if (!file_item_->path().isEmpty()) {
    if (material_loader_ != NULL) {
      // we need to wait until the current icon load is done
      // when the current icon loader finishes, it will just call update icon again
      is_material_dirty_ = true;
    } else {
      is_material_dirty_ = false;
      material_loader_ = new MaterialLoader();
      material_loader_->initAsIconForFilePath(file_item_->path(), MaterialLoader::kStandardIcon);
      assert(QObject::connect(material_loader_, SIGNAL(backgroundLoadingComplete(MaterialLoader*)),  // NOLINT
                              this, SLOT(loadQuickLookIcon(MaterialLoader*))));  // NOLINT
      assert(QObject::connect(material_loader_, SIGNAL(backgroundLoadingComplete(MaterialLoader*)),  // NOLINT
                              this, SLOT(setMaterialNameAndDeleteMaterialLoader(MaterialLoader*))));  // NOLINT
    }
  }
}

void BumpBox::updateLabel(Ogre::Real size_factor) {
  BumpBoxLabelColour colour = label_colour();
  deleteLabel();
  QString label_text = display_name();
  if (label_text != "") {
    label_ = new BumpBoxLabel(label_text, this);
    set_label_colour(colour);
    if (size_factor == -1) {
      if (size().x == 0) {
        label_->init(1);
      } else {
        label_->init(1 + (size().x / kInitialActorSize - 1)/2);
      }
    } else {
      label_->init(size_factor);
    }
    label_->set_render_queue_group(render_queue_group());
    label_->set_to_render_after(visual_actor());
    label_->set_visible(label_visible_);
    updateLabelPosition();
    if (is_selected_) {
      label_->set_selected(true);
    }
    assert(QObject::connect(label_, SIGNAL(onMouseDown(MouseEvent*)),  // NOLINT
                            this, SLOT(labelClicked(MouseEvent*))));  // NOLINT
  }
}

void BumpBox::deleteLabel() {
  if (label_ != NULL)
    delete label_;
  label_ = NULL;
}

void BumpBox::clearMaterialLoader() {
  if (material_loader_ != NULL) {
    material_loader_->disconnect(this);
    material_loader_->set_delete_self_on_load_complete(true);
    material_loader_ = NULL;
  }
}

void BumpBox::update() {
  VisualPhysicsActor::update();
  updateLabelPosition();
}

bool BumpBox::initFromBuffer(VisualPhysicsActorBuffer* buffer, bool physics_enabled) {
  if (buffer->file_name() == "/") {
    return false;
  }
  QString file_name = QStringFromUtf8(buffer->file_name());
  if (!QFileInfo(file_name).exists()) {
    return false;
  }
  // temporary measure to get rid of items that were in the scene file but shouldn't have
  // TODO: eliminate this code in a future version
  if (!file_name.startsWith(room_->path()) &&
      !file_name.startsWith(FileManager::getApplicationDataPath() + "OtherDesktopItems/") &&
      !FileManager::isVolume(file_name)) {
    return false;
  }
  initWithPath(file_name, physics_enabled);

  set_position(Vector3BufferToVector3(buffer->position()));
  set_orientation(QuaternionBufferToQuaternion(buffer->orientation()));
  set_size(Vector3BufferToVector3(buffer->size()));
  if (buffer->label_colour() == -1) {
    FileManager::getAndSetLabelColourThroughNSTask(unique_id());
  } else {
    if (buffer->label_colour() == COLOURLESS) {
      FileManager::getAndSetLabelColourThroughNSTask(unique_id());
    } else {
      set_label_colour((BumpBoxLabelColour)(buffer->label_colour()));
      updateLabel();
      FileManager::setFinderLabelColourThroughNSTask(display_name(),(BumpBoxLabelColour)buffer->label_colour());
    }
  }
  set_name_hidden(buffer->name_hidden());

  if (buffer->has_room_surface()) {
    RoomSurface* surface = room_->getSurface((RoomSurfaceType)buffer->room_surface());
    set_room_surface(surface);
    if (surface->is_pinnable_receiver())
      pinToSurface(surface);
  }
  return true;
}

void BumpBox::set_path(QString path) {
  if (file_item_ != NULL)
    delete file_item_;
  file_item_ = new FileItem(path);
  assert(QObject::connect(file_item_, SIGNAL(onFileRenamed(const QString&, const QString&)),
                          this, SLOT(updateLabel())));
  assert(QObject::connect(file_item_, SIGNAL(onFileRenamed(const QString&, const QString&)),
                          this, SLOT(updateIcon())));
  assert(QObject::connect(file_item_, SIGNAL(onFileModified(const QString&)),
                          this, SLOT(updateIcon())));
  assert(QObject::connect(file_item_, SIGNAL(onFileRemoved(const QString&)),
                          this, SLOT(fileRemoved(const QString&))));
}

BumpPose BumpBox::getPoseForSurface(RoomSurface* surface) {
  Ogre::Vector3 normal = surface->normal();
  Ogre::Real pi_by_two = Ogre::Math::PI/2.0;
  BumpPose current_pose = pose();

  Ogre::Vector3 new_position = position();
  Ogre::Quaternion orientation = surface->orientation();

  if (normal == Ogre::Vector3::UNIT_Y) {
    new_position.y = 0;
  } else if (normal == Ogre::Vector3::UNIT_X) {
    new_position.x = 0;
  } else if (normal == -Ogre::Vector3::UNIT_X) {
    new_position.x = room_->floor_width();
  } else if (normal == Ogre::Vector3::UNIT_Z) {
    new_position.z = 0;
  } else if (normal == -Ogre::Vector3::UNIT_Z) {
    // the last component should ideally be  Ogre::Radian(2*pi_by_two), however the corresponding
    // physics constraint has a problem with that..
    Ogre::Matrix3 mat;
    mat.FromEulerAnglesXYZ(Ogre::Radian(pi_by_two), Ogre::Radian(0), Ogre::Radian(0));
    orientation = Ogre::Quaternion(mat);
    new_position.z = room_->floor_depth();
  }

  QHash<VisualPhysicsActorId, BumpPose> desired_pose;
  desired_pose.insert(unique_id_, BumpPose(new_position, orientation));

  desired_pose = getActorPosesConstrainedToRoom(desired_pose, room_);
  QHash<VisualPhysicsActorId, BumpPose> constrained_pose;
  constrained_pose = getActorPosesConstrainedToNoIntersections(desired_pose, room_);

  // move the item a bit further away from the wall to get rid of any jitter from the physics constraint
  // this also fixed the issue where the wall items were constantly being repositioned hence marking the scene
  // as dirty hence preventing the cpu usage from settling down
  constrained_pose[unique_id_].position += size()*normal*0.05;

  return constrained_pose[unique_id_];
}

std::pair<Ogre::Vector3, Ogre::Vector3> BumpBox::angularConstraintsForSurface(RoomSurface* surface) {
  Ogre::Vector3 normal = surface->normal();
  Ogre::Real pi_by_two = Ogre::Math::PI/2.0;

  std::pair<Ogre::Vector3, Ogre::Vector3> constraints;
  Ogre::Real jiggle_factor = 0.01;
  if (normal == Ogre::Vector3::UNIT_Y) {
    constraints.first = Ogre::Vector3(-0.8, -jiggle_factor, -0.8);
    constraints.second = Ogre::Vector3(0.8, jiggle_factor, 0.8);
  } else if (normal == Ogre::Vector3::UNIT_X) {
    constraints.first = Ogre::Vector3(pi_by_two - jiggle_factor, -0.01, -pi_by_two - 0.01);
    constraints.second = Ogre::Vector3(pi_by_two + jiggle_factor,  0.01, -pi_by_two + 0.01);
  } else if (normal == -Ogre::Vector3::UNIT_X) {
    constraints.first = Ogre::Vector3(pi_by_two - jiggle_factor, -0.01, pi_by_two - 0.01);
    constraints.second = Ogre::Vector3(pi_by_two + jiggle_factor,  0.01, pi_by_two + 0.01);
  } else if (normal == Ogre::Vector3::UNIT_Z) {
    constraints.first = Ogre::Vector3(pi_by_two - 0.01, - jiggle_factor, - 0.01);
    constraints.second = Ogre::Vector3(pi_by_two + 0.01, jiggle_factor, 0.01);
  } else if (normal == -Ogre::Vector3::UNIT_Z) {
    constraints.first = Ogre::Vector3(pi_by_two - 0.01, - jiggle_factor, - 0.01);
    constraints.second = Ogre::Vector3(pi_by_two + 0.01, jiggle_factor, 0.01);
  }
  return constraints;
}

void BumpBox::setPhysicsConstraintsForSurface(RoomSurface* surface) {
  std::pair<Ogre::Vector3, Ogre::Vector3> angular_constraints = angularConstraintsForSurface(surface);

  physics_actor_->set6DofConstraint(Ogre::Vector3(1, 1, 1),
                                    Ogre::Vector3(0, 0, 0),
                                    angular_constraints.first,
                                    angular_constraints.second);
}

void BumpBox::pinToSurface(RoomSurface* surface) {
  std::pair<Ogre::Vector3, Ogre::Vector3> angular_constraints = angularConstraintsForSurface(surface);

  BumpPose pose = getPoseForSurface(surface);
  set_position(pose.position);

  physics_actor_->set6DofConstraint(Ogre::Vector3::ZERO,
                                    Ogre::Vector3::ZERO,
                                    angular_constraints.first,
                                    angular_constraints.second);
}

void BumpBox::launch() {
  startLaunchAnimation();
  FileManager::launchPath(file_item_->path());
}

void BumpBox::launch(QString app) {
  startLaunchAnimation();
  FileManager::launchPath(file_item_->path(), app);
}

BumpBox* BumpBox::constructCopyOfMyType() {
  return new BumpBox(scene_manager_, physics_, room_, 0);
}

void BumpBox::startLaunchAnimation() {
  VisualPhysicsActor* visual_copy_of_actor = createVisualCopyOfSelf();

  // determine the final orientation so that animation goes to the camera
  Ogre::Vector3 camera_direction = BumpTopApp::singleton()->camera()->getDirection();
  Ogre::Radian angle_between = camera_direction.angleBetween(Ogre::Vector3(0, 0, 1));
  Ogre::Matrix3 matrix3 = Ogre::Matrix3();
  matrix3.FromEulerAnglesXYZ(angle_between-Ogre::Radian(Ogre::Degree(90)), Ogre::Radian(0), Ogre::Radian(0));
  Ogre::Quaternion desired_orientation = Ogre::Quaternion(matrix3);

  VisualPhysicsActorAnimation* actor_animation = new VisualPhysicsActorAnimation(visual_copy_of_actor, 400,
                                                                                 BumpTopApp::singleton()->camera()->getPosition(),  // NOLINT
                                                                                 desired_orientation, NULL, 0);
  assert(QObject::connect(actor_animation, SIGNAL(onAnimationComplete(VisualPhysicsActorAnimation*)),  // NOLINT
                          room_, SLOT(deleteActorCopyWhenFadeFinishes(VisualPhysicsActorAnimation*)))); // NOLINT
  visual_copy_of_actor->set_render_queue_group(99);
  actor_animation->start();
}

void BumpBox::set_selected(bool is_selected) {
  if (is_selected != is_selected_) {
    is_selected_ = is_selected;
    emit onSelectedChanged(unique_id_);
    if (label_ != NULL)
      label_->set_selected(is_selected);
    if (highlight_ != NULL) {
      // TODO: we don't need to do this every time, figure out why it's not happening
      highlight_->set_height_of_parent(visual_size().y);
      highlight_->set_visible(is_selected);
    }
    BumpTopApp::singleton()->markGlobalStateAsChanged();
  }
  if (is_selected) {
    if (parent() != NULL) {
      parent()->set_selected(false);
    }
    room_->set_last_selected_actor(unique_id_);
  }
}

bool BumpBox::selected() {
  return is_selected_;
}

const QString& BumpBox::path() {
  return file_item_->path();
}

void BumpBox::set_render_queue_group_for_mouse_up(uint8 queue_id) {
  render_queue_group_on_mouse_down_ = queue_id;
}

void BumpBox::mouseDown(MouseEvent* mouse_event) {
  updateActorParentInfoBeforeDrag();
  updateActorOffsetPoseToItsParentBeforeDrag();
  updateActorSiblingOffsetPoseToParentBeforeDrag();
  VisualPhysicsActor::mouseDown(mouse_event);
  room_->openNewUndoCommand();

  if (mouse_event->num_clicks == 2) {
    launch();
  } else {
    bool command_or_shift_pressed = mouse_event->modifier_flags & COMMAND_KEY_MASK
                                    || mouse_event->modifier_flags & SHIFT_KEY_MASK;
    if (!selected() && !command_or_shift_pressed) {
      room_->deselectActors();
    }
    if (selected() && command_or_shift_pressed) {
      set_selected(false);
    } else {
      set_selected(true);
      if (command_or_shift_pressed) {
        room_->updateBumpToolbar();
      }
      mouse_handler_->mouseDown(mouse_event);
    }
  }
  render_queue_group_on_mouse_down_ = render_queue_group();
  set_render_queue_group(99);
  mouse_event->handled = true;
}

void BumpBox::mouseDragged(MouseEvent* mouse_event) {
  VisualPhysicsActor::mouseDragged(mouse_event);
  mouse_handler_->mouseDragged(mouse_event);
}

void BumpBox::mouseUp(MouseEvent* mouse_event) {
  VisualPhysicsActor::mouseUp(mouse_event);
  mouse_handler_->mouseUp(mouse_event);
  if (render_queue_group() == 99) {
    // nobody else changed our render queue group, so let's just change it back
    set_render_queue_group(render_queue_group_on_mouse_down_);
  }
  emit onFinishedDragging(this);
}

void BumpBox::rightMouseDown(MouseEvent* mouse_event) {
  VisualPhysicsActor::rightMouseDown(mouse_event);

  bool command_key_pressed = mouse_event->modifier_flags & COMMAND_KEY_MASK;
  if (!command_key_pressed && !selected())
    room_->deselectActors();

  if (selected() && command_key_pressed) {
    set_selected(false);
  } else {
    if (parent() == NULL) {
      set_selected(true);
    }
    if (command_key_pressed)
      room_->updateBumpToolbar();
    if (file_item_->path() != "") {
      BumpEnvironment env(physics_, room_, scene_manager_);
      if (parent() == NULL) {
        launchContextMenu(env, room_->selected_actors(), mouse_event->mouse_in_window_space);
      } else {
        VisualPhysicsActorList context_menu_actors;
        context_menu_actors.append(this);
        launchContextMenu(env, context_menu_actors, mouse_event->mouse_in_window_space);
      }
    }
  }
  mouse_event->handled = true;
}

void BumpBox::labelClicked(MouseEvent* mouse_event) {
  mouse_event->handled = true;

  if ((mouse_event->num_clicks == 1 && selected()) || mouse_event->num_clicks == 2) {
    VisualPhysicsActorList list_with_just_me;
    list_with_just_me.append(this);
    BumpEnvironment bump_environment = BumpEnvironment(BumpTopApp::singleton()->physics(),
                                                       BumpTopApp::singleton()->scene()->room(),
                                                       BumpTopApp::singleton()->ogre_scene_manager());
    if (Rename::singleton()->canBeAppliedToActors(bump_environment, list_with_just_me)) {
      Rename::singleton()->applyToActors(bump_environment, list_with_just_me);
    }
  } else {
    bool command_or_shift_pressed = mouse_event->modifier_flags & COMMAND_KEY_MASK
                                    || mouse_event->modifier_flags & SHIFT_KEY_MASK;
    if (!selected() && !command_or_shift_pressed) {
      room_->deselectActors();
    }
    if (command_or_shift_pressed) {
      room_->updateBumpToolbar();
    }
    set_selected(true);
  }
}

void BumpBox::draggingEntered(MouseEvent* mouse_event) {
  draggingUpdated(mouse_event);
}

void BumpBox::draggingUpdated(MouseEvent* mouse_event) {
  if (mouse_event->items_being_dropped.size() != 0) {
    bool i_am_being_dragged = false;
    for_each(VisualPhysicsActor* actor, mouse_event->items_being_dropped) {
      if (i_am_being_dragged) {
        break;
      }

      if (actor == this) {
        i_am_being_dragged = true;
      }
      for_each(VisualPhysicsActor* child, actor->children()) {
        if (child == this) {
          i_am_being_dragged = true;
        }
      }
    }

    if (!i_am_being_dragged) {
      if (is_dir_) {
        if (!FileManager::arePathsOnSameVolume(path(), mouse_event->items_being_dropped[0]->path()) &&
            (mouse_event->drag_operations & NSDragOperationCopy)) {
          mouse_event->drag_operations = NSDragOperationCopy;
        } else if (mouse_event->drag_operations & NSDragOperationGeneric) {
          mouse_event->drag_operations = NSDragOperationGeneric;
        } else if (mouse_event->drag_operations & NSDragOperationMove) {
          mouse_event->drag_operations = NSDragOperationMove;
        } else if (mouse_event->drag_operations & NSDragOperationCopy) {
          mouse_event->drag_operations = NSDragOperationCopy;
        } else if (mouse_event->drag_operations & NSDragOperationLink) {
          mouse_event->drag_operations = NSDragOperationLink;
        } else {
          mouse_event->drag_operations = NSDragOperationNone;
        }

        mouse_event->handled = true;
        mouse_event->drop_receiver = drop_receiver_;
        if (highlight_ != NULL) {
          // TODO: we don't need to do this every time, figure out why it's not happening
          highlight_->set_height_of_parent(visual_size().y);
          highlight_->set_visible(true);
        }
      } else {
        // for the case of a regular bumpbox, handle the event but don't include any dropreceiver,
        //      basically to show you can't drag to this (but to prevent the event from continuing to travel)
        mouse_event->handled = true;
      }
    }
  }
}

void BumpBox::performDragOperation() {
  if (highlight_ != NULL) {
    highlight_->set_visible(false);
  }
}

void BumpBox::draggingExited() {
  if (highlight_ != NULL) {
    highlight_->set_visible(false);
  }
}

Ogre::Vector2 BumpBox::labelPositionForCurrentPosition() {
  Ogre::AxisAlignedBox bounding_box = screenBoundingBox();

  Ogre::Vector2 screen_position;
  std::vector<Ogre::Vector2> corners = getCornersInScreenSpace();
  qSort(corners.begin(), corners.end(), yLessThanVec2);
  Ogre::Real y = corners[7].y;
  QList<Ogre::Vector2> corner_list;
  corner_list.push_back(corners[7]);
  corner_list.push_back(corners[6]);
  corner_list.push_back(corners[5]);
  corner_list.push_back(corners[4]);
  qSort(corner_list.begin(), corner_list.end(), xLessThan);
  Ogre::Real x = (corner_list[0].x + corner_list[3].x)/2.0;

  return Ogre::Vector2(x, y);
}

void BumpBox::updateLabelPosition() {
  if (label_ != NULL && label_visible_) {
    label_->set_position_in_pixel_coords(labelPositionForCurrentPosition());
    Ogre::Camera* camera = BumpTopApp::singleton()->camera();
    // We don't want to show labels for items behind the camera or being clipped
    if ((world_position() - camera->getPosition()).dotProduct(camera->getDirection()) < 0 ||
        (world_position() - camera->getPosition()).length() < camera->getNearClipDistance() ||
        BumpTopApp::singleton()->scene()->surface_that_camera_is_zoomed_to() == FLOOR && room_surface()->is_pinnable_receiver()) {  // NOLINT
      set_label_visible_from_camera_position(false);
    } else {
      set_label_visible_from_camera_position(true);
    }
  }
}

void BumpBox::set_label_visible_from_camera_position(bool visible) {
  if (label_visible_from_camera_position_ != visible) {
    label_visible_from_camera_position_ = visible;
    if (!label_visible_from_camera_position_ && label_visible_) {
      if (label_ != NULL)
        label_->set_visible(false);
    } else if (label_visible_from_camera_position_ && label_visible_) {
      if (label_ != NULL)
        label_->set_visible(true);
    }
  }
}

void BumpBox::set_label_visible(bool label_visible) {
  if (label_visible_ && !label_visible) {
    if (label_ != NULL)
      label_->set_visible(false);
  } else if (!label_visible_ && label_visible) {
    if (label_ != NULL && label_visible_from_camera_position_) {
      label_->set_visible(true);
    }
  }
  label_visible_ = label_visible;
  updateLabelPosition();

  BumpTopApp::singleton()->markGlobalStateAsChanged();
}

bool BumpBox::label_visible() {
  return label_visible_;
}

void BumpBox::set_room_surface(RoomSurface* room_surface) {
  VisualPhysicsActor::set_room_surface(room_surface);

  if (AppSettings::singleton()->image_name_on_walls_hidden()
      && is_an_image_on_wall()
      || name_hidden_) {
    set_label_visible(false);
  } else {
    set_label_visible(true);
  }
}

bool BumpBox::is_an_image_on_wall() {
  QString extension = QFileInfo(file_item_->path()).suffix();
  if (room_surface_->room_surface_type() != FLOOR
      && AppSettings::singleton()->image_extensions().contains(extension)) {
    return true;
  }
  return false;
}

void BumpBox::set_name_hidden(bool name_hidden) {
  name_hidden_ = name_hidden;
  set_label_visible(!name_hidden_);
}

bool BumpBox::name_hidden() {
  return name_hidden_;
}

bool BumpBox::nameable() {
  if (FileManager::isVolume(path())) {
    return false;
  }
  return true;
}

void BumpBox::rename(QString new_name) {
  if (new_name[0] == QChar('.')) {
    return;
  }
  file_item_->rename(new_name);
}

void BumpBox::loadQuickLookIcon(MaterialLoader* material_loader) {
  clearMaterialLoader();
  material_loader_ = new MaterialLoader();
  material_loader_->initAsIconForFilePath(file_item_->path(), MaterialLoader::kQuickLookWithStandardIconFallback);
  assert(QObject::connect(material_loader_, SIGNAL(backgroundLoadingComplete(MaterialLoader*)),  // NOLINT
                          this, SLOT(setMaterialNameAndDeleteMaterialLoader(MaterialLoader*))));  // NOLINT
}

void BumpBox::setMaterialNameAndDeleteMaterialLoader(MaterialLoader* material_loader) {
  set_material_name(material_loader->name());
  if (material_loader_ == material_loader) {
    clearMaterialLoader();
  }
  delete material_loader;
  if (is_material_dirty_) {
    // we need to update the icon again to ensure we have the newest one
    // updateIcon();
  }
}

void BumpBox::writeToBuffer(VisualPhysicsActorBuffer* buffer) {
  buffer->set_actor_type(actor_type());
  Vector3ToBuffer(position(), buffer->mutable_position());
  QuaternionToBuffer(orientation(), buffer->mutable_orientation());
  Vector3ToBuffer(scale(), buffer->mutable_size());
  buffer->set_file_name(utf8(file_item_->path()));
  if (room_surface() != NULL)
    buffer->set_room_surface(room_surface()->room_surface_type());
  buffer->set_name_hidden(name_hidden_);
  buffer->set_label_colour(-1);
}

bool BumpBox::serializable() {
  return true;
}

void BumpBox::fileRemoved(const QString& path) {
  emit onRemoved(unique_id_);
}

void BumpBox::set_size(Ogre::Vector3 size) {
  VisualPhysicsActor::set_size(size);
  updateLabelPosition();
  if (highlight_ != NULL)
    highlight_->set_height_of_parent(visual_size().y);
}

void BumpBox::set_position_no_physics(const Ogre::Vector3 &pos) {
  VisualPhysicsActor::set_position_no_physics(pos);
  updateLabelPosition();
}

void BumpBox::set_surface_and_position(RoomSurface* surface, Ogre::Vector3 position) {
  set_room_surface(surface);
  if (surface->is_pinnable_receiver()) {
    set_orientation(getPoseForSurface(surface).orientation);
    set_position(position);
    pinToSurface(surface);
  } else {
    set_pose(getActorPoseConstrainedToRoomAndNoIntersections(this, room_));
    set_position(Ogre::Vector3(position.x, 50, position.z));
  }
}

void BumpBox::set_parent(VisualPhysicsActor* parent) {
  if (parent_ != NULL)
    parent_->disconnect(this);
  VisualPhysicsActor::set_parent(parent);
  updateLabelPosition();
  // We are now linking the parent's onPoseChanged signal to update as opposed to
  // to updateLabelPosition. This is because we need to update the physics actor
  // in order to properly update the label position.
  if (parent != NULL)
    assert(QObject::connect(parent, SIGNAL(onPoseChanged(VisualPhysicsActorId)),
                            this, SLOT(update())));
}

QString BumpBox::display_name() {
  NSString* path = NSStringFromQString(file_item_->path());
  NSFileManager* file_manager = [NSFileManager defaultManager];
  NSString* name = [file_manager displayNameAtPath:path];

  return QStringFromNSString(name);
}

void BumpBox::set_render_queue_group(uint8 queue_id) {
  VisualPhysicsActor::set_render_queue_group(queue_id);
  if (label_ != NULL)
    label_->set_render_queue_group(queue_id);
  if (highlight_ != NULL)
    highlight_->set_render_queue_group(queue_id);
}

BumpBoxLabel* BumpBox::label() {
  return label_;
}

VisualPhysicsActor* BumpBox::lowest_child_with_visual_actor() {
  return this;
}

Ogre::Vector2 BumpBox::actor_screen_position_before_drag() {
  return screen_position_before_drag_;
}

BumpPose BumpBox::actor_pose_before_drag() {
  return pose_before_drag_;
}

RoomSurface* BumpBox::actor_room_surface_before_drag() {
  return room_surface_before_drag_;
}

void BumpBox::updateActorStatusBeforeDrag() {
  screen_position_before_drag_ = getScreenPosition();
  pose_before_drag_ = pose();
  room_surface_before_drag_ = room_surface();
}

VisualPhysicsActorId BumpBox::actor_parent_id_before_drag() {
  return actor_parent_id_before_drag_;
}

VisualPhysicsActorType BumpBox::actor_parent_type_before_drag() {
  return actor_parent_type_before_drag_;
}

Ogre::Vector3 BumpBox::actor_parent_position_before_drag() {
  return actor_parent_position_before_drag_;
}

BumpPose BumpBox::actor_offset_pose_to_its_parent() {
  return actor_offset_pose_to_its_parent_;
}

QHash<VisualPhysicsActorId, BumpPose> BumpBox::actor_siblings_offset_poses_to_parent() {
  return actor_siblings_offset_poses_to_parent_;
}

void BumpBox::updateActorParentInfoBeforeDrag() {
  if (parent() != NULL) {
    actor_parent_id_before_drag_ = parent()->unique_id();
    actor_parent_type_before_drag_ = parent()->actor_type();
    actor_parent_position_before_drag_ = parent()->position();
  } else {
    actor_parent_id_before_drag_ = 0;
  }
}

void BumpBox::updateActorOffsetPoseToItsParentBeforeDrag() {
  if (parent() != NULL) {
    actor_offset_pose_to_its_parent_ = parent()->children_offset_pose(this);
  } else {
    actor_offset_pose_to_its_parent_ = BumpPose(Ogre::Vector3::ZERO, Ogre::Quaternion::IDENTITY);
  }
}

void BumpBox::updateActorSiblingOffsetPoseToParentBeforeDrag() {
  if (parent() != NULL) {
    for_each(VisualPhysicsActor* sibling, parent()->children()) {
      if (sibling != this) {
        actor_siblings_offset_poses_to_parent_[sibling->unique_id()] = parent()->children_offset_pose(sibling);
      }
    }
  } else {
    QHash<VisualPhysicsActorId, BumpPose>();
  }
}

#include "moc/moc_BumpBox.cpp"
