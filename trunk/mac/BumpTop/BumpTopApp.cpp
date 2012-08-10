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

#include "BumpTop/BumpTopApp.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <string>

#include "BumpTop/AppSettings.h"
#include "BumpTop/Authorization.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/KeyboardEventManager.h"
#include "BumpTop/MaterialLoader.h"
#include "BumpTop/MouseEventManager.h"
#include "BumpTop/Physics.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/Room.h"

BumpTopApp* BumpTopApp::singleton_ = NULL;

BumpTopApp::BumpTopApp()
: camera_(NULL),
  camera_node_(NULL),
  scene_manager_(NULL),
  viewport_(NULL),
  scene_(NULL),
  render_window_(NULL),
  mouse_event_manager_(NULL),
  keyboard_event_manager_(NULL),
  global_state_changed_this_frame_(true),
  global_state_changed_last_frame_(true),
  should_force_bumptop_window_to_front_(true),
  context_menu_open_(false) {
  if (BumpTopApp::singleton_ == NULL)
    BumpTopApp::singleton_ = this;
}

// we don't actually create the BumpTopApp here because it's an abstract class
// we just assume that a singleton instance has been created (in Mac's case, in OSXCocoaBumpTopApplication)
BumpTopApp* BumpTopApp::singleton() {
  return BumpTopApp::singleton_;
}

BumpTopApp::~BumpTopApp() {
}

void BumpTopApp::init() {
  createRootNode();
  setRenderSystem();
  initOgreCore();
  createSceneManager();
  initResources();
  initGlobalBumpTopMaterials();
  createCamera();
  createViewports();
  createMouseEventManager();
  createKeyboardEventManager();
  initPhysics();
  initRenderStopwatch();
  //initUsageTracker();
}

void BumpTopApp::initRenderStopwatch() {
  render_stopwatch_.restart();
}

void BumpTopApp::initUsageTracker() {
  usage_tracker_.init();
}

void BumpTopApp::makeSelfForegroundApp() {
  ProcessSerialNumber bumptop_process_serial_number;
  GetCurrentProcess(&bumptop_process_serial_number);
  SetFrontProcess(&bumptop_process_serial_number);
}

Ogre::RenderWindow* BumpTopApp::render_window() {
  return render_window_;
}

Ogre::SceneManager* BumpTopApp::ogre_scene_manager() {
  return scene_manager_;
}

Ogre::Camera* BumpTopApp::camera() {
  return camera_;
}

Ogre::SceneNode* BumpTopApp::camera_node() {
  return camera_node_;
}

Ogre::Viewport* BumpTopApp::viewport() {
  return viewport_;
}

Physics* BumpTopApp::physics() {
  return physics_;
}

MouseEventManager* BumpTopApp::mouse_event_manager() {
  return mouse_event_manager_;
}

KeyboardEventManager* BumpTopApp::keyboard_event_manager() {
  return keyboard_event_manager_;
}

void BumpTopApp::windowRectChanged() {
  emit onWindowRectChanged();
}

void BumpTopApp::renderTick() {
  // We cap off the maximum elapsed time to prevent a feedback loop of slowness
  uint64_t elapsed = std::min((uint64_t)20, render_stopwatch_.elapsed());
  render_stopwatch_.restart();

  // need this so background loaded textures will fire their events and force visuals to refresh
  Ogre::Root::getSingleton().getWorkQueue()->processResponses();

  emit onRender();
  if (!isInIdleMode()) {
    pushGLContextAndSwitchToOgreGLContext();
    Ogre::Root::getSingleton().renderOneFrame();
    popGLContext();
#define NUM_PHYSICS_ITERS_PER_STEP 3.0f
    float time_step = (1.1*elapsed)/1000.0;  // The factor of 1.1 is here to speed up physics a bit
    physics_->stepSimulation(time_step, NUM_PHYSICS_ITERS_PER_STEP, time_step/NUM_PHYSICS_ITERS_PER_STEP);
  }

  global_state_changed_last_frame_ = global_state_changed_this_frame_;
  global_state_changed_this_frame_ = false;
}

bool BumpTopApp::isInIdleMode() {
  return !(global_state_changed_this_frame_ || global_state_changed_last_frame_);
}

void BumpTopApp::markGlobalStateAsChanged() {
  global_state_changed_this_frame_ = true;
}

DragAndDrop* BumpTopApp::drag_and_drop() {
  return drag_and_drop_;
}

bool BumpTopApp::should_force_bumptop_window_to_front() {
  return should_force_bumptop_window_to_front_;
}

void BumpTopApp::set_should_force_bumptop_window_to_front(bool force_to_front) {
  should_force_bumptop_window_to_front_ = force_to_front;
}

void BumpTopApp::set_context_menu_open(bool open) {
  context_menu_open_ = open;
}

bool BumpTopApp::context_menu_open() {
  return context_menu_open_;
}

void BumpTopApp::createRootNode() {
  QString resource_path = FileManager::getResourcePath();

  // Create a new root object with the correct paths
  new Ogre::Root(utf8(resource_path + "/plugins.cfg"),
                 utf8(resource_path + "/ogre.cfg"),
                 utf8(resource_path + "/Ogre.log"));
}

void BumpTopApp::setRenderSystem() {
  Ogre::Root& OgreRoot = Ogre::Root::getSingleton();
  OgreRoot.setRenderSystem(OgreRoot.getAvailableRenderers().front());
}

void BumpTopApp::createSceneManager() {
  // Create the SceneManager, in this case a generic one
  scene_manager_ = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_GENERIC);
}

void BumpTopApp::initResources() {
  QString resource_path = FileManager::getResourcePath();

  Ogre::ResourceGroupManager& OgreResourceGroupManager = Ogre::ResourceGroupManager::getSingleton();

  // Add resource locations -- looking at folders recursively
  OgreResourceGroupManager.addResourceLocation(utf8(resource_path),
                                               std::string("FileSystem"),
                                               Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,  // NOLINT
                                               false);
  OgreResourceGroupManager.addResourceLocation("/",
                                               std::string("FileSystem"),
                                               Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,  // NOLINT
                                               false);

  OgreResourceGroupManager.initialiseAllResourceGroups();
}

void loadGlobalMaterial(GlobalMaterials global_material_name, QString resource_name) {
  MaterialLoader* material_loader = new MaterialLoader();
  material_loader->initAsImageWithFilePath(FileManager::pathForResource(resource_name), false);
  AppSettings::singleton()->set_global_material_name(global_material_name, material_loader->name());
  BumpMaterialManager::singleton()->incrementReferenceCount(material_loader->name());
  BumpTextureManager::singleton()->incrementReferenceCount(material_loader->name());
  delete material_loader;
}

void BumpTopApp::initGlobalBumpTopMaterials() {
  loadGlobalMaterial(DEFAULT_ICON, "default_icon.png");
  loadGlobalMaterial(GRIDDED_PILE_BACKGROUND, "square.png");
  loadGlobalMaterial(TOOLBAR_TOP, "toolbar_top.png");
  loadGlobalMaterial(TOOLBAR_MIDDLE, "toolbar_middle.png");
  loadGlobalMaterial(TOOLBAR_BOTTOM, "toolbar_bottom.png");
  loadGlobalMaterial(HIGHLIGHT, "square_texture.png");
  loadGlobalMaterial(TRANSPARENT_PIXEL, "1x1.png");
  loadGlobalMaterial(STICKY_NOTE_MATERIAL, "StickyNote.png");
  loadGlobalMaterial(STICKY_NOTE_PAD_MATERIAL, "StickyNotePad.png");
  loadGlobalMaterial(NO_STICKIES_LEFT, "no_stickies_left.png");
  loadGlobalMaterial(NEW_ITEMS_PILE_ICON, "new_items_pile_icon.png");
}

void BumpTopApp::createCamera() {
  // Create the camera
  camera_ = scene_manager_->createCamera("MainCamera");

  // This allows us to view the scene from the direction (0, -1, 0)
  // See: https://www.ogre3d.org/forums/viewtopic.php?f=1&t=46580
  camera_->setFixedYawAxis(false);

  // Define the frame of view to be 43 degrees
  Ogre::Degree fov_y = Ogre::Degree(43);
  camera_->setFOVy(fov_y);
  camera_node_ = scene_manager_->getRootSceneNode()->createChildSceneNode();
  camera_node_->attachObject(camera_);
}

void BumpTopApp::createViewports() {
  // Create one viewport, entire window
  viewport_ = render_window_->addViewport(camera_);
}

void BumpTopApp::createMouseEventManager() {
  mouse_event_manager_ = new MouseEventManager(this);
}

void BumpTopApp::createKeyboardEventManager() {
  keyboard_event_manager_ = new KeyboardEventManager(this);
}

void BumpTopApp::initPhysics() {
  physics_ = new Physics();
  physics_->init();
}

void BumpTopApp::mouseDown(float x, float y, int num_clicks, int modifier_flags) {
  emit onMouseDown(x, y, num_clicks, modifier_flags);
  mouse_event_manager_->mouseDown(x, y, num_clicks, modifier_flags);
}

void BumpTopApp::mouseDragged(float x, float y, int num_clicks, int modifier_flags) {
  emit onMouseDragged(x, y, num_clicks, modifier_flags);
  mouse_event_manager_->mouseDragged(x, y, num_clicks, modifier_flags);
}

void BumpTopApp::mouseUp(float x, float y, int num_clicks, int modifier_flags) {
  emit onMouseUp(x, y, num_clicks, modifier_flags);
  mouse_event_manager_->mouseUp(x, y, num_clicks, modifier_flags);
}

void BumpTopApp::mouseMoved(float x, float y, int num_clicks, int modifier_flags) {
  emit onMouseMoved(x, y, num_clicks, modifier_flags);
  mouse_event_manager_->mouseMoved(x, y, num_clicks, modifier_flags);
}

void BumpTopApp::rightMouseDown(float x, float y, int num_clicks, int modifier_flags) {
  emit onRightMouseDown(x, y, num_clicks, modifier_flags);
  mouse_event_manager_->rightMouseDown(x, y, num_clicks, modifier_flags);
}

void BumpTopApp::rightMouseDragged(float x, float y, int num_clicks, int modifier_flags) {
  emit onRightMouseDragged(x, y, num_clicks, modifier_flags);
  mouse_event_manager_->rightMouseDragged(x, y, num_clicks, modifier_flags);
}

void BumpTopApp::scrollWheel(float x, float y, int num_clicks, int modifier_flags, float delta_y) {
  mouse_event_manager_->scrollWheel(x, y, num_clicks, modifier_flags, delta_y);
}

void BumpTopApp::rightMouseUp(float x, float y, int num_clicks, int modifier_flags) {
  emit onRightMouseUp(x, y, num_clicks, modifier_flags);
  mouse_event_manager_->rightMouseUp(x, y, num_clicks, modifier_flags);
}

void BumpTopApp::applicationWillTerminate() {
  emit onApplicationWillTerminate();
}

void BumpTopApp::set_scene(BumpTopScene* scene) {
  scene_ = scene;
}

BumpTopScene* BumpTopApp::scene() {
  return scene_;
}

#include "BumpTop/moc/moc_BumpTopApp.cpp"
