// Copyright 2012 Google Inc. All Rights Reserved.
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

#ifndef BT_ATESTGENERAL
#define BT_ATESTGENERAL

#include "BT_GLTextureManager.h"
#include "BT_FileSystemManager.h"
#include "BT_WindowsOS.h"
#include "BT_CommonFiniteStates.h"

// -----------------------------------------------------------------------------

	/*
	 * Prepare the scene for the tests.
	 */
	class PrepareSceneForTests : public FiniteState
	{
	public:
		PrepareSceneForTests(unsigned int duration)
		: FiniteState(duration)
		{}

		// FiniteState
		virtual void	onStateChanged()
		{
			// copy over all the test files we need
			QDir dataPath = parent(winOS->GetTexturesDirectory()) / "AutomatedTestData";
			QDir desktopPath = scnManager->getWorkingDirectory();
			QString desktopPathStr = native(desktopPath);

			vector<QString> dirList = fsManager->getDirectoryContents(native(dataPath));
			for (int i = 0; i < dirList.size(); ++i)
			{
				QString fileName = filename(dirList[i]);
				if (!exists(desktopPath / fileName))
					fsManager->copyFileByName(dirList[i], desktopPathStr, fileName, false, true);
			}
		}

		virtual bool	finalizeStateChanged()
		{
			// ensure that all the textures have finished loading
			bool allImagesLoaded = true;
			QDir desktopPath = scnManager->getWorkingDirectory();
			QString desktopPathStr = native(desktopPath);
			vector<FileSystemActor *> imgFiles = scnManager->getFileSystemActors("\\_test[^\\.]*\\.jpg", false, true);
			for (int i = 0; (i < imgFiles.size()) && allImagesLoaded; ++i)
			{
				allImagesLoaded = allImagesLoaded &&
					texMgr->isTextureState(imgFiles[i]->getThumbnailID(), TextureLoaded);
			}	
			return allImagesLoaded;
		}
	};

	/*
	 * Finalize the scene after the tests.
	 */
	class FinalizeSceneAfterTests : public FiniteState
	{
	public:
		FinalizeSceneAfterTests(unsigned int duration)
		: FiniteState(duration)
		{}

		// FiniteState
		virtual void	onStateChanged()
		{
			// clean up any lingering files if they still exist
			QDir dataPath = parent(winOS->GetTexturesDirectory()) / "AutomatedTestData";
			QDir desktopPath = scnManager->getWorkingDirectory();
			QString desktopPathStr = native(desktopPath);

			vector<QString> dirList = fsManager->getDirectoryContents(native(dataPath));
			for (int i = 0; i < dirList.size(); ++i)
			{
				QString fileName = filename(dirList[i]);
				QFileInfo desktopFilePath = make_file(desktopPath, fileName);
				if (exists(desktopFilePath))
					fsManager->deleteFileByName(native(desktopFilePath));
			}
		}
	};

// -----------------------------------------------------------------------------

	class ATS_General : public AutomatedTestSuite
	{
	public:
		ATS_General()
		: AutomatedTestSuite("General Tests", "Tests various general behaviours in BumpTop")
		{
			QDir desktopPath = scnManager->getWorkingDirectory();
			QString desktopPathStr = native(desktopPath);

#define NEW_FIRST_STATE(newState) prevState = tmpState; tmpState = newState; _firstState = tmpState; _states.addState(tmpState);  
#define NEW_STATE(newState) prevState = tmpState; tmpState = newState; _states.addState(tmpState); _states.addTransition(prevState, tmpState)

			// ---- Test Case 1 ----

			// Prepare Scene
			FiniteState * tmpState = NULL, * prevState = NULL;
			NEW_FIRST_STATE(new PrepareSceneForTests(1));
			
			// Finalize Scene
			NEW_STATE(new FinalizeSceneAfterTests(1));
			// mark this state as the finalization clean up
			_finalizeState = tmpState;

			// ---- Test Case 2 ----


#undef NEW_STATE
#undef NEW_FIRST_STATE

		}
	};

// -----------------------------------------------------------------------------

	class ATS_Cleanup : public AutomatedTestSuite
	{
	public:
		ATS_Cleanup()
			: AutomatedTestSuite("Cleanup Tests", "Tests the cleanup of various bumptop states")
		{
			QDir desktopPath = scnManager->getWorkingDirectory();
			QString desktopPathStr = native(desktopPath);

#define NEW_FIRST_STATE(newState) prevState = tmpState; tmpState = newState; _firstState = tmpState; _states.addState(tmpState); 
#define NEW_STATE(newState) prevState = tmpState; tmpState = newState; _states.addState(tmpState); _states.addTransition(prevState, tmpState)

			// TEST_CASE: Deleting all the items from a soft pile should destroy the pile
			FiniteState * tmpState = NULL, * prevState = NULL;
			NEW_FIRST_STATE(new PrepareSceneForTests(1));							// prepare Scene			
			NEW_STATE(new CreatePileState(2, "\\_test[^\\.]*\\.txt"));				// create pile of txt files
			NEW_STATE(new DeleteFilesState(3, "\\_test[^\\.]*\\.txt"));				// delete txt files
			NEW_STATE(new AssertNoPilesWithFileSystemActorsState(4, "\\_test[^\\.]*\\.txt"));	// ensure that the pile created earlier no longer exists
			NEW_STATE(new FinalizeSceneAfterTests(5));								// clean up

			// TEST_CASE: Deleting all the items from a hard pile should folderize that pile
			NEW_FIRST_STATE(new PrepareSceneForTests(6));							// prepare Scene			
			NEW_STATE(new AssertBumpObjectCount(7, BumpPile, 0));
			NEW_STATE(new CreatePileState(8, "\\_test[^\\.]*\\.jpg"));				// create pile of image files
			NEW_STATE(new AssertBumpObjectCount(9, BumpPile, 1));
			NEW_STATE(new SelectPilesWithFileSystemActorsState(10, "\\_test[^\\.]*\\.jpg"));
			NEW_STATE(new FolderizeSelectedPileState(1250));						// folderize the pile
			NEW_STATE(new AssertBumpObjectCount(11, BumpPile, 0));
			NEW_STATE(new AssertBumpObjectExists(12, "Piled Stuff", Folder, true));		// ensure that the pile was folderized to the correct name

			NEW_STATE(new SelectFileSystemActorsState(13, "Piled Stuff"));			
			NEW_STATE(new PileizeSelectedFileSystemActorState(1250));				// pilelize that new fs actor
			NEW_STATE(new AssertBumpObjectCount(14, BumpPile, 1));
			NEW_STATE(new DeleteFilesState(1250, "\\_test[^\\.]*"));
			NEW_STATE(new AssertBumpObjectCount(17, BumpPile, 0));
			NEW_STATE(new AssertBumpObjectExists(18, "Piled Stuff", Folder, true));
			NEW_STATE(new DeleteFilesState(19, "Piled Stuff"));
			NEW_STATE(new FinalizeSceneAfterTests(20));								// clean up

			// TEST_CASE: Deleting a file that is being watched through the slideshow ends the slideshow
			NEW_FIRST_STATE(new PrepareSceneForTests(1));							// prepare Scene			
			NEW_STATE(new AssertBumpObjectCount(2, BumpPile, 0));
			NEW_STATE(new ZoomIntoImageState(1250, "\\_test[^\\.]*\\.jpg"));			// start the slideshow
			NEW_STATE(new AssertSlideShowEnabled(4, true));
			NEW_STATE(new DeleteFilesState(1250, "\\_test[^\\.]*\\.jpg"));			// delete all the images being watched
			NEW_STATE(new AssertSlideShowEnabled(5, false));
			NEW_STATE(new FinalizeSceneAfterTests(6));								// clean up

			// TEST_CASE: filesystem actors, piles (hard/soft) can be grown and shrunk
			NEW_FIRST_STATE(new PrepareSceneForTests(1));							// prepare Scene			
			NEW_STATE(new CreatePileState(2, "\\_test[^\\.]*\\.txt"));				// create pile of txt files																					
			NEW_STATE(new SelectPilesWithFileSystemActorsState(3, "\\_test[^\\.]*\\.txt"));	// convert it into a hard pile
			NEW_STATE(new FolderizeSelectedPileState(1250));				
			NEW_STATE(new SelectFileSystemActorsState(4, "Piled Stuff"));
			NEW_STATE(new PileizeSelectedFileSystemActorState(1250));
			NEW_STATE(new CreatePileState(5, "\\_test[^\\.]*\\.jpg"));				// create pile of jpg files
			NEW_STATE(new ResizeActorsState(6, "", BumpPile, true));
			NEW_STATE(new ResizeActorsState(7, "\\_test[^\\.]*\\.dll", FileSystem, true));
			Vec3 defaultDims(GLOBAL(settings).xDist, GLOBAL(settings).yDist, GLOBAL(settings).zDist);
			NEW_STATE(new AssertBumpObjectDimensions(8, "", BumpPile, defaultDims, AssertBumpObjectDimensions::GreaterThan));
			NEW_STATE(new AssertBumpObjectDimensions(9, "\\_test[^\\.]*\\.dll", FileSystem, defaultDims, AssertBumpObjectDimensions::GreaterThan));
			NEW_STATE(new ResizeActorsState(10, "", BumpPile, false));
			NEW_STATE(new ResizeActorsState(11, "\\_test[^\\.]*\\.dll", FileSystem, false));
			NEW_STATE(new ResizeActorsState(12, "", BumpPile, false));
			NEW_STATE(new ResizeActorsState(13, "\\_test[^\\.]*\\.dll", FileSystem, false));
			NEW_STATE(new AssertBumpObjectDimensions(14, "", BumpPile, defaultDims, AssertBumpObjectDimensions::LessThan));
			NEW_STATE(new AssertBumpObjectDimensions(15, "\\_test[^\\.]*\\.dll", FileSystem, defaultDims, AssertBumpObjectDimensions::LessThan));
			NEW_STATE(new FinalizeSceneAfterTests(16));								// clean up
			

			/*
			// TEST_CASE: Ensuring that folderization works
			NEW_FIRST_STATE(new PrepareSceneForTests(1));							// prepare Scene			
			NEW_STATE(new CreatePileState(1, "\\_test[^\\.]*\\.jpg"));				// create pile of image files
			NEW_STATE(new SelectPilesWithFileSystemActorsState(1, "\\_test[^\\.]*\\.jpg"));
			NEW_STATE(new FolderizeSelectedPileState(1));							// folderize the pile
			NEW_STATE(new AssertNoFileSystemActorsExist(1, "\\_test[^\\.]*\\.jpg"));	// ensure that all images were folderized
			NEW_STATE(new AssertFileSystemActorsExist(1, "Piled Stuff"));			// ensure that the pile was folderized to the correct name
			NEW_STATE(new AssertNoFileSystemActorsExist(1, "Piled Stuff 1"));		
			NEW_STATE(new AssertNoFileSystemActorsExist(1, "Piled Stuff 2"));		
			NEW_STATE(new FinalizeSceneAfterTests(1));								// clean up

			// TEST_CASE: Ensuring that repeated folderization works
			NEW_FIRST_STATE(new PrepareSceneForTests(1));							// prepare Scene			
				// folderize first pile
			NEW_STATE(new CreatePileState(1, "\\_test[^\\.]*\\.jpg"));				// create pile of image files
			NEW_STATE(new SelectPilesWithFileSystemActorsState(1, "\\_test[^\\.]*\\.jpg"));
			NEW_STATE(new FolderizeSelectedPileState(1));							// folderize the pile
			NEW_STATE(new AssertNoFileSystemActorsExist(1, "\\_test[^\\.]*\\.jpg"));	// ensure that all images were folderized
			NEW_STATE(new AssertFileSystemActorsExist(1, "Piled Stuff"));			// ensure that the pile was folderized to the correct name
				// folderize second pile
			NEW_STATE(new CreatePileState(1, "\\_test[^\\.]*\\.txt"));				// create pile of txt files
			NEW_STATE(new SelectPilesWithFileSystemActorsState(1, "\\_test[^\\.]*\\.txt"));
			NEW_STATE(new FolderizeSelectedPileState(1));							// folderize the pile
			NEW_STATE(new AssertNoFileSystemActorsExist(1, "\\_test[^\\.]*\\.txt"));	// ensure that all images were folderized
			NEW_STATE(new AssertFileSystemActorsExist(1, "Piled Stuff 1"));			// ensure that the pile was folderized to the correct name
				// folderize third pile

				
			NEW_STATE(new FinalizeSceneAfterTests(1));								// clean up
			*/
			
			// Finalize Scene
			// NEW_STATE(new FinalizeSceneAfterTests(1));
			// mark this state as the finalization clean up
			// _finalizeState = tmpState;

			// ---- Test Case 2 ----


#undef NEW_STATE
#undef NEW_FIRST_STATE

		}
	};

#endif // BT_ATESTGENERAL