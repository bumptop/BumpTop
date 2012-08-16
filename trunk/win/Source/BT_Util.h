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

#ifndef _BT_UTIL_
#define _BT_UTIL_

#include "BT_Singleton.h"
#include "BT_Stopwatch.h"

class AnimationEntry;
class FileSystemActor;
class FileSystemPile;
class Pile;
class Actor;
class BumpObject;
class NxActorWrapper;
class TextOverlay;
class Library;

// -----------------------------------------------------------------------------

#define PI						3.14159265358979f
#define NONE					(-1)

// -----------------------------------------------------------------------------

enum EaseFunctionType
{
	NoEase		= (1 << 0),
	Ease		= (1 << 1),
	SoftEase	= (1 << 2)
};

// -----------------------------------------------------------------------------

// workaround to allow for translated strings
class BtUtilStrings
{
	Q_DECLARE_TR_FUNCTIONS(BtUtilStrings)

private:
	QHash<QString, QString> _strs;

	friend class Singleton<BtUtilStrings>;
	BtUtilStrings();

public:
	QString getString(QString key);
	QString tryGetString(QString key);
};
#define BtUtilStr Singleton<BtUtilStrings>::getInstance()

// -----------------------------------------------------------------------------
void SwitchToLibrary(QSharedPointer<Library> library);
void RemoveLibrary(QSharedPointer<Library>& library);
void CleanUpSceneFiles();
float roundOffDecimal(float number);
Vec3& roundOffDecimals(Vec3& vecNum);
int getRemainingTrialDays();
void printTimedUnique(QString key, int time, QString str);
void DrawSphere(Vec3 center, float radius, int slicesAndStacks);
void DrawEllipse(Vec3 center, float height, float width);
TextOverlay* CreateGoProFingerMessage();
bool isPointInPolygon(const Vec3& point, Vec3List &polygonPoints);
float calculateAreaOfTriangle(const Vec3 &firstPoint, const Vec3 &secondPoint, const Vec3 &thirdPoint);
float calculateAreaOfPolygon(const Vec3List &polygonPoints);
Vec3 calculatePolygonCentroid(const Vec3List &polygonPoints);
Vec3List createEllipse(Vec3 center, float width, float height);
void glutSolidCube(GLdouble size);
void glutSolidSphere(GLdouble radius, GLint slices, GLint stacks);
void glutSolidCube(GLdouble size);
void DeleteActor(NxActorWrapper* a);
Vec3 minAABBvalues(Box obb);
Vec3 maxAABBvalues(Box obb);
QSet<BumpObject *> getAllObjectsUnderCursor(int x, int y);
void oneSecondTick();
bool adjustBoxToInsideWalls(Box &innerBoxOut);
Vec3 adjustPtOnWall(BumpObject * obj, NxActorWrapper * wall);
void *CreateBumpObjectsFromDirectory(QString DirectoryPath, bool createFirstRunObjects = true);
void CreateBumpObjectsFromWorkingDirectory(bool createFirstRunObjects = true);
void CreateBumpObjectsFromLibrary(QSharedPointer<Library>& library, bool createFirstRunObjects = true);
BumpObject *GetBumpObject(NxActorWrapper *a);
void startConsoleWin(int width=240, int height=999, char* fname = "ErrorLog.txt");
void qtMessageHandler(QtMsgType type, const char *msg);
int consoleWrite(const QString& message);
int consoleWrite(const QString& name, const Vec3& vec);
void fghCircleTable(float **sint,float **cost,const int n);
deque<Mat34> lerpMat34RangeRaw(const Mat34 & begin, const Mat34 & end, const int count, EaseFunctionType easeType=Ease);
float AngleBetweenVectors(Vec3 one, Vec3 two);
float maxVec3Dim(const Vec3 & v);
float ease(float t);
float softEase(float t);
bool approxEqual(float a, float b, float tolerance);
bool approxEqual(Vec3 a, Vec3 b, float tolerance);
void LineLineIntersection(Vec3 o1, Vec3 dir1, Vec3 o2, Vec3 dir2, float & t1, float & t2);
Vec3 ClosestPointOnLineToPoint(Vec3 o, Vec3 dir, Vec3 pt);
bool RayLineSegmentIntersection(Vec3 p, Vec3 dir, Vec3 a, Vec3 b, Vec3 & intPoint, float & T2);
bool LineSegLineSegIntersection(Vec3 a, Vec3 b, Vec3 c, Vec3 d);
float RayPointDistSq(Vec3 p, Vec3 rayP, Vec3 rayDir);
tuple<int, NxReal, Vec3, NxPlane> RayIntersectDesktop(Ray ray, float padding = 0.0f);
void PeeledPoly(Vec3List ps, Vec3 P, Vec3 P_, Vec3List & peeledPs, Vec3List & uvs, Vec3List & reflectedPart);
void drawAxis(float length, bool positiveOnly=true);
void DrawSphere(Vec3 center, float radius, int slicesAndStacks);
void unpick();
tuple<NxActorWrapper*, BumpObject*, Vec3> pick(int x, int y);
void pickAndSet(int x, int y);	//pick an object (stab)
Actor *GetBumpActor(NxActorWrapper *a);
void FreezeActor(NxActorWrapper* a, bool freeze=true);
NxActorWrapper* CreateBumpIcon(Vec3 pos, Vec3 dims, int textureNum, bool staticActor = false, const Vec3 & initial_velocity = Vec3(0,0,0), float density = 10.0f, bool fadeIn = false);
NxActorWrapper* CreateCuboid(const Vec3 pos = Vec3(0,0,0), float l = 7, float w = 7, float h = 1, bool staticActor = false, const Vec3 initial_velocity = Vec3(0,0,0), float density = 10.0f);
NxActorWrapper* CreateCuboidOri(const Mat34& pose, float l, float w, float h, bool staticActor = false, const Vec3 & initial_velocity = 0, float density = 10.0f);
void DisableRotation(NxActorWrapper* a, bool justY=false);
void DisableRotation(vector<NxActorWrapper*> v);
void window2world(int x, int y, Vec3 & v, Vec3 & w);
bool isMatEqual(Mat34 a, Mat34 b);
void doNovodexTick(float timeElapsed);
NxActorWrapper *LoadSingleFile(QString Inpt);
void PushActorAboveGround(NxActorWrapper* a, float extraPush=1.0, bool alwaysPush=false);
vector<NxActorWrapper*> GetDesktopItems();
bool isDesktopItem(NxActorWrapper* actor);
Actor *FadeAndDeleteActor(Actor *obj);
void DeletePile(Pile *p, bool deleteActors=false, bool fade=false, bool removeAnimation = true);
NxActorWrapper *SoftPileToFolderIcon(Pile *pile, QString folderName = NULL);
void setOnPinItemHandler(boost::function<void(BumpObject *obj)> onPinItemHandler);
void PinItemToWall(NxActorWrapper* u, bool breakable = true);
int createThumbnailFromFile(QString fileName, float &sizeX, float &sizeY);
void *ReshapeTextAfterAnim(AnimationEntry *animEntry);
void *PostIntroLoad(AnimationEntry *animEntry);
void *DeletePileAfterAnim(AnimationEntry *animEntry);
void *UpdatePileDimsAfterAnim(AnimationEntry *animEntry);
void *UpdatePileStateAfterLeafing(AnimationEntry *animEntry);
void *DeleteActorAfterAnim(AnimationEntry *animEntry);
void *SyncStickyNoteAfterResize(AnimationEntry *animEntry);
void EnableRotation(vector<NxActorWrapper*> v);
void EnableRotation(NxActorWrapper* a, bool justY=false);
void ExitBumptop();
Vec3 zoomToBounds(const Bounds& bounds, bool animate = true, int timeStep = 30);
Vec3 calculateEyeForBounds (const Bounds& bounds);
Vec3 zoomToAngledBounds(const Bounds& bounds, bool animate = true);
deque<Vec3> lerpQuardaticCurve(Vec3 startPoint, Vec3 topPoint, Vec3 endPoint, int numSteps, EaseFunctionType easeType=Ease);
deque<Vec3> lerpQuardaticFittingCurve(Vec3 startPoint, Vec3 topPoint, Vec3 endPoint, int numSteps, EaseFunctionType easeType=Ease);
void setDimsFromFileSize(NxActorWrapper *actor);
void world2window(const Vec3& point, int & x, int & y);
Vec3 world2window(const Vec3& point);
bool SERIALIZE_WRITE(unsigned char **bufPtr, uint dataSz, uint &streamSize, void *dataPtr);
bool SERIALIZE_WRITE_QSTRING(unsigned char **bufPtr, uint &streamSize, QString str);
bool SERIALIZE_WRITE_VEC3(unsigned char **bufPtr, uint &streamSize, Vec3 vec);
bool SERIALIZE_READ(unsigned char **bufPtr, void *dataPtr, uint varSize, uint &bufSz);
bool SERIALIZE_READ_STRING(unsigned char **bufPtr, QString& str, uint &bufSz);
bool SERIALIZE_READ_QSTRING(unsigned char **bufPtr, QString& str, uint &bufSz);
bool SERIALIZE_READ_VEC3(unsigned char **bufPtr, Vec3& vec, uint &bufSz);
void SaveSceneToFile();
bool LoadSceneFromFile();
void PruneJoints(NxActorWrapper *actor);
void *EnableGravityAfterAnim(AnimationEntry * entry);
void InitNovodex();
void ResizeWallsToWorkArea(int maxX, int maxY);
void handleNovodexCrash();
void CreateWalls();
void ClearBumpTop();
bool stripInvalidChars(QString &fileName);
void PushBelowGround(NxActorWrapper *actor);
void Key_ResetBumpTopLayout();
void Key_ResetBumpTopSettings();
void Key_DeleteSceneFiles();
void Key_SendFeedback();
void Key_ZoomToSavedCamera();
void Key_ZoomToAll();
void Key_ToggleInvisibleActors();
void Key_TriggerUndo();
void Key_TriggerRedo();
void Key_TogglePrintMode();
void Key_ShowConsole();
void Key_ShowSettingsDialog();
void Key_ShowSettingsDialogWindow(QString paramsStr);
void Key_NextMonitor();
void Key_Test();
void Key_ToggleUserAgent();
void Key_CreateWebActor();
void Key_ReloadWebActor();
void Key_ToggleSharingMode();
void Key_UnsetSharingMode();
void Key_ToggleProfiling();
void Key_ForceDump();
void Key_SetLaunchOverride();
void Key_CreateCustomActor();
void Key_CreateStickyNote();
void Key_ToggleCustomEmailActor();
void Key_ToggleCustomStickyNotePadActor();
void Key_ToggleSharingWidget();
void Key_ToggleFacebookWidget();
void Key_ToggleCustomPrinterActor();
void Key_ToggleCustomFacebookActor();
void Key_ToggleCustomTwitterActor();
void Key_ToggleCustomFlickrActor();
bool Key_IsEmailWidgetEnabled();
bool Key_IsStickyNotePadWidgetEnabled();
bool Key_IsSharingWidgetEnabled();
bool Key_IsFacebookWidgetEnabled();
bool Key_IsPrinterWidgetEnabled();
bool Key_IsTwitterWidgetEnabled();
void Key_ToggleFPS();
void Key_RefreshFileActors();
void setOnInvokeCommandHandler(boost::function<void()> onInvokeCommandHandler);
void Key_StartAutomatedDemo();
void Key_StartAutomatedTradeshowDemo();
void Key_StartAutomatedTests();
Json::Value getValueFromRoot (QString keyPath, Json::Value root);
void Key_LoadTradeshowScene();
void Key_ReloadScene();
void Key_SelectAll();
void Key_NameSoftPile();
void Key_SetCameraAsTopDown();
void Key_ToggleRepositionBounds();
void Key_MoveToDesktopOne();
void Key_MoveToDesktopTwo();
void Key_MoveToDesktopThree();
void Key_SetCameraAsAngled();
void Key_MoveCameraForward();
void Key_MoveCameraBackwards();
void Key_MoveCameraRight();
void Key_MoveCameraLeft();
void Key_ShootCuboid();
void ForcePush(Vec3 pointOfOriginScreenSpace);
void Key_ToggleDrawText();
void Key_ToggleCPUOptimization();
void Key_ToggleWindowMode();
void Key_PreviousSlide();
void Key_NextSlide();
void Key_RepollDesktop();
void Key_ClearBumpTop();
void Key_SetCameraOnSelection();
void Key_ToggleOtherBumpTops();
void Key_TogglePausePhysics();
void Key_ToggleCollisions();
void Key_ToggleFullScreen();
void Key_ToggleInfiniteDesktopMode();
void Key_ToggleSelectionText();
void Key_DeleteSelection();
void Key_LaunchSelection();
void Key_RenameSelection();
void Key_LogoutServiceSelection();
void Key_GridLayoutSelection();
void Key_SearchSubString();
void Key_DeleteSelectionSkipRecycleBin();
void Key_ToggleAxisAlignMode();
void Key_Shrink();
void Key_Grow();
void Key_ResetSize();
void setMakePileHandler( boost::function<void(Pile*)> onMakePileHandler );
void Key_MakePile();
void Key_CreatePileFromCluster();
void Key_ToggleSlideShow();
void Key_DisableSlideShow();
void Key_EnableSlideShow();
void Key_GridView();
void Key_LeafForwardSelection();
void Key_FanOut();
void Key_SortByType();
void Key_SortBySize();
void Key_SortByName();
void setPileByTypeHandler(boost::function<void()> onPileByTypeHandler);
QString sortIntoPilesKeyByType(const FileSystemActor * fsActor);
QString sortIntoPilesKeyByTime(const FileSystemActor * fsActor);
void gridLayoutPile(QList<pair<QString, vector<FileSystemActor *>>> & container, const Bounds & selectionBounds);
void Key_SortIntoPiles(bool groupSingleItemsIntoMisc, bool setPileByTypeHandler, QString ( * sortIntoPilesKey) (const FileSystemActor * fsActor));
void Key_SortIntoPilesByType();
void Key_SortIntoPilesByTime();
void Key_Folderize();
void Key_ClosePile();
void Key_BreakPile();
void Key_RemoveFromPile();
void Key_ExpandToPile();
void Key_PileizeAndGridView();
void Key_MoreOptionsHelper(POINT p, bool enableTracking);
void Key_MoreOptions();
void Key_StackPile();
void Key_ShowAbout();
void Key_ToggleDebugKeys();
void Key_ModifySelectedPhotoFrameSource();
void Key_DeleteSelectedPhotoFrameSource();
void Key_ShowSettingsOpenThemesTab();
void Key_ThemeDialog();
void Key_CreateNewPhotoFrame();
void Key_RunBumptopTests();
StrList GetFilteredSetForIDataObject(vector<BumpObject *>& selectionInOut, IShellFolder2 ** psfOut);
void Key_CopySelection();
void Key_CutSelection();
void Key_PasteSelection();
void Key_LaunchExplorerWithFileSelected();
void Key_LaunchExplorerWindowOnWorkingDirectory();
void Key_LaunchCommandWindowOnWorkingDirectory();
void Key_PlayPause();
void Key_OverrideFileTexture();
void Key_DumpFileSystemActorsToFile();
void Key_CreateSharingWidget();
void Key_CreateFacebookWidget();
void Key_CreateNewDirectory();
void Key_ToggleBubbleClusters();
bool isValidFilename(QString newName);

//bool SearchFlickR(String Inp);
int loadImageDirAsObjects(QString imgDirStr, int maxImgWidth=256, int maxImgHeight=256, int maxBumpObjDim=20, int bumpObjDepth=1, bool recursive=false, bool scaled = false);

Vec3 pointOnFloor(int x, int y);
void switchToPerspective();
void switchToOrtho();
void StartModeBasedOnSelection(int x, int y);
void FinishModeBasedOnSelection();

#ifdef DXRENDER
void RenderLasso(const QList<Vec3> &lasso);
#else
void RenderLine(const QList<Vec3>& points, bool inScreenSpace, int dotSize, float lineWidth=1.5);
void RenderLasso(const QList<Vec3> &lasso, bool useDottedLine=true, bool inScreenSpace=true);
#endif

void *updatePhantomActor(AnimationEntry &entry);
void CreateRandomAnimPath(NxActorWrapper *actor, Vec3 startPos, Vec3 endPos, int numFrames);
Vec3 getDimensions(NxActorWrapper *a);
FileSystemActor *GetFileSystemActor(NxActorWrapper *a);
bool adjustPointToInsideWalls(Vec3 &newPt);
bool adjustPointToInsideWalls(Vec3 &newPt, const Vec3& boxInset);
Box GetDesktopBox(float buffer=0.0f);
QString generateUniqueGUID();
void *updatePileStateAfterAnim(AnimationEntry *entry);
void orderSpatially2D(vector<BumpObject *>& objectsInOut, bool groupPileItems = false);
bool isSlideshowModeActive();
float getAngleBetween(Vec3 firstVector, Vec3 secondVector);
bool isParallel(const Mat33& firstOrientation, const Mat33& secondOrientation);
Vec3List getConvexHull(Vec3List points);
bool hasOriginalPhoto(FileSystemActor* photo);
bool backupOriginalPhoto(FileSystemActor* photo);
bool restoreOriginalPhoto(FileSystemActor* photo);
void loadDemoSceneFromDirectory(QDir sourceDir, bool showInfoControl=true);

bool isSharedFolderUrl(const QString& url);
QDir fromSharedFolderUrl(const QString& url);
QString getSharedFolderUrl(const QString& info=QString(""));

void animateObjectsBackToPreDropPose(const vector<BumpObject *>& objs);

// client coordinates are window coordinates
// window coordinates are 3d coordinates in bumptop space
Vec3 WorldToClient(Vec3 p, bool reverseY = true, bool zeroZ = false);
Vec3 CurrentWorldMousePos();
Vec3 ClientToWorldBox(int x, int y, float offset);
Vec3 ClientToWorld(int x, int y, float height = 0.0f);
tuple<int, NxReal, Vec3, NxPlane> unProjectToDesktop(int x, int y, float padding = 0.0f);
bool ToEulerAnglesXYZ(const Mat33 & MatrixEntry, float &rfXAngle, float &rfYAngle, float &rfZAngle);
bool ToEulerAngleZ(const Mat33 & MatrixEntry, float &rfZAngle);
float DistanceBetweenPoints(Vec3 one, Vec3 two);
deque<Mat34> slerpPose(const Mat34& begin, const Mat34& end, const int count, EaseFunctionType easeType);

// Testing functions
void startAutomatedTests();
void StartAutomatedJSONTesting();

QString printVec3(Vec3 v);
QString printMat33(Mat33 m);

// Method to determine how high objects should be pinned on walls
// when invoking reset layout
float getHeightForResetLayout(Box *desktopBox, float buffer = 0);

// -----------------------------------------------------------------------------

template <class T> vector<T> mergeVectors(const vector<T>& prev, const vector<T>& last);
template <class T> int vecContains(const vector<T>& vec, const T& value);
template <class T> T lerp(const T& begin, const T& end, const float t);
template <class T> bool inRange(T value, T low, T high);
template <class T> T clampVals(T value, T low, T high);
template <class T> deque<T> lerpRange(const T& begin, const T& end, const int count, EaseFunctionType easeType=Ease);
template<class T> deque<T> bounceGrow( T initialSize, T endSize, uint numSteps );
template<class T> deque<T> bounce( T cur, T magnitude, uint numSteps );

// -----------------------------------------------------------------------------
#define GL_OK(call) call; assert(glGetError() == GL_NO_ERROR);
#define GL_OK_EXT(call) call; GLenum glErr = glGetError(); assert(glErr == GL_NO_ERROR);
// -----------------------------------------------------------------------------
class glPushAttribToken
{
public:
	// inline these functions
	glPushAttribToken(GLbitfield mask)
	{
		glPushAttrib(mask);
	}

	~glPushAttribToken()
	{
		glPopAttrib();
	}
};

// ----------------------------------------------------------------------------

class ConsoleWriteGuard
{
	static int count;
	Stopwatch timer;
	QString key;
	int threshold;

public:
	ConsoleWriteGuard(QString k);
	ConsoleWriteGuard(QString k, int t);
	~ConsoleWriteGuard();
};

// -----------------------------------------------------------------------------

#ifndef _BT_UTIL_TEMPLATES_
#define _BT_UTIL_TEMPLATES_

// -----------------------------------------------------------------------------

#include "BT_Util.inl"

// -----------------------------------------------------------------------------

#endif
#endif
