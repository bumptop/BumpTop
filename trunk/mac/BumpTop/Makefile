#  Copyright 2012 Google Inc. All Rights Reserved.
#  
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#  
#      http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

# for more info, read http://doc.trolltech.com/4.5.1/moc.html
all: qt_preprocess protoc_preprocess 

MOC=../Dependencies/moc
qt_preprocess: moc moc/moc_Cube.cpp moc/moc_BumpTopApp.cpp moc/moc_MouseEventTest.mm moc/moc_MouseEventManager.cpp \
		moc/moc_Clickable.cpp moc/moc_ClickableTest.cpp moc/moc_Box.cpp moc/moc_StaticBox.cpp moc/moc_Scene.cpp \
		moc/moc_BumpTopScene.cpp moc/moc_RoomSurfaceTest.cpp moc/moc_BumpBox.cpp moc/moc_QPainterMaterial.cpp \
		moc/moc_Lasso.cpp moc/moc_FileSystemWatcherTest.cpp moc/moc_FileSystemWatcher.cpp moc/moc_FSEventsWatcher.cpp \
		moc/moc_Room.cpp moc/moc_BumpBoxLabel.cpp moc/moc_AnimationManager.cpp moc/moc_UndoRedoTest.cpp \
		moc/moc_BumpFlatSquare.cpp moc/moc_FileItem.cpp moc/moc_FileSystemEventDispatcher.cpp \
		moc/moc_FileSystemEventTestHelper.cpp moc/moc_FileSystemEventDispatcherTest.cpp moc/moc_PhysicsActor.cpp \
		moc/moc_PhysicsBoxActor.cpp moc/moc_VisualActor.cpp moc/moc_VisualPhysicsActor.cpp moc/moc_BumpPile.cpp \
		moc/moc_MaterialLoader.cpp moc/moc_DampedSpringMouseHandler.cpp moc/moc_HighlightActor.cpp \
		moc/moc_VisualPhysicsActorAnimation.cpp moc/moc_Authorization.cpp moc/moc_BumpTopInstanceLock.cpp \
		moc/moc_SearchBoxOverlay.cpp moc/moc_Timer.cpp moc/moc_SearchBox.cpp moc/moc_GriddedPile.cpp \
		moc/moc_KeyboardEventManager.cpp moc/moc_QuickLookPreviewPanel.cpp moc/moc_AlphaElementAnimation.cpp \
		moc/moc_QPainterOverlay.cpp moc/moc_RoomSurface.cpp moc/moc_BumpButton.cpp moc/moc_PerformanceStatsHUD.cpp \
		moc/moc_BumpTopCommands.cpp moc/moc_CustomQLineEdit.cpp moc/moc_BumpToolbar.cpp moc/moc_NSTaskDeletionManager.cpp \
		moc/moc_PileBreakUndoCommand.cpp moc/moc_UsageTracker.cpp moc/moc_CameraAnimation.cpp moc/moc_StickyNote.cpp \
		moc/moc_ToolTipOverlay.cpp moc/moc_FileDropReceiver.cpp moc/moc_OverlayButton.cpp moc/moc_ThemeDownloader.cpp \
		moc/moc_NewItemsPile.cpp

moc:
	mkdir moc

moc/moc_NewItemsPile.cpp: NewItemsPile.h
	$(MOC) $< -o $@

moc/moc_ThemeDownloader.cpp: ThemeDownloader.h
	$(MOC) $< -o $@

moc/moc_OverlayButton.cpp: OverlayButton.h
	$(MOC) $< -o $@

moc/moc_FileDropReceiver.cpp: FileDropReceiver.h
	$(MOC) $< -o $@

moc/moc_ToolTipOverlay.cpp: ToolTipOverlay.h
	$(MOC) $< -o $@

moc/moc_StickyNote.cpp: StickyNote.h
	$(MOC) $< -o $@

moc/moc_CameraAnimation.cpp: CameraAnimation.h
	$(MOC) $< -o $@

moc/moc_UsageTracker.cpp: UsageTracker.h
	$(MOC) $< -o $@

moc/moc_PileBreakUndoCommand.cpp: UndoCommands/PileBreakUndoCommand.h
	$(MOC) $< -o $@

moc/moc_BumpToolbar.cpp: BumpToolbar.h
	$(MOC) $< -o $@

moc/moc_BumpTopCommands.cpp: BumpTopCommands.h
	$(MOC) $< -o $@

moc/moc_PerformanceStatsHUD.cpp: PerformanceStatsHUD.h
	$(MOC) $< -o $@

moc/moc_BumpButton.cpp: BumpButton.h
	$(MOC) $< -o $@

moc/moc_GriddedPile.cpp: GriddedPile.h
	$(MOC) $< -o $@

moc/moc_QuickLookPreviewPanel.cpp: QuickLookPreviewPanel.h
	$(MOC) $< -o $@

moc/moc_KeyboardEventManager.cpp: KeyboardEventManager.h
	$(MOC) $< -o $@

moc/moc_BumpTopInstanceLock.cpp: BumpTopInstanceLock.h
	$(MOC) $< -o $@

moc/moc_Authorization.cpp: Authorization.h
	$(MOC) $< -o $@

moc/moc_VisualPhysicsActorAnimation.cpp: VisualPhysicsActorAnimation.h
	$(MOC) $< -o $@

moc/moc_HighlightActor.cpp: HighlightActor.h
	$(MOC) $< -o $@

moc/moc_MaterialLoader.cpp: MaterialLoader.h
	$(MOC) $< -o $@

moc/moc_DampedSpringMouseHandler.cpp: DampedSpringMouseHandler.h
	$(MOC) $< -o $@

moc/moc_BumpPile.cpp: BumpPile.h
	$(MOC) $< -o $@

moc/moc_VisualPhysicsActor.cpp: VisualPhysicsActor.h
	$(MOC) $< -o $@

moc/moc_VisualActor.cpp: VisualActor.h
	$(MOC) $< -o $@

moc/moc_PhysicsBoxActor.cpp: PhysicsBoxActor.h
	$(MOC) $< -o $@

moc/moc_PhysicsActor.cpp: PhysicsActor.h
	$(MOC) $< -o $@

moc/moc_FileSystemEventDispatcherTest.cpp: FileSystemEventDispatcherTest.cpp
	$(MOC) $< -o $@

moc/moc_FileSystemEventTestHelper.cpp: FileSystemEventTestHelper.h
	$(MOC) $< -o $@

moc/moc_FileSystemEventDispatcher.cpp: FileSystemEventDispatcher.h
	$(MOC) $< -o $@

moc/moc_FileItem.cpp: FileItem.h
	$(MOC) $< -o $@

moc/moc_BumpFlatSquare.cpp: BumpFlatSquare.h
	$(MOC) $< -o $@

moc/moc_UndoRedoTest.cpp: UndoRedoTest.cpp
	$(MOC) $< -o $@

moc/moc_AnimationManager.cpp: AnimationManager.h
	$(MOC) $< -o $@

moc/moc_BumpBoxLabel.cpp: BumpBoxLabel.h
	$(MOC) $< -o $@

moc/moc_Room.cpp: Room.h
	$(MOC) $< -o $@

moc/moc_FSEventsWatcher.cpp: FSEventsWatcher.h
	$(MOC) $< -o $@

moc/moc_FileSystemWatcher.cpp: FileSystemWatcher.h
	$(MOC) $< -o $@

moc/moc_FileSystemWatcherTest.cpp: FileSystemWatcherTest.cpp
	$(MOC) $< -o $@

moc/moc_Lasso.cpp: Lasso.h
	$(MOC) $< -o $@

moc/moc_QPainterMaterial.cpp: QPainterMaterial.h
	$(MOC) $< -o $@

moc/moc_BumpBox.cpp: BumpBox.h
	$(MOC) $< -o $@

moc/moc_RoomSurfaceTest.cpp: RoomSurfaceTest.cpp
	$(MOC) $< -o $@

moc/moc_Cube.cpp: Cube.h
	$(MOC) $< -o $@

moc/moc_BumpTopApp.cpp: BumpTopApp.h
	 $(MOC) $< -o $@

moc/moc_MouseEventTest.mm: OSX/MouseEventTest.mm
	 $(MOC) $< -o $@

moc/moc_MouseEventManager.cpp: MouseEventManager.h
	 $(MOC) $< -o $@

moc/moc_Clickable.cpp: Clickable.h
	 $(MOC) $< -o $@

moc/moc_ClickableTest.cpp: ClickableTest.cpp
	 $(MOC) $< -o $@

moc/moc_Box.cpp: Box.h
	$(MOC) $< -o $@

moc/moc_StaticBox.cpp: StaticBox.h
	$(MOC) $< -o $@

moc/moc_Scene.cpp: Scene.h
	$(MOC) $< -o $@

moc/moc_BumpTopScene.cpp: BumpTopScene.h
	$(MOC) $< -o $@

moc/moc_SearchBoxOverlay.cpp: SearchBoxOverlay.h
	$(MOC) $< -o $@

moc/moc_Timer.cpp: Timer.h
	$(MOC) $< -o $@

moc/moc_SearchBox.cpp: SearchBox.h
	$(MOC) $< -o $@

moc/moc_AlphaElementAnimation.cpp: AlphaElementAnimation.h
	$(MOC) $< -o $@

moc/moc_QPainterOverlay.cpp: QPainterOverlay.h
	$(MOC) $< -o $@

moc/moc_RoomSurface.cpp: RoomSurface.h
	$(MOC) $< -o $@

moc/moc_CustomQLineEdit.cpp: CustomQLineEdit.h
	$(MOC) $< -o $@

moc/moc_NSTaskDeletionManager.cpp: NSTaskDeletionManager.h
	$(MOC) $< -o $@

PROTOC=../Dependencies/protobuf-2.4.1/build/bin/protoc
PROTOC_DIR=protoc

protoc_preprocess: $(PROTOC_DIR)/AllMessages.pb.cc

protoc/AllMessages.pb.cc: $(PROTOC_DIR)/AllMessages.proto
	-mv ../Dependencies/protobuf-2.4.1/build/lib/dylib/*.dylib ../Dependencies/protobuf-2.4.1/build/lib
	$(PROTOC) -I=$(PROTOC_DIR) --cpp_out=$(PROTOC_DIR) $<
	mv ../Dependencies/protobuf-2.4.1/build/lib/*.dylib ../Dependencies/protobuf-2.4.1/build/lib/dylib
