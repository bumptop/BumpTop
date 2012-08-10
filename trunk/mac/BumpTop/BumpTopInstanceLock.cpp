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

#include "BumpTop/BumpTopInstanceLock.h"

#include <utility>

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/Processes.h"
#include "BumpTop/QPainterMaterial.h"
#include "BumpTop/QStringHelpers.h"

float kCircleSize = 50.0;

#define LOCK_FILENAME "lock.txt"
#define SANDBOX_LOCK_FILENAME "sandbox_lock.txt"

#define LOCK_FILE_PATH FileManager::getApplicationDataPath() + LOCK_FILENAME
#define SANDBOX_LOCK_FILE_PATH FileManager::getApplicationDataPath() + SANDBOX_LOCK_FILENAME

bool BumpTopInstanceLock::is_running_in_sandbox_ = false;

BumpTopInstanceLock::BumpTopInstanceLock(int argc, char *argv[])
: locked_file_(NULL) {
  is_running_in_sandbox_ = argc == 2 && QStringFromUtf8(argv[1]) == "-sandbox";
}

BumpTopInstanceLock::~BumpTopInstanceLock() {
  if (locked_file_ != NULL)
    delete locked_file_;
}

void BumpTopInstanceLock::init() {
  if (!is_running_in_sandbox_) {
    FileSystemEventDispatcher::singleton()->addPathToWatch(FileManager::getApplicationDataPath(), this, true);
    blockUntilOtherInstanceOfBumpTopDies();
  }

  clearSandboxLock();

  if (is_running_in_sandbox_ && isOtherInstanceOfBumpTopRunning().first)
    sleep(1);  // if we don't sleep here, we get cases where the other version doesn't notice added file
}

void BumpTopInstanceLock::clearSandboxLock() {
  if (QFileInfo(SANDBOX_LOCK_FILE_PATH).exists())
    QDir(FileManager::getApplicationDataPath()).remove(SANDBOX_LOCK_FILENAME);
}

bool BumpTopInstanceLock::tryLock() {
  locked_file_ = new QtLockedFile((is_running_in_sandbox_) ? SANDBOX_LOCK_FILE_PATH : LOCK_FILE_PATH);
  return locked_file_->open(QIODevice::ReadWrite) && locked_file_->lock(QtLockedFile::WriteLock, false);
}

void BumpTopInstanceLock::blockUntilOtherInstanceOfBumpTopDies() {
  std::pair<bool, ProcessInfo> other_bumptop_instance = isOtherInstanceOfBumpTopRunning();
  if (other_bumptop_instance.first)
    while (isProcessRunning(other_bumptop_instance.second.psn))
      sleep(1);
}

std::pair<bool, ProcessInfo> BumpTopInstanceLock::isOtherInstanceOfBumpTopRunning() {
  for_each(ProcessInfo process_info, listAllProcesses()) {
    if (process_info.name == "BumpTop")
      return std::pair<bool, ProcessInfo>(true, process_info);
  }
  return std::pair<bool, ProcessInfo>(false, ProcessInfo());
}

bool BumpTopInstanceLock::is_running_in_sandbox() {
  return is_running_in_sandbox_;
}

void BumpTopInstanceLock::fileAdded(const QString& path) {
  if (path == SANDBOX_LOCK_FILE_PATH) {
    blockUntilOtherInstanceOfBumpTopDies();
    clearSandboxLock();
  }
}

void BumpTopInstanceLock::fileRemoved(const QString& path) {
}

void BumpTopInstanceLock::fileRenamed(const QString& old_path, const QString& new_path) {
}

void BumpTopInstanceLock::fileModified(const QString& path) {
}

RedDot::RedDot() {
}

RedDot::~RedDot() {
  red_dot_material_->disconnect(this);

  Ogre::OverlayManager::getSingleton().destroyOverlayElement("RedDotOverlayPanel");
  Ogre::OverlayManager::getSingleton().destroy("RedDot");

  delete red_dot_material_;
  red_dot_material_ = NULL;

  BumpTopApp::singleton()->markGlobalStateAsChanged();
}

void RedDot::init() {
  red_dot_material_ = new QPainterMaterial();
  red_dot_material_->initWithSize(kCircleSize+4, kCircleSize+4);

  assert(QObject::connect(red_dot_material_, SIGNAL(draw(QPainter*)),  // NOLINT
                          this, SLOT(drawRedDot(QPainter*))));         // NOLINT
  red_dot_material_->update();

  Ogre::OverlayElement *panel_as_element = Ogre::OverlayManager::getSingleton().createOverlayElement("Panel",
                                                                                                     "RedDotOverlayPanel");  // NOLINT
  Ogre::OverlayContainer* panel = static_cast<Ogre::OverlayContainer*>(panel_as_element);

  panel->setMetricsMode(Ogre::GMM_PIXELS);

  panel->setDimensions(red_dot_material_->width(), red_dot_material_->height());
  panel->setPosition(25, 50);
  panel->setMaterialName(utf8(red_dot_material_->name()));

  Ogre::Overlay* overlay = Ogre::OverlayManager::getSingleton().create("RedDotOverlay");
  overlay->add2D(panel);
  // Show the overlay
  overlay->show();
}

void RedDot::drawRedDot(QPainter* painter) {
  QBrush fill_brush(QColor(0xFF, 0x00, 0x00));
  painter->setBrush(fill_brush);

  QPen pen;  // creates a default pen
  pen.setStyle(Qt::NoPen);
  painter->setPen(pen);

  painter->setRenderHint(QPainter::Antialiasing, true);
  QRectF rectangle(2.0, 2.0, kCircleSize, kCircleSize);

  painter->drawEllipse(rectangle);
}

#include "moc/moc_BumpTopInstanceLock.cpp"
