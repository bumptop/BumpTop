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

#include "BumpTop/StickyNote.h"

#include <string>

#include "BumpTop/AppSettings.h"
#include "BumpTop/Authorization.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/FileItem.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/HighlightActor.h"
#include "BumpTop/Math.h"
#include "BumpTop/MouseEventManager.h"
#include "BumpTop/ProtocolBufferHelpers.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/Shape.h"
#include "BumpTop/StickyNotePad.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"

const int kDocSize = 350;

class StickyNoteText;

BumpTopCommandSet* StickyNote::sticky_note_context_menu_items_set = MakeQSet(7,  // count, must keep this updated
                                                              Open::singleton(),
                                                              CreatePile::singleton(),
                                                              MoveToTrash::singleton(),
                                                              Grow::singleton(),
                                                              Shrink::singleton(),
                                                              PileByTypeForSelectedActors::singleton(),
                                                              GridView::singleton());

StickyNote::StickyNote(Ogre::SceneManager *scene_manager, Physics* physics, Room *room, VisualPhysicsActorId unique_id)
: BumpFlatSquare(scene_manager, physics, room, unique_id),
  text_(NULL),
  visual_copy_of_actor_(NULL),
  is_dummy_(false) {
}

StickyNote::~StickyNote() {
  if (text_ != NULL) {
    delete text_;
  }
  if (!is_dummy_)
    StickyNoteCounter::singleton()->decrementStickyNoteCount();
}

BumpTopCommandSet* StickyNote::supported_context_menu_items() {
  return sticky_note_context_menu_items_set;
}

void StickyNote::init() {
  BumpBox::init();

  StickyNoteCounter::singleton()->incrementStickyNoteCount();
  set_material_name(AppSettings::singleton()->global_material_name(STICKY_NOTE_MATERIAL));
  text_ = new StickyNoteText(scene_manager_, ogre_scene_node());
  text_->init();
  text_->set_to_render_after(visual_actor());

  room_->addActor(this);
}

bool StickyNote::initFromBuffer(VisualPhysicsActorBuffer* buffer, bool physics_enabled) {
  StickyNote::init();

  set_position(Vector3BufferToVector3(buffer->position()));
  set_orientation(QuaternionBufferToQuaternion(buffer->orientation()));
  set_size(Vector3BufferToVector3(buffer->size()));

  if (buffer->has_room_surface()) {
    RoomSurface* surface = room_->getSurface((RoomSurfaceType)buffer->room_surface());
    set_room_surface(surface);
    if (surface->is_pinnable_receiver())
      pinToSurface(surface);
  }

  set_path(QStringFromUtf8(buffer->file_name()));
  loadTextFromPath();

  return true;
}

void StickyNote::initWithPath(QString path, bool physics_enabled) {
  StickyNote::init();
  printLine(StickyNoteCounter::singleton()->count());
  Ogre::Real size = ProAuthorization::singleton()->sticky_note_sizes(std::min(2, StickyNoteCounter::singleton()->count() - 1));  // NOLINT
  set_size(Ogre::Vector3(size, size, size));
  set_path(path);
  loadTextFromPath();
}

void StickyNote::set_path(QString path) {
  if (file_item_ != NULL) {
    delete file_item_;
  }

  file_item_ = new FileItem(path);
  assert(QObject::connect(file_item_, SIGNAL(onFileRenamed(const QString&, const QString&)),
                        this, SLOT(updateLabel())));
  assert(QObject::connect(file_item_, SIGNAL(onFileRemoved(const QString&)),
                        this, SLOT(fileRemoved(const QString&))));
}

void StickyNote::initAsVisualCopyOfActor(VisualPhysicsActor* actor) {
  BumpFlatSquare::initAsVisualCopyOfActor(actor);

  is_dummy_ = true;

  text_ = new StickyNoteText(scene_manager_, ogre_scene_node());
  text_->init();
  text_->set_text(static_cast<StickyNote*>(actor)->text_->text());
  text_->set_to_render_after(visual_actor());
}

void StickyNote::set_alpha(Ogre::Real alpha) {
  BumpFlatSquare::set_alpha(alpha);

  const std::string material_name = utf8(text_->material_name());
  Ogre::MaterialPtr material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(material_name));
  if (!material.isNull()) {
    material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(Ogre::LBX_MODULATE,
                                                                                     Ogre::LBS_TEXTURE,
                                                                                     Ogre::LBS_MANUAL,
                                                                                     0.0,
                                                                                     alpha);
  }
}

bool StickyNote::nameable() {
  return false;
}

StickyNote* StickyNote::constructCopyOfMyType() {
  return new StickyNote(scene_manager_, physics_, room_, 0);
}

std::string StickyNote::meshName() {
  return Shape::singleton()->flat_square();
}

Ogre::Vector3 StickyNote::absoluteMeshSizeDividedBy100() {
  return Ogre::Vector3(1.0, 0, 1.0);
}

btVector3 StickyNote::physicsSize() {
  return btVector3(1.0, 0.125, 1.0);
}

bool StickyNote::pinnable() {
  return true;
}

VisualPhysicsActorType StickyNote::actor_type() {
  return STICKY_NOTE;
}

void StickyNote::set_render_queue_group(uint8 queue_id) {
  BumpBox::set_render_queue_group(queue_id);
  if (text_ != NULL) {
    text_->set_render_queue_group(queue_id);
  }
}

void StickyNote::launch() {
  visual_actor()->set_visible(false);
  Ogre::Camera *camera = BumpTopApp::singleton()->camera();
  // Get the orientation based on the camera's orientation
  Ogre::Quaternion original_camera_orientation = camera->getOrientation();
  camera->pitch(Ogre::Radian(Ogre::Degree(90)));
  Ogre::Quaternion desired_orientation = camera->getOrientation();
  camera->setOrientation(original_camera_orientation);

  // Move closer to the camera
  const Ogre::Real kDesiredDistanceToCamera = 750;
  Ogre::Vector3 vector_between_camera_and_actor = camera->getPosition() - position();
  vector_between_camera_and_actor.normalise();
  Ogre::Plane camera_plane(camera->getDirection(), camera->getPosition());
  Ogre::Vector3 position_to_animate_to = camera->getPosition() - vector_between_camera_and_actor * kDesiredDistanceToCamera;  // NOLINT

  // Make sure that you're a uniform distance from the camera plane
  Ogre::Real distance_to_camera_place = camera_plane.getDistance(position_to_animate_to);
  Ogre::Vector3 closest_point_on_plane = position_to_animate_to - camera->getDirection() * distance_to_camera_place;
  position_to_animate_to = closest_point_on_plane + camera->getDirection() * kDesiredDistanceToCamera;

  // Adjust the position of the sticky note to fit within the screen (don't want it cut off at the edges)
  pose_before_editing_ = pose();  // Save our pose before we start changing it
  Ogre::Quaternion orientation_before_animation = orientation();
  Ogre::Vector3 position_before_animation = position();

  set_position(position_to_animate_to);
  set_orientation(desired_orientation);

  const Ogre::Real kScreenMargin = 40;
  Ogre::AxisAlignedBox screen_box(40, 40, -1, BumpTopApp::singleton()->screen_resolution().x - kScreenMargin,
                                            BumpTopApp::singleton()->screen_resolution().y - kScreenMargin,
                                            1);
  Ogre::AxisAlignedBox sticky_bounding_box = screenBoundingBox();

  Ogre::Real tolerance = 0.5;
  Ogre::Vector3 position_adjustment(0, 0, 0);
  if (!(sticky_bounding_box.getMaximum() < screen_box.getMaximum())) {
    Ogre::Vector3 adjustment = screen_box.getMaximum() + Ogre::Vector3(tolerance) - sticky_bounding_box.getMaximum();
    position_adjustment += Math::componentwise_min(adjustment, Ogre::Vector3::ZERO);
  }
  if (!(sticky_bounding_box.getMinimum() > screen_box.getMinimum())) {
    Ogre::Vector3 adjustment = screen_box.getMinimum() + Ogre::Vector3(tolerance) - sticky_bounding_box.getMinimum();
    position_adjustment += Math::componentwise_max(adjustment, Ogre::Vector3::ZERO);
  }

  Ogre::Real x_scale = (sticky_bounding_box.getMaximum().x - sticky_bounding_box.getMinimum().x) / 200.0;
  Ogre::Real y_scale = (sticky_bounding_box.getMaximum().y - sticky_bounding_box.getMinimum().y) / 200.0;
  position_to_animate_to += BumpTopApp::singleton()->camera()->getRight()*position_adjustment.x/x_scale;
  position_to_animate_to -= BumpTopApp::singleton()->camera()->getUp()*position_adjustment.y/y_scale;

  set_orientation(pose_before_editing_.orientation);
  set_position(pose_before_editing_.position);

  // Create a visual copy of myself, and animate it
  visual_copy_of_actor_ = createVisualCopyOfSelf();
  VisualPhysicsActorAnimation* actor_animation = new VisualPhysicsActorAnimation(visual_copy_of_actor_, 200,
                                                                                 position_to_animate_to,
                                                                                 desired_orientation, NULL, 1.0,
                                                                                 200.0 / scale().x);
  //  assert(QObject::connect(actor_animation, SIGNAL(onAnimationComplete(VisualPhysicsActorAnimation*)),  // NOLINT
  //                          room_, SLOT(deleteActorCopyWhenFadeFinishes(VisualPhysicsActorAnimation*)))); // NOLINT
  assert(QObject::connect(actor_animation, SIGNAL(onAnimationComplete(VisualPhysicsActorAnimation*)),  // NOLINT
                          this, SLOT(beginEditingAnimationFinished(VisualPhysicsActorAnimation*)))); // NOLINT
  visual_copy_of_actor_->set_render_queue_group(99);
  actor_animation->start();

  // TODO: not having this line gives a really weird bug with the sticky note text fading
  set_selected(false);
}


Ogre::AxisAlignedBox StickyNote::screenBoundingBox() {
  QList<Ogre::Vector3> corners;
  corners.push_back(Ogre::Vector3(-50, 0, -50));
  corners.push_back(Ogre::Vector3(50, 0, 50));
  corners.push_back(Ogre::Vector3(-50, 0, 50));
  corners.push_back(Ogre::Vector3(50, 0, -50));

  Ogre::AxisAlignedBox sticky_bounding_box;

  bool first_pass = true;
  Ogre::Matrix4 transform = this->transform();
  for_each(Ogre::Vector3 corner, corners) {
    Ogre::Vector2 transformed_corner = worldPositionToScreenPosition(transform * corner);
    corner = Ogre::Vector3(transformed_corner.x, transformed_corner.y, 0);
    if (first_pass) {
      sticky_bounding_box = Ogre::AxisAlignedBox(corner, corner);
      first_pass = false;
    } else {
      sticky_bounding_box.merge(corner);
    }
  }
  return sticky_bounding_box;
}

void StickyNote::beginEditingAnimationFinished(VisualPhysicsActorAnimation* animation) {
  QTextEdit* editable_text = text_->text_edit();

  Ogre::AxisAlignedBox sticky_bounding_box = visual_copy_of_actor_->screenBoundingBox();
  editable_text->move(sticky_bounding_box.getMinimum().x + 5, sticky_bounding_box.getMinimum().y + 26);

  // Do not remove this code -- this shows how the size is calculated
  /*editable_text->resize(sticky_bounding_box.getMaximum().x - sticky_bounding_box.getMinimum().x - 9,
                         sticky_bounding_box.getMaximum().y - sticky_bounding_box.getMinimum().y - 8);*/

  // xxx ??
  editable_text->show();
  editable_text->grabKeyboard();
  assert(QObject::connect(editable_text, SIGNAL(textChanged()),
                          this, SLOT(editableTextChanged())));

  NSView* text_edit_view = reinterpret_cast<NSView *>(editable_text->winId());
  NSWindow* text_edit_window = [text_edit_view window];
  BumpTopApp::singleton()->set_should_force_bumptop_window_to_front(false);
  [text_edit_window setLevel:kCGDesktopIconWindowLevel];
  [text_edit_window orderFrontRegardless];

  assert(QObject::connect(BumpTopApp::singleton()->mouse_event_manager(), SIGNAL(onMouseDown(MouseEvent*)),  // NOLINT
                          this, SLOT(closeEditableStickyNote(MouseEvent*))));  // NOLINT
}

void StickyNote::closeEditableStickyNote(MouseEvent* mouse_event) {
  BumpTopApp::singleton()->set_should_force_bumptop_window_to_front(true);

  assert(QObject::disconnect(BumpTopApp::singleton()->mouse_event_manager(), SIGNAL(onMouseDown(MouseEvent*)),  // NOLINT
                          this, SLOT(closeEditableStickyNote(MouseEvent*))));  // NOLINT
  text_->update();
  assert(visual_copy_of_actor_ != NULL);

  VisualPhysicsActorAnimation* actor_animation = new VisualPhysicsActorAnimation(visual_copy_of_actor_, 200,
                                                    pose_before_editing_.position,
                                                    pose_before_editing_.orientation,
                                                    NULL, 1,
                                                    scale().x / 200.0);
  assert(QObject::connect(actor_animation, SIGNAL(onAnimationComplete(VisualPhysicsActorAnimation*)),  // NOLINT
                          this, SLOT(finishEditingAnimationFinished(VisualPhysicsActorAnimation*)))); // NOLINT
  actor_animation->start();

  text_->set_to_render_after(visual_actor());


  QTextEdit* editable_text = text_->text_edit();
  editable_text->hide();
  save();
}

void StickyNote::finishEditingAnimationFinished(VisualPhysicsActorAnimation* animation) {
  delete visual_copy_of_actor_;
  visual_copy_of_actor_ = NULL;
  visual_actor()->set_visible(true);

  // For some reason "visual_actor()->set_visible(true);"
  // also sets the highlight to be visible, so we need to hide
  // if the sticky not is not selected.
  if (!selected())
    highlight_->set_visible(false);
}

void StickyNote::editableTextChanged() {
  text_->setFontSizeBasedOnAmountOfText();
}

QString StickyNote::text() {
  return text_->text();
}


void StickyNote::save() {
  QString sticky_note_path = FileManager::getApplicationDataPath() + "Stickies/";

  // First we make sure the sticky note directory exists
  QDir sticky_note_dir = QDir(sticky_note_path);
  if (!sticky_note_dir.exists())
    sticky_note_dir.mkpath(sticky_note_dir.absolutePath());

  // next, we check to see if this sticky note has a path, if not, then
  // we pick a file name and create a path (to store the text)
  if (path() == "") {
    QStringList stickies = sticky_note_dir.entryList();
    QString new_name = "Sticky Note 1.txt";
    QString str_i;
    int i = 2;
    while (stickies.contains(new_name)) {
      str_i.setNum(i);
      new_name = "Sticky Note " + str_i + ".txt";
      i++;
    }
    set_path(sticky_note_path + new_name);
  }

  QFile sticky_note_file(sticky_note_dir.filePath(path()));
  if (!sticky_note_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return;
  }

  QTextStream sticky_file_out(&sticky_note_file);
  sticky_file_out << text();
  sticky_note_file.close();
}

void StickyNote::loadTextFromPath() {
  QFile sticky_note_file(path());
  if (!sticky_note_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return;
  }

  QString text_from_file;
  QTextStream sticky_file_out(&sticky_note_file);
  text_from_file = sticky_file_out.readAll();
  sticky_note_file.close();

  text_->set_text(text_from_file);
  text_->update();
}

StickyNoteText::StickyNoteText(Ogre::SceneManager *scene_manager, Ogre::SceneNode *parent_ogre_scene_node)
: VisualActor(scene_manager, parent_ogre_scene_node, Shape::singleton()->flat_square(), false) {
}

StickyNoteText::~StickyNoteText() {
  delete sticky_note_text_edit_;
  delete material_;
}

void StickyNoteText::init() {
  VisualActor::init();

  // Create
  sticky_note_text_edit_ = new QTextEdit();
  sticky_note_text_edit_->setWindowFlags(Qt::FramelessWindowHint);
  sticky_note_text_edit_->setContextMenuPolicy(Qt::PreventContextMenu);
  sticky_note_text_edit_->resize(kDocSize, kDocSize);

  QPalette p;
  p.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(9),
             QColor(240, 233, 140));
  sticky_note_text_edit_->setPalette(p);

  QTextOption option;
  option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
  QTextDocument* sticky_note_text_doc = sticky_note_text_edit_->document();
  sticky_note_text_doc->setDefaultTextOption(option);
  sticky_note_text_doc->setTextWidth(kDocSize);

  QString device_name = QStringFromUtf8(Ogre::Root::getSingleton().getRenderSystem()->getCapabilities()->getDeviceName());  // NOLINT
  int num_mip_maps = 7;
  if (device_name == "Intel GMA 950 OpenGL Engine" || device_name == "Intel GMA X3100 OpenGL Engine") {
    num_mip_maps = 0;
  }

  // create a new QPainter material
  material_ = new QPainterMaterial();
  material_->initWithSize(kDocSize, kDocSize, num_mip_maps, true);
  assert(QObject::connect(material_, SIGNAL(draw(QPainter*)),  // NOLINT
                          this, SLOT(drawText(QPainter*))));  // NOLINT
  update();

  set_material_name(material_->name());

  // This ugly little piece of code accounts for the fact that the
  // material size is rounded up to the nearest power of 2 (512x512),
  // whereas our document size is 350x350. Thus we scale the material up and
  // translate it
  set_scale(Ogre::Vector3((450.0/350)*100, 100, (450.0/350)*100));
  set_position(Ogre::Vector3(20, 0, 20));
}

void StickyNoteText::update() {
  material_->update();
}

void StickyNoteText::setFontSizeBasedOnAmountOfText() {
  QTextDocument* sticky_note_text_doc = sticky_note_text_edit_->document();

  QFont sticky_note_font;
  sticky_note_font.setPointSize(36);
  sticky_note_text_doc->setDefaultFont(sticky_note_font);

  QSizeF text_size = sticky_note_text_doc->size();
  const int kMinStickyNoteFontSize = 20;
  while (text_size.height() > kDocSize && sticky_note_font.pointSize() > kMinStickyNoteFontSize) {
    sticky_note_font.setPointSize(sticky_note_font.pointSize() - 1);
    sticky_note_text_doc->setDefaultFont(sticky_note_font);
    text_size = sticky_note_text_doc->size();
  }
}

void StickyNoteText::drawText(QPainter* painter) {
  QTextDocument* sticky_note_text_doc = sticky_note_text_edit_->document();

  setFontSizeBasedOnAmountOfText();

  // Our actual material is bigger than kDocSize x kDocSize, so we need to
  // clip the painter to the the bounds... the -7 is so that we don't cut it
  // off in the middle of a line.
  painter->setClipRegion(QRegion(0, 0, kDocSize, kDocSize - 7));
  painter->setPen(QPen(Qt::black));
  sticky_note_text_doc->drawContents(painter);
}

void StickyNoteText::set_text(QString text) {
  sticky_note_text_edit_->setPlainText(text);
}

QString StickyNoteText::text() {
  return sticky_note_text_edit_->toPlainText();
}

QTextEdit* StickyNoteText::text_edit() {
  return sticky_note_text_edit_;
}

#include "moc/moc_StickyNote.cpp"
