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

#include "BT_Common.h"
#include "BT_Authorization.h"
#include "BT_FileSystemManager.h"
#include "BT_GLTextureManager.h"
#ifdef DXRENDER
	#include "BT_DXRender.h"
#endif
#include "BT_OverlayComponent.h"
#include "BT_QtUtil.h"
#include "BT_RenderManager.h"
#include "BT_SceneManager.h"
#include "BT_ThreadableUnit.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"

#include "BT_TexHelperManager.h"
#include "TexHelperResponse.h"

#define TRY_SLEEP_LOCK_WAIT 16
#define TRY_SLEEP_LOCK(m) while (!m.tryLock()) { Sleep(TRY_SLEEP_LOCK_WAIT); }
#define COMPRESSION_VERSION 1

// If we don't check for errors after each DevIL call, they can build up and
// cause confusing errors when we do check for errors.
// At the very least, use this macro after every DevIL call
#define IL_ASSERT_NO_ERROR() {ILenum IL_ERROR_CODE = ilGetError(); if (IL_NO_ERROR != IL_ERROR_CODE) \
								{ printf("IL_ERROR 0x%.4X %s(%d) \n", IL_ERROR_CODE, __FILE__, __LINE__); _ASSERT(false);}}

// Using GDI+; allocates data and copies pixels into data (ARGB), returns true if successful; caller needs to delete data
bool LoadPicture(const wchar_t * path, unsigned int & width, unsigned int & height, unsigned int & format, unsigned int & size, unsigned char * (& data)) 
{
	Gdiplus::Bitmap * bitmap = new Gdiplus::Bitmap(path);
	
	if (bitmap->GetLastStatus() != Gdiplus::Ok)
	{
		delete bitmap;
#ifdef DEBUG
		consoleWrite(QString("GDI+ Bitmap failed to load: %1 \n").arg(QString::fromUtf16((unsigned short *)path)));
#endif
		return false;
	}
	
	width = bitmap->GetWidth();
	height = bitmap->GetHeight();
	_ASSERT(bitmap->GetLastStatus() == Gdiplus::Ok);

	Gdiplus::BitmapData bitmapData = {0};
	Gdiplus::Rect rect(0,0,width,height);

	bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bitmapData);
	_ASSERT(bitmap->GetLastStatus() == Gdiplus::Ok);
	if (bitmap->GetLastStatus() != Gdiplus::Ok)
	{
		delete bitmap;
		return false;
	}

	format = bitmapData.PixelFormat;
	if (Gdiplus::GetPixelFormatSize(format) != 32) // DevIL do not have non 32bpp formats that match GDI formats
	{
		delete bitmap;
		return false;
	}
	
	unsigned char * scan0 = (unsigned char *)bitmapData.Scan0; 
	unsigned int stride = abs(bitmapData.Stride);
	data = new unsigned char [height * stride];
	size = sizeof(char) * height * stride;
	if (bitmapData.Stride < 0) // If the pixel data is stored bottom-up, the Stride data member is negative.
	{	
		_ASSERT(false); // ARGB format shouldn't have negative stride
		// Negative stride: scan0 points to the start of last (top) line
		memcpy(data, scan0 - (height - 1) * stride, height * stride);
	}
	else // DevIL is flipped from GDI (DevIL image is bottom up)
	{	
		for (unsigned int y=0; y<height; y++)
			memcpy(&(data[y * stride]), scan0 + stride * (height - y - 1), stride); 
	}

	bitmap->UnlockBits(&bitmapData);
	_ASSERT(bitmap->GetLastStatus() == Gdiplus::Ok);

	delete bitmap;

	return true;
}

// http://msdn.microsoft.com/en-us/library/ms533843(VS.85).aspx
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0; j < num; ++j)
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}    
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

// Put a stack allocated COMOLEResourceInitializer variable in each function requiring COM / OLE,
// so they will be automatically initialized and uninitialized when going out of scope.
class COMOLEResourceInitializer
{
	bool _comInitialized;
	bool _oleInitialized;
public:
	COMOLEResourceInitializer()
	{
		_oleInitialized = !FAILED(OleInitialize(NULL));
		_comInitialized = !FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED));
	}
	~COMOLEResourceInitializer()
	{
		if (_comInitialized)
			CoUninitialize();
		if (_oleInitialized)
			OleUninitialize();
	}
};

// ----------------------------------------------------------------------------

const float GLTextureManager::frameThicknessPercent = 0.05f;
const QColor GLTextureManager::frameBorderColor(237, 237, 237);

GLTextureManager::GLTextureManager()
: themeManagerRef(themeManager)
, _suspendLoading(false)
, _maxNumLoadingThreads(2)
{
	// set the texture directory
	_textureDirectory = winOS->GetTexturesDirectory();
	_cacheDirectory = winOS->GetCacheDirectory();
	_defaultGLTextureId = 0;
	_texHelperManager = new TexHelperManager(native(winOS->GetExecutableDirectory() / "TexHelper.exe"), _maxNumLoadingThreads);
}

GLTextureManager::~GLTextureManager()
{
	_suspendLoading = true;

	// NOTE: we don't really have to delete the textures because they are 
	// reference counted
	
	// stop listening for theme events
	if (shared_ptr<ThemeManager> tm = themeManagerRef.lock())
		tm->unregisterThemeEventHandler(this);

	// kill all active loading threads that might be using _texHelperManager then _texHelperManager 
	TextureThreads::iterator threadIter = _loadingThreads.begin();
	while (threadIter != _loadingThreads.end())
	{
		(*threadIter)->join(1);
		delete *threadIter;
		threadIter++;
	}	
	_loadingThreads.clear();
	SAFE_DELETE(_texHelperManager);

#ifdef DXRENDER
	SAFE_RELEASE(_defaultGLTextureId);
#endif
}

bool GLTextureManager::initIL()
{
	// initialize DevIL
	ilInit();
	iluInit();
	ilutInit();
#ifndef DXRENDER
	ilutRenderer(ILUT_OPENGL);
#endif
	ilHint(IL_MEM_SPEED_HINT, IL_FASTEST);
	IL_ASSERT_NO_ERROR();
	return true;
}

void GLTextureManager::initGLParams()
{
#ifndef DXRENDER
	// anisiotropic filtering
	float maximumAnistropy;
	if (GLOBAL(settings).useAnisotropicFiltering)
	{
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnistropy);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maximumAnistropy);
	}

	// border clamping (to fix the white border issue)
	if (GLEW_ARB_texture_border_clamp)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_ARB);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_ARB);
	}
	// clamping the uvs to ensure no texture bleeding
	// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// mipmap filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
#endif
}


bool GLTextureManager::init()
{
	// load the default texture
	// NOTE: we don't have to lock the IL mutex because the worker threads have not
	// yet started
#ifdef DXRENDER
	loadDefaultTexture();
#else
	QFileInfo defaultImage = make_file(_textureDirectory, "square.png");
	QString defaultImagePath = native(defaultImage);
	ILuint defaultILImageId = ilGenImage();
	ilBindImage(defaultILImageId);
	bool loaded = ilLoadImage((ILconst_string) defaultImagePath.utf16());
	if (!loaded)
	{
		ilDeleteImage(defaultILImageId);
		IL_ASSERT_NO_ERROR(); 
		return false;
	}
	_defaultGLTextureId = ilutGLBindMipmaps();
	ilDeleteImage(defaultILImageId);
	IL_ASSERT_NO_ERROR();
#endif

	// load the thumbs db path for the working directory for windows xp
	if (winOS->IsWindowsVersion(WindowsXP))
	{
		QString workingDirPath = native(scnManager->getWorkingDirectory());
		appendWinThumbnailSearchPath(workingDirPath);
	}

	// queue the bumptop-related textures
	queueBumpTopTextures();

	// bind the theme event handler
	themeManager->registerThemeEventHandler(this);
	_suspendLoading = true;
	IL_ASSERT_NO_ERROR();

	return true;
}

void GLTextureManager::queueBumpTopTextures()
{
	QString textureDir = native(_textureDirectory);
	ensurePathSlash(textureDir);

	// regulars	
	loadPersistentTexture(GLTextureObject(Load|Compress, QT_NT("cameraZoomOut"), textureDir + QT_NT("zoom_out.png"), HiResImage, LowPriority));
	loadPersistentTexture(GLTextureObject(Load, QT_NT("wall.tb.ao_overlay"), textureDir + QT_NT("ao_top_bottom_wall_256.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load, QT_NT("wall.lr.ao_overlay"), textureDir + QT_NT("ao_top_bottom_wall_256.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load, QT_NT("floor.ao_overlay"), textureDir + QT_NT("ao_floor_256.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load, QT_NT("volumeline.glow"), textureDir + QT_NT("line_glow.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress, QT_NT("photoframe.loading"), textureDir + QT_NT("loadingPhotoFrame.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress, QT_NT("photoframe.empty"), textureDir + QT_NT("emptyPhotoFrame.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress, QT_NT("icon.removable.drive"), textureDir + QT_NT("removable_drive.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress, QT_NT("icon.removable.disc"), textureDir + QT_NT("removable_disc.png"), HiResImage, HighPriority));
	// loadPersistentTexture(Load|Compress, GLTextureObject(QT_NT("overlay.background"), textureDir + QT_NT("overlayBackground.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("icon.custom.facebook"), textureDir + QT_NT("facebook.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("icon.custom.twitter"), textureDir + QT_NT("twitter.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("icon.custom.flickr"), textureDir + QT_NT("flickr.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("icon.custom.stickyNotePad"), textureDir + QT_NT("stickyNotePad.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("icon.custom.stickyNotePadDisabled"), textureDir + QT_NT("stickyNotePadDisabled.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Reload, QT_NT("pui.divider"), textureDir + QT_NT("pui-divider.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Reload, QT_NT("pui.close"), textureDir + QT_NT("pui-close.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Reload, QT_NT("pui.edit"), textureDir + QT_NT("pui-editor.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Reload, QT_NT("pui.email"), textureDir + QT_NT("pui-email.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Reload, QT_NT("pui.facebook"), textureDir + QT_NT("pui-facebook.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Reload, QT_NT("pui.next"), textureDir + QT_NT("pui-next.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Reload, QT_NT("pui.previous"), textureDir + QT_NT("pui-previous.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Reload, QT_NT("pui.print"), textureDir + QT_NT("pui-print.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Reload, QT_NT("pui.twitter"), textureDir + QT_NT("pui-twitter.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Reload, QT_NT("multitouch.touchpoint"), textureDir + QT_NT("touch_point.png"), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Reload, QT_NT("library_default.desktop.icon"), textureDir + QT_NT("desktop_library.png"), HiResImage, NormalPriority));
	loadPersistentTexture(GLTextureObject(Load|Reload, QT_NT("library_default.folder.icon"), textureDir + QT_NT("folder_library.png"), HiResImage, NormalPriority));
	
	// Demo "sharing" mode -- only enable sharing mode if all the textures exist.
	// When in sharing mode, pressing F5 will zoom out to show other desktops
	// where files can be dragged. Also, if a hard pile named "Bump Shared Wall"
	// exists, and it is placed in a vertical grid on the wall, we show a
	// Live Mesh widget which, when clicked, launches a separate instance of
	// BumpTop pointed at that folder using -d.
	if (false &&		//DISABLE sharing mode for now
		exists(make_file(textureDir, QT_NT("multi_me.png"))) &&
		exists(make_file(textureDir, QT_NT("multi_left.png"))) && 
		exists(make_file(textureDir, QT_NT("multi_top.png"))) && 
		exists(make_file(textureDir, QT_NT("toshibaFrame.png"))) && 
		exists(make_file(textureDir, QT_NT("vista_bg.jpg"))) && 
		exists(make_file(textureDir, QT_NT("vista_bg2.jpg"))) &&
		exists(make_file(textureDir, QT_NT("live_mesh.png"))))
	{
		GLOBAL(enableSharingMode) = true;
		loadPersistentTexture(GLTextureObject(Load|Compress, QT_NT("user.me"), textureDir + QT_NT("multi_me.png"), HiResImage, LowPriority));
		loadPersistentTexture(GLTextureObject(Load|Compress, QT_NT("user.left"), textureDir + QT_NT("multi_left.png"), HiResImage, LowPriority));
		loadPersistentTexture(GLTextureObject(Load|Compress, QT_NT("user.top"), textureDir + QT_NT("multi_top.png"), HiResImage, LowPriority));
		loadPersistentTexture(GLTextureObject(Load|Compress, QT_NT("user.overlay.toshibaFrame"), textureDir + QT_NT("toshibaFrame.png"), HiResImage, LowPriority));
		loadPersistentTexture(GLTextureObject(Load|Compress, QT_NT("user.bg.vista"), textureDir + QT_NT("vista_bg.jpg"), HiResImage, LowPriority));
		loadPersistentTexture(GLTextureObject(Load|Compress, QT_NT("user.bg.vista_2"), textureDir + QT_NT("vista_bg2.jpg"), HiResImage, LowPriority));
		loadPersistentTexture(GLTextureObject(Load|Compress, QT_NT("icon.live_mesh"), textureDir + QT_NT("live_mesh.png"), HiResImage, LowPriority));
	}
	IL_ASSERT_NO_ERROR();
}

int GLTextureManager::loadImageThread(GLTextureObject obj)
{	
	// ConsoleWriteGuard guard(QString("Load Image Thread: %1").arg(obj.key));

	int ilImageId = -1;
	if (obj.operation & Load)
	{
		// pass onto the appropriate load function
		if (obj.srcDetail == FileIcon)
			ilImageId = loadFileIcon(obj);
		else if (obj.srcDetail == SampledImage)
			ilImageId = loadSampledImage(obj);
		else if (obj.srcDetail == HiResImage)
		{
			// compress the texture if necessary
			QString loadPath = obj.srcPath;
			if ((obj.operation & Compress)
#ifdef DXRENDER
				&& false // TODO DXR DXT texture compression 
#else
				&& GLEW_EXT_texture_compression_s3tc
#endif
				)
			{
				// compress and cache the file
				QString cLoadPath = ensureCompressedImage(obj);
				if (!cLoadPath.isEmpty())
					loadPath = cLoadPath;
				else
				{
					// NOTE: this means that we will load the full resolution
					//		 image of this file
				}
			}

			ilImageId = loadHiResImage(loadPath);
		}
		else
			assert(false);
	}
	IL_ASSERT_NO_ERROR();
	
	if (0xFFFFFFFF != obj.texhelperId && _texHelperManager)
	{
		_ASSERT(_texHelperManager->isPipeBusy(obj.texhelperId));
		_texHelperManager->freePipe(obj.texhelperId);
		obj.texhelperId = 0xFFFFFFFF;
	}	
	return ilImageId;
}

int GLTextureManager::loadFileIcon(const GLTextureObject& obj)
{
	// ConsoleWriteGuard guard("\t\tloadFileIcon");

	// initialize COM on this thread for the win32 icon extraction calls
	OleInitialize(NULL);
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	// load the icon
	int ilImageId = -1;
	Gdiplus::Bitmap * bitmap = NULL;
	int virtualIconIndex = winOS->GetIconTypeFromFileName(obj.srcPath);
	if (virtualIconIndex > -1)
	{
		// this is a virtual folder
		bitmap = winOS->GetIconGraphic(virtualIconIndex);
	}
	else
	{
		// this is an icon index
		bitmap = winOS->GetIconGraphic(obj.srcPath);
	}

	if (bitmap)
	{
		// convert/copy the bitmap into a devil image
		unsigned int width = bitmap->GetWidth();
		unsigned int height = bitmap->GetHeight();
		Gdiplus::Rect bitmapRect(0, 0, width, height);
		Gdiplus::BitmapData * bitmapData = new Gdiplus::BitmapData;
		if (Gdiplus::Ok == bitmap->LockBits(&bitmapRect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, bitmapData))
		{
			TRY_SLEEP_LOCK(_ilMutex)
				int imgId = ilGenImage();
				ilBindImage(imgId);
				if (ilTexImage(width, height, 1, 4, IL_BGRA, IL_UNSIGNED_BYTE, bitmapData->Scan0))
					ilImageId = imgId;
				else
					ilDeleteImage(imgId);
			_ilMutex.unlock();
			bitmap->UnlockBits(bitmapData);
		}
		SAFE_DELETE(bitmapData);
		SAFE_DELETE(bitmap);
	}

	// uninitialize
	CoUninitialize();
	OleUninitialize();
	IL_ASSERT_NO_ERROR();
	return ilImageId;
}

int GLTextureManager::loadSampledImage(const GLTextureObject& obj)
{
	// ConsoleWriteGuard guard(QString("\t\tloadSampledImage: %1").arg(obj.key));

	// initialize COM on this thread for the win32 icon extraction calls
	COMOLEResourceInitializer comOLEResource;

	bool thumbnailLoaded = false;

	// try and load the thumbnail from window's thumbs db
	int ilImageId = -1;
	/* TODO: reimplement BT_ThumbsDBReader
	if (winOS->IsWindowsVersion(WindowsXP))
	{
		byte * data = NULL;
		size_t numBytes = 0;

		// find the thumbnailer and load the thumbnail data
		QString lowerPath = native(parent(obj.srcPath)).toLower();
		_thumbsDbsMutex.lock();
			ThumbsDbs::const_iterator iter = _thumbsDbs.find(lowerPath);
			if (iter != _thumbsDbs.end())
			{
				QString fileName = filename(obj.srcPath);
				data = iter.value()->getThumbData((LPCWSTR) fileName.utf16(), numBytes);
			}
		_thumbsDbsMutex.unlock();

		// load the thumbnail data into an IL image
		if (data && numBytes > 0)
		{
			TRY_SLEEP_LOCK(_ilMutex)
				int imgId = ilGenImage();
				ilBindImage(imgId);
				if (ilLoadL(IL_JPG, data, numBytes))
				{
					// convert the image to bgra if necessary
					if (ilGetInteger(IL_IMAGE_FORMAT) != IL_BGR)
						ilConvertImage(IL_BGR, IL_UNSIGNED_BYTE);

					// flip windows bitmaps since they are defined differently
					iluRotate(-180.0f);
					ilImageId = imgId;
				}
			_ilMutex.unlock();

			// free the thumbs db image memory
			SAFE_DELETE_ARRAY(data);
		}
	}
	else*/
	if (winOS->IsWindowsVersionGreaterThanOrEqualTo(WindowsVista))
	{
		// run the process to make sure it's up to date
		TRY_SLEEP_LOCK(_ilMutex);
			QString cacheDir = native(_cacheDirectory);
		_ilMutex.unlock();
		ensurePathSlash(cacheDir);
		QDir().mkpath(cacheDir);

		IL_ASSERT_NO_ERROR();
		// Setup appropriate text to pass to TexHelper
		QString texHelperAppStr = native(winOS->GetExecutableDirectory() / "TexHelper.exe");
		QString qpath = obj.srcPath;		
		QString qdir = cacheDir;
		QString thumbPath = generateCacheFilePath(obj.srcPath, obj.imageDimensions);

		// Ensure strings are quoted
		ensureQuoted(qpath);
		ensureQuoted(qdir);
		ensureQuoted(thumbPath);

		IL_ASSERT_NO_ERROR();

		// Construct command line arguments
		QString commandLine = QString("-f%1 -d%2 -t%3 -v\"\"")
			.arg(qpath).arg(qdir)
			.arg(thumbPath);
		
		// Run the process if the file doesn't exist
		thumbPath = cacheDir + generateCacheFilePath(obj.srcPath, obj.imageDimensions);
		int texHelperExitCode = 0;
		void * imageData = NULL;
		TexHelperResponse response;
		if (!QFile::exists(thumbPath))
			texHelperExitCode = createTexHelperProcess(texHelperAppStr, commandLine, obj.texhelperId, 2000, &response, &imageData);
		IL_ASSERT_NO_ERROR();
		// Load the texture that was just saved
		if (!texHelperExitCode)
		{
			TRY_SLEEP_LOCK(_ilMutex);
			int imgId = ilGenImage();
			ilBindImage(imgId);
			
			IL_ASSERT_NO_ERROR();
			
			if (response.ImageData)
			{
				ilTexImage(response.ImageWidth, response.ImageHeight, 1, 4, response.ImageFormat, IL_UNSIGNED_BYTE, imageData);
				IL_ASSERT_NO_ERROR();
				SAFE_DELETE_ARRAY(imageData);

				iluRotate(-180.0f); // flip windows bitmaps since they are defined differently
				IL_ASSERT_NO_ERROR();
				ilImageId = imgId;
			}
			else if (ilLoad(IL_DDS, (ILstring)thumbPath.utf16()))
			{
				// flip windows bitmaps since they are defined differently
				iluRotate(-180.0f);
				IL_ASSERT_NO_ERROR();
				ilImageId = imgId;
			}
			else
			{
				//some file types such as .EXE, .TXT could request a sample that does not exist; cause is in FileSystemActor::setFilePath : 583
				int newErrorCode = ilGetError();
#ifdef DEBUG
				wprintf(L"GLTextureManager::loadSampledImage 0x%.04X failed to load thumbPath=\"%s\" srcPath=\"%s\" \n", newErrorCode, thumbPath.utf16(), obj.srcPath.utf16());
#endif
				_ASSERT(newErrorCode == IL_COULD_NOT_OPEN_FILE);
			}

			_ilMutex.unlock();
		}
	}

	// if the windows thumbnail loading failed, try and thumbnail the image ourselves manually
	if (ilImageId < 0)
	{
		IL_ASSERT_NO_ERROR();
		QString ext = fsManager->getFileExtension(obj.srcPath);
		if(GLOBAL(supportedExtensions).contains(ext + "."))
		{
#ifdef DXRENDER
			// TODO DXR texture compression
			QString cLoadPath = ensureThumbnail(obj, false);
#else
			QString cLoadPath = ensureThumbnail(obj, GLEW_EXT_texture_compression_s3tc);
#endif
			if (!cLoadPath.isEmpty())
				ilImageId = loadHiResImage(cLoadPath);
			else
				consoleWrite(QString("%1 Failed to thumbnail.\n").arg(obj.srcPath));
		}
	}

	IL_ASSERT_NO_ERROR();

	return ilImageId;
}

int GLTextureManager::loadHiResImage( QString loadPath)
{
	// ConsoleWriteGuard guard(QString("\t\ttloadHiResImage: %1").arg(loadPath));
	COMOLEResourceInitializer comOLEResource;

	int ilImageId = -1;
	QString extension = fsManager->getFileExtension(loadPath);
	
	// load into a devil IL image
	TRY_SLEEP_LOCK(_ilMutex)
		int imgId = ilGenImage();
		ilBindImage(imgId);

		// graphics file formats supported by GDI+ are BMP, GIF, JPEG, PNG, TIFF, Exif, WMF, and EMF
		// try let GDI load first, if it failed, try DevIL next.
		unsigned int width = 0, height = 0, format = 0, size = 0; //return values from LoadPicture
		unsigned char * data = NULL; //LoadPicture will allocate, delete after ilTexImage
		
		if (extension != ".dds" && LoadPicture((const wchar_t *)loadPath.utf16(), width, height, format, size, data))
		{
			//LoadPicture returns false if pixels are not 32bpp since IL and GDI have different non 32bpp formats
			_ASSERT(Gdiplus::GetPixelFormatSize(format) == 32); 
			ILenum ilFormat = IL_BGRA;
			if (!(format & PixelFormatAlpha))
				ilFormat = IL_BGR;
			ilTexImage(width, height, 1, 4, ilFormat, IL_UNSIGNED_BYTE, (void *)data); //all 32bpp GDI formats are (A)RGB
			delete [] data;
			IL_ASSERT_NO_ERROR();
			ilImageId = imgId;
		}
		else if (ilLoadImage((ILconst_string)loadPath.utf16()))
		{
			ilImageId = imgId;
		}
		else
		{
			ILenum errorCode = ilGetError(); // This one is reached frequently due to missing pictures or bad pictures
			while (IL_INVALID_FILE_HEADER == errorCode || IL_FILE_READ_ERROR == errorCode || IL_COULD_NOT_OPEN_FILE == errorCode)
			{
				wprintf(L"GLTextureManager::loadHiResImage failed, code=0x%.4X, path=%s \n", errorCode, loadPath.utf16());
				errorCode = ilGetError(); // IL_INVALID_FILE_HEADER is followed by many IL_FILE_READ_ERROR
			}
			ilDeleteImage(imgId);

			errorCode = ilGetError();
			_ASSERT(IL_NO_ERROR == errorCode || IL_COULD_NOT_OPEN_FILE == errorCode);
			
			_ilMutex.unlock();
			return -1;
		}
				
		IL_ASSERT_NO_ERROR();
		int numBytes = ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL) * 
			ilGetInteger(IL_IMAGE_WIDTH) * 
			ilGetInteger(IL_IMAGE_HEIGHT);
		IL_ASSERT_NO_ERROR();

		// skip certain operations if this was a compressed dds file
		int compressionFormat = ilGetInteger(IL_DXTC_DATA_FORMAT);
		IL_ASSERT_NO_ERROR();

		if (
#ifdef DXRENDER
			false && // TODO DXR check DXT format support
#else
			GLEW_EXT_texture_compression_s3tc &&
#endif
			extension == ".dds" &&
			compressionFormat != IL_DXT_NO_COMP)
		{
			// flip the image
			iluFlipImage();
		}
		else
		{
			// scale it down to the dimensions of the monitor if it's larger
			int width = ilGetInteger(IL_IMAGE_WIDTH);
			int height = ilGetInteger(IL_IMAGE_HEIGHT);
			IL_ASSERT_NO_ERROR();
			int monitorWidth = nextPowerOfTwo(winOS->GetMonitorWidth());
			if (width - monitorWidth > 64)
			{
				int newWidth = min(width, monitorWidth);
				int newHeight = ((float) monitorWidth / width) * height;
				iluScale(newWidth, newHeight, ilGetInteger(IL_IMAGE_DEPTH));
			}
			IL_ASSERT_NO_ERROR();

			// convert the image to bgra if necessary
			int format = ilGetInteger(IL_IMAGE_FORMAT);
			switch (format)
			{
			case IL_RGBA:
				ilConvertImage(IL_BGRA, IL_UNSIGNED_BYTE);
				break;
			case IL_RGB:
				ilConvertImage(IL_BGR, IL_UNSIGNED_BYTE);
				break;
			case IL_BGRA:
			case IL_BGR:
				break;
			default:
				ilConvertImage(IL_BGRA, IL_UNSIGNED_BYTE);
				break;
			}
			IL_ASSERT_NO_ERROR();


			/*
			// XXX: scale it down to the closest power of two
			if (imgData->width != imgData->height ||
				!isPowerOfTwo(imgData->width))
			{
				int maxDims = max(imgData->width, imgData->height);
				int newSquareSize = min(closestPowerOfTwo(maxDims),
					(unsigned int) GLOBAL(settings).maxTextureSize);
				if (newSquareSize > maxDims)
				{
					// scaling up, choose appropriate algorithm
					iluImageParameter(ILU_FILTER, ILU_BILINEAR);
				}
				else
				{
					// scaling down, choose appropriate algorithm
					iluImageParameter(ILU_FILTER, ILU_SCALE_TRIANGLE);
				}
				iluScale(newSquareSize, newSquareSize, ilGetInteger(IL_IMAGE_DEPTH));
			}
			*/

			// flip windows bitmaps since they are defined differently
			if (extension == ".bmp" || extension == ".ico" || extension == ".cur")
				iluRotate(-180.0f);
			else
			 	iluFlipImage();
			IL_ASSERT_NO_ERROR();
		}
	_ilMutex.unlock();
	IL_ASSERT_NO_ERROR();
	return ilImageId;
}

void GLTextureManager::scaleToPowerOfTwo()
{
	unsigned int width = ilGetInteger(IL_IMAGE_WIDTH);
	unsigned int height = ilGetInteger(IL_IMAGE_HEIGHT);
	unsigned int newWidth = width;
	unsigned int newHeight = height;

	if (!isPowerOfTwo(width))
		newWidth = min(closestPowerOfTwo(width), (unsigned int)GLOBAL(settings).maxTextureSize);
	if (!isPowerOfTwo(height))
		newHeight = min(closestPowerOfTwo(height), (unsigned int)GLOBAL(settings).maxTextureSize);

	if (newWidth != width || newHeight != height)
	{
		iluImageParameter(ILU_FILTER, ILU_BILINEAR);
		iluScale(newWidth, newHeight, ilGetInteger(IL_IMAGE_DEPTH));
	}
}

bool GLTextureManager::loadBlockingTexture(GLTextureObject& obj, bool loadMipMaps, bool freeImageData)
{
	IL_ASSERT_NO_ERROR();
	if (_suspendLoading)
		return false;
	
	// ConsoleWriteGuard guard("loadImageThread");
	// load the image into memory
	int imgId = loadImageThread(obj);

	if (imgId > -1)
	{
		// ConsoleWriteGuard guard("loadBlockingTexture");
		// ensure it's not already in the texture list
		if (_textures.find(obj.key) != _textures.end())
		{
			IL_ASSERT_NO_ERROR();
			if (obj.operation & Reload)
				deleteTexture(obj.key);
			else
				return false;
		}

		// update the image information
		_ilMutex.lock();		
			// save the current file mod time so that we can check against this in the future
			if ( (imgId > -1) && exists(obj.srcPath) )
				obj.srcPathFileSize = fsManager->getFileSize(obj.srcPath);

			{
				// ConsoleWriteGuard guard("updateTextureWithILImageId");
				updateTextureWithILImageId(obj, imgId);
				obj.state = TextureQueued;
			}

			{
				// ConsoleWriteGuard guard("ilutGLBindMipmaps");
				// load the texture into video memory
				if (obj.resultCode == NoError)
				{
					ilBindImage(obj.resultTextureData->ilImageId);
					// XXX: check if this fails and set the result code/state accordingly
#ifdef DXRENDER
					ilConvertImage(IL_BGRA, IL_UNSIGNED_BYTE);
					unsigned int width = ilGetInteger(IL_IMAGE_WIDTH), height = ilGetInteger(IL_IMAGE_HEIGHT);
					bool flipImage = ilGetInteger(IL_IMAGE_ORIGIN) != IL_ORIGIN_LOWER_LEFT;

					/*
					// If the texture requires a border to be rendered, do so using Qt
					if (obj.hasBorder)
					{
						QImage photoImage(ilGetData(), width, height, QImage::Format_ARGB32_Premultiplied);
						applyFrameBorder(photoImage, frameThicknessPercent, frameBorderColor);
						ilTexImage(photoImage.width(), photoImage.height(), ilGetInteger(IL_IMAGE_DEPTH), 4, IL_BGRA, IL_UNSIGNED_BYTE, photoImage.bits());
					}*/	
					
					scaleToPowerOfTwo();

					width = ilGetInteger(IL_IMAGE_WIDTH);
					height = ilGetInteger(IL_IMAGE_HEIGHT);
					obj.resultTextureData->glTextureId = dxr->createTextureFromData(width, height, ilGetData(), width * 4, flipImage);
					obj.resultTextureData->pow2Dimensions = Vec3((float)width, (float)height, 0.0f);
#else
					if (loadMipMaps)
						obj.resultTextureData->glTextureId = ilutGLBindMipmaps();
					else
						obj.resultTextureData->glTextureId = ilutGLBindTexImage();
#endif

					// mark the state as loaded
					obj.state = TextureLoaded;
					obj.resultCode = NoError;

#ifndef DXRENDER
					// set the open gl texture params
					if (loadMipMaps)
						initGLParams();
					else
					{
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					}
#endif
				}
			}
		_ilMutex.unlock();

		// free the image memory
		if (freeImageData)
			obj.resultTextureData->freeImage();

		// manually insert it into the textures list
		_textures.insert(obj.key, obj);

		IL_ASSERT_NO_ERROR();
		return true;
	}
	IL_ASSERT_NO_ERROR();
	return false;
}

void GLTextureManager::applyFrameBorder(QImage& image, float thicknessPercent, QColor frameColor)
{
	// alphaBuffer is a constant that specifies how many pixels from the edge of the image
	// should be alpha filled and not rendered to
	const unsigned int alphaBuffer = 2;
	unsigned int width = image.width();
	unsigned int height = image.height();
	
	// These values determine the size of the frame and gradient, depending on the size of the image
	unsigned int frameWidth = (unsigned int)NxMath::ceil(NxMath::min(width, height) * thicknessPercent);
	unsigned int gradientSize = (unsigned int)NxMath::floor(NxMath::min(width, height) * thicknessPercent / 4.0f);
	
	// Use pre-multiplied because Qt says it is faster
	QImage borderedImage(width, height, QImage::Format_ARGB32_Premultiplied);
	
	// Fill the image with transparent black
	borderedImage.fill(0);
	
	// Create the painter which we will use to render our frame
	QPainter borderPainter(&borderedImage);
	borderPainter.setPen(Qt::NoPen);
	
	// Fill the entire image (minus a bit around the edges so we blend well) with the frame colour. 
	// We will render the gradients and original image on top of this
	borderPainter.fillRect(alphaBuffer, alphaBuffer, width - (alphaBuffer * 2), height - (alphaBuffer * 2), QBrush(frameColor));
	
	// Draw the original image scaled and translated to the middle of the image. make sure
	// the scaling is smooth
	borderPainter.setRenderHint(QPainter::SmoothPixmapTransform);
	QRect targetRect(frameWidth, frameWidth, width - (frameWidth * 2), height - (frameWidth * 2));
	borderPainter.drawImage(targetRect, image, image.rect());
	borderPainter.setRenderHint(QPainter::SmoothPixmapTransform, false);
	
	// Set the colors of the gradient
	QRect borderInset;
	QLinearGradient gradient;
	gradient.setColorAt(0, QColor(0, 0, 0, 120));
	gradient.setColorAt(1, QColor(0, 0, 0, 0));

	// Set up the left border gradient and draw it
	gradient.setStart(frameWidth, 0);
	gradient.setFinalStop(frameWidth + gradientSize, 0);
	borderInset.setRect(frameWidth, frameWidth, gradientSize, height - (frameWidth * 2));
	borderPainter.fillRect(borderInset, QBrush(gradient));
	
	// Set up the right border gradient and draw it
	gradient.setStart(width - frameWidth, 0);
	gradient.setFinalStop(width - frameWidth - gradientSize, 0);
	borderInset.setRect(width - frameWidth - gradientSize, frameWidth, gradientSize, height - (frameWidth * 2));
	borderPainter.fillRect(borderInset, QBrush(gradient));
					
	// Set up the top border gradient and draw it
	gradient.setStart(0, height - frameWidth);
	gradient.setFinalStop(0, height - frameWidth - gradientSize);
	borderInset.setRect(frameWidth, height - frameWidth - gradientSize, width - (frameWidth * 2), gradientSize);
	borderPainter.fillRect(borderInset, QBrush(gradient));
	
	// Set up the bottom border gradient and draw it
	gradient.setStart(0, frameWidth);
	gradient.setFinalStop(0, frameWidth + gradientSize);
	borderInset.setRect(frameWidth, frameWidth, width - (frameWidth * 2), gradientSize);
	borderPainter.fillRect(borderInset, QBrush(gradient));
	
	// Finalize rendering and override the image passed into this function
	borderPainter.end();
	image = borderedImage;
}

bool GLTextureManager::loadQueuedTexture(GLTextureObject& objectOut)
{
	// ConsoleWriteGuard guard("loadQueuedTexture");

	if (_suspendLoading)
		return false;

	if (!_texturesQueued.empty())
	{
		// get the next texture to load into video memory
		QString idx = _texturesQueued.top().key;
		assert(_textures.find(idx) != _textures.end());
		GLTextureObject& obj = _textures[idx];
		// ConsoleWriteGuard guard(QString("Load Queued Texture: %1, %2").arg(obj.key).arg(obj.resultCode));

		// check the state of the image
		if (obj.resultCode == NoError)
		{
			assert(obj.state == TextureQueued);
			assert(obj.resultTextureData);
			assert(obj.resultTextureData->ilImageId);
			assert(!obj.resultTextureData->glTextureId);
			
			// try and load the texture
			if (_ilMutex.tryLock())
			{
				shared_ptr<GLTextureData> imgData = obj.resultTextureData;
				ilBindImage(imgData->ilImageId);	
#ifdef DXRENDER
				ilConvertImage(IL_BGRA, IL_UNSIGNED_BYTE);
				
				unsigned int width = ilGetInteger(IL_IMAGE_WIDTH), height = ilGetInteger(IL_IMAGE_HEIGHT);
				bool flipImage = ilGetInteger(IL_IMAGE_ORIGIN) != IL_ORIGIN_LOWER_LEFT;

				/*
				// If the texture requires a border to be rendered, so so using Qt		
				if (obj.hasBorder)
				{
					QImage photoImage(ilGetData(), width, height, QImage::Format_ARGB32_Premultiplied);
					applyFrameBorder(photoImage, frameThicknessPercent, frameBorderColor);
					ilTexImage(photoImage.width(), photoImage.height(), ilGetInteger(IL_IMAGE_DEPTH), 4, IL_BGRA, IL_UNSIGNED_BYTE, photoImage.bits());
				}*/	

				scaleToPowerOfTwo();
				
				width = ilGetInteger(IL_IMAGE_WIDTH);
				height = ilGetInteger(IL_IMAGE_HEIGHT);
				imgData->glTextureId = dxr->createTextureFromData(width, height, ilGetData(), 4 * width, flipImage);
				imgData->pow2Dimensions = Vec3((float)width, (float)height, 0.0f);
#else
				// XXX: check if this fails and set the result code/state accordingly
				imgData->glTextureId = ilutGLBindMipmaps();
#endif
				// If it was requested, serialize the texture to an image file in the cache directory
				// This is used by the sharing widget, which needs an image file to reference from HTML.
				if (obj.operation & Serialize)
				{
					QString cacheDir = native(_cacheDirectory);
					ensurePathSlash(cacheDir);
					QString cacheFilePath = cacheDir + generateCacheFilePath(obj.srcPath, obj.imageDimensions, "png");
					ilSaveImage((ILconst_string)cacheFilePath.utf16());
					ILenum err = ilGetError();
					if (err == IL_NO_ERROR)
					{
//						obj.serializedPath = cacheFilePath;
						consoleWrite(QString("Successfully serialized texture for %1\n").arg(obj.srcPath));
					}
					else
					{
						consoleWrite(QString("Failed to serialize texture for %1, err = %2\n").arg(obj.srcPath).arg(err));
					}
				}

				_ilMutex.unlock();

				// XXX: handle thrashing here!

				// clear the image data from system memory after loading the texture
				// into video memory
				// NOTE: this call also locks the IL mutex, so don't do it in the 
				//		 same scope as above
				imgData->freeImage();

				// mark the state as loaded
				obj.state = TextureLoaded;
				obj.resultCode = NoError;

				// copy the texture information out
				objectOut = obj;

#ifndef DXRENDER
				// set the open gl texture params
				initGLParams();
#endif

				_texturesQueued.pop();

				return true;
			}
		}
		else
		{
			// just pop the item off the queue without loading
			_texturesQueued.pop();
		}
	}
	else
	{
		// dismiss the "restoring state" message from earlier once
		// all the items are loaded after restoring from sleep
		QString message("GLTextureManager::onPowerResume");
		if (!scnManager->messages()->getMessage(message) || !scnManager->messages()->getMessage(message)->isAnimating())
			dismiss(message);
	}
	return false;
}

bool GLTextureManager::loadPersistentTexture(GLTextureObject& obj)
{
	obj.persistent = true;
	return loadTexture(obj);
}

bool GLTextureManager::loadTexture(GLTextureObject& obj)
{
	// workaround for the lack of validation in RGM
	assert(!obj.srcPath.isEmpty());

	if((obj.isAFile)&&(!exists(obj.srcPath))) {
		return false;
	}

	// ensure it's not already in the texture list
	if (_textures.find(obj.key) != _textures.end())
	{
		IL_ASSERT_NO_ERROR();
	
		// check the last modified date to see if we actually need to load this file
		int curTexSrcPathSize = _textures[obj.key].srcPathFileSize;
		if ( exists(obj.srcPath) && (curTexSrcPathSize > 0) && (curTexSrcPathSize == fsManager->getFileSize(obj.srcPath)) )
			return true;

		if (obj.operation & Reload)
			deleteTexture(obj.key);
		else
			return false;
	}

	// XXX: skip if the image data points to something already

	// add it to the texture list and associated structures
	obj.resultCode = NoError;
	obj.state = ImageQueued;
	_textures.insert(obj.key, obj);
	_imagesQueued.push(TexturesListIndex(&_textures, obj.key));
	IL_ASSERT_NO_ERROR();
	return true;
}

bool GLTextureManager::postTexture( GLTextureObject& obj )
{
	// ConsoleWriteGuard guard("postTexture");
	IL_ASSERT_NO_ERROR();
	if (_suspendLoading)
		return true;

	_postedTexturesMutex.lock();
		_postedTextures.insert(obj.key, obj);
	_postedTexturesMutex.unlock();
	return true;
}

bool GLTextureManager::hasTexture( QString key )
{
	return _textures.contains(key);
}

void GLTextureManager::deleteTexture( QString key )
{
	// ConsoleWriteGuard guard("deleteTexture");

	// assert(_textures.find(key) != _textures.end());

	// skip if it's not yet loaded
	if (_textures.find(key) == _textures.end())
		return;

	// remove it from the queues
	// ConsoleWriteGuard g1("removeFromPrioritizedTextureIndices");
	if (_textures[key].state == ImageQueued)
		removeFromPrioritizedTextureIndices(_imagesQueued, key);
	if (_textures[key].state == TextureQueued)
		removeFromPrioritizedTextureIndices(_texturesQueued, key);

	// check if any of the threads are currently loading the key,
	//		and if so, move the thread to the invalidatedLoadingThreads
	{
		// ConsoleWriteGuard g1("invalidate thread");
		TextureThreads::iterator iter = _loadingThreads.begin();
		while (iter != _loadingThreads.end())
		{
			if ((*iter)->getParam().key == key)
			{
				assert(*iter);
				_invalidatedLoadingThreads.push_back(*iter);
				iter = _loadingThreads.erase(iter);
			}
			else
				iter++;
		}
	}


	// remove it from the texture list
	{
		// ConsoleWriteGuard g1("remove from texture list");
		const GLTextureObject& obj = _textures[key];
		if (obj.resultTextureData)
		{
			obj.resultTextureData->freeTexture();
		}
		_textures.remove(key);
	}
}

void GLTextureManager::removeFromPrioritizedTextureIndices( PrioritizedTextureIndices& queue, QString key )
{
	// sanity check
	if (queue.empty())
		return;

	// do a quick check for the top item
	if (queue.top().key == key)
	{
		queue.pop();
		return;
	}

	// filter the pqueue for the item(s) matching the key
	PrioritizedTextureIndices tmpQueue;
	while (!queue.empty())
	{
		const TexturesListIndex& idx = queue.top();
		if (idx.key != key)
			tmpQueue.push(idx);
		queue.pop();
	}
	queue = tmpQueue;
}

void GLTextureManager::updateTextureWithILImageId(GLTextureObject& obj, int imgId)
{
	if (imgId < 0)
	{
		// mark the image loading as failed, but still queue it
		// for callback handling, etc.
		obj.resultCode = ImageLoadError;
	}
	else
	{
		// resolve some of the image information from the il image
		GLTextureData * imgData = new GLTextureData;
		imgData->ilImageId = imgId;
		ilBindImage(imgId);
		IL_ASSERT_NO_ERROR();
		int format = ilGetInteger(IL_IMAGE_FORMAT);
		imgData->alpha = (format == IL_BGRA || format == IL_RGBA);
		imgData->dimensions = Vec3((float) ilGetInteger(IL_IMAGE_WIDTH),
			(float) ilGetInteger(IL_IMAGE_HEIGHT), 0);
		obj.resultTextureData.reset(imgData);
		IL_ASSERT_NO_ERROR();
	}
}

bool GLTextureManager::hasWinThumbnail( QString path )
{
	/* TODO: reimplement BT_ThumbsDBReader
	QString lowerPath = native(parent(path)).toLower();
	QString fileName = QFileInfo(path).fileName();

	// get the set of thumbnail files for that directory
	bool found = false;
	_thumbsDbsMutex.lock();
		ThumbsDbs::const_iterator iter = _thumbsDbs.find(lowerPath);
		if (iter != _thumbsDbs.end())
		{
			// and check if this file is one of those thumbnailed
			shared_ptr<ThumbsDBReader> reader = iter.value();
			vector<LPCWSTR> files = reader->getThumbFilenames();
			for (int i = 0; i < files.size(); ++i)
			{
				if (QString::fromUtf16((const ushort *) files[i]) == fileName)
					found = true;
			}
		}
	_thumbsDbsMutex.unlock();
	return found;
	*/
	return false;
}

void GLTextureManager::appendWinThumbnailSearchPath( QString path )
{
	/* TODO: reimplement BT_ThumbsDBReader
	// Automated testing currently has problems deleting thumbs.db files
	if(scnManager->runAutomatedJSONTestsOnStartup)
		return;
	_thumbsDbsMutex.lock();
		QString lowerPath = path.toLower();
		_thumbsDbs.remove(lowerPath);
		_thumbsDbs.insert(lowerPath, shared_ptr<ThumbsDBReader>(new ThumbsDBReader(lowerPath)));
	_thumbsDbsMutex.unlock();
	*/
}

void GLTextureManager::removeWinThumbnailSearchPath( QString path )
{
	/* TODO: reimplement BT_ThumbsDBReader
	_thumbsDbsMutex.lock();
		QString lowerPath = path.toLower();
		_thumbsDbs.remove(lowerPath);
	_thumbsDbsMutex.unlock();
	*/
}

void GLTextureManager::lockIlMutex()
{
	_ilMutex.lock();
}

void GLTextureManager::unlockIlMutex()
{
	_ilMutex.unlock();
}

void GLTextureManager::onThemeChanged()
{
	winOS->LoadSettingsFile();

	// reload all the necessary textures
	QDir texPath = winOS->GetUserThemesDirectory(true)
		/ themeManager->getValueAsQString(QT_NT("textures.relativeRoot"),QT_NT(""));
	QDir iconRelPath = texPath / themeManager->getValueAsQString(QT_NT("textures.icon.relativeRoot"),QT_NT(""));
	
	QString iconRelPathStr = native(iconRelPath).append('\\');
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("icon.linkOverlay"), iconRelPathStr + themeManager->getValueAsQString(QT_NT("textures.icon.linkOverlay"),QT_NT("")), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("icon.selection.alphaMask"), iconRelPathStr + themeManager->getValueAsQString(QT_NT("textures.icon.selection.alphaMask"),QT_NT("")), HiResImage, HighPriority));

	QString iconCustomRelPathStr = native(iconRelPath / themeManager->getValueAsQString(QT_NT("textures.icon.custom.relativeRoot"),QT_NT(""))).append('\\');
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("icon.custom.printer"), iconCustomRelPathStr + themeManager->getValueAsQString(QT_NT("textures.icon.custom.printer"),QT_NT("")), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("icon.custom.email"), iconCustomRelPathStr + themeManager->getValueAsQString(QT_NT("textures.icon.custom.email"),QT_NT("")), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("icon.custom.stickyNote"), iconCustomRelPathStr + themeManager->getValueAsQString(QT_NT("textures.icon.custom.stickyNote"),QT_NT("")), HiResImage, HighPriority));

	QString pileRelPathStr = native(texPath / themeManager->getValueAsQString(QT_NT("textures.pile.relativeRoot"),QT_NT(""))).append('\\');
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("pile.background"), pileRelPathStr + themeManager->getValueAsQString(QT_NT("textures.pile.background"),QT_NT("")), HiResImage, NormalPriority));

	// we only want to compress the background and such if we are in low quality texture mode
	unsigned int operationOverride = Load|Reload;
	if (GLOBAL(settings).visuals == LowVisuals)
		operationOverride |= Compress;
	QString floorRelPathStr = native(texPath / themeManager->getValueAsQString(QT_NT("textures.floor.relativeRoot"),QT_NT(""))).append('\\');
	if (GLOBAL(settings).useWindowsBackgroundImage)
	{
		winOS->LoadDesktopTexture();
	}
	else
	{
		// use the background if specified
		loadPersistentTexture(GLTextureObject(operationOverride, QT_NT("floor.desktop"), floorRelPathStr + themeManager->getValueAsQString(QT_NT("textures.floor.desktop"),QT_NT("")), HiResImage, ImmediatePriority));
	}
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("floor.infinite"), floorRelPathStr + themeManager->getValueAsQString(QT_NT("textures.floor.infinite"),QT_NT("")), HiResImage, IdlePriority));

	QString wallRelPathStr = native(texPath / themeManager->getValueAsQString(QT_NT("textures.wall.relativeRoot"),QT_NT(""))).append('\\');
	if (!themeManager->getValueAsQString(QT_NT("textures.wall.top"),QT_NT("")).isEmpty())
		loadPersistentTexture(GLTextureObject(operationOverride, QT_NT("wall.top"), wallRelPathStr + themeManager->getValueAsQString(QT_NT("textures.wall.top"),QT_NT("")), HiResImage, ImmediatePriority));
	else deleteTexture(QT_NT("wall.top"));
	if (!themeManager->getValueAsQString(QT_NT("textures.wall.right"),QT_NT("")).isEmpty())
		loadPersistentTexture(GLTextureObject(operationOverride, QT_NT("wall.right"), wallRelPathStr + themeManager->getValueAsQString(QT_NT("textures.wall.right"),QT_NT("")), HiResImage, ImmediatePriority));
	else deleteTexture(QT_NT("wall.right"));
	if (!themeManager->getValueAsQString(QT_NT("textures.wall.bottom"),QT_NT("")).isEmpty())
		loadPersistentTexture(GLTextureObject(operationOverride, QT_NT("wall.bottom"), wallRelPathStr + themeManager->getValueAsQString(QT_NT("textures.wall.bottom"),QT_NT("")), HiResImage, ImmediatePriority));
	else deleteTexture(QT_NT("wall.bottom"));
	if (!themeManager->getValueAsQString(QT_NT("textures.wall.left"),QT_NT("")).isEmpty())
		loadPersistentTexture(GLTextureObject(operationOverride, QT_NT("wall.left"), wallRelPathStr + themeManager->getValueAsQString(QT_NT("textures.wall.left"),QT_NT("")), HiResImage, ImmediatePriority));
	else deleteTexture(QT_NT("wall.left"));

	QString widgetRelPathStr = native(texPath / themeManager->getValueAsQString(QT_NT("textures.widget.relativeRoot"),QT_NT(""))).append('\\');
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("widget.close"), widgetRelPathStr + themeManager->getValueAsQString(QT_NT("textures.widget.close"),QT_NT("")), HiResImage, HighPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("widget.launchInExplorer"), widgetRelPathStr + themeManager->getValueAsQString(QT_NT("textures.widget.launchInExplorer"),QT_NT("")), HiResImage, NormalPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("widget.scroll.down"), widgetRelPathStr + themeManager->getValueAsQString(QT_NT("textures.widget.scroll.down"),QT_NT("")), HiResImage, NormalPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("widget.scroll.up"), widgetRelPathStr + themeManager->getValueAsQString(QT_NT("textures.widget.scroll.up"),QT_NT("")), HiResImage, NormalPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("widget.scroll.down_disabled"), widgetRelPathStr + themeManager->getValueAsQString(QT_NT("textures.widget.scroll.down_disabled"),QT_NT("")), HiResImage, NormalPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("widget.scroll.up_disabled"), widgetRelPathStr + themeManager->getValueAsQString(QT_NT("textures.widget.scroll.up_disabled"),QT_NT("")), HiResImage, NormalPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("widget.vertStack"), widgetRelPathStr + themeManager->getValueAsQString(QT_NT("textures.widget.lassoMenu.dragAndCross"),QT_NT("")), HiResImage, NormalPriority));

	QString overrideRelPathStr = native(texPath / themeManager->getValueAsQString(QT_NT("textures.override.relativeRoot"),QT_NT(""))).append('\\');
	if (GLOBAL(settings).useThemeIconOverrides)
	{
		loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("override.virtual.mycomputer"), overrideRelPathStr + themeManager->getValueAsQString(QT_NT("textures.override.virtual.myComputer"),QT_NT("")), HiResImage, HighPriority));
		loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("override.ext.folder"), overrideRelPathStr + themeManager->getValueAsQString(QT_NT("textures.override.ext.folder"),QT_NT("")), HiResImage, HighPriority));

		// handle arbitrary extension overrides
		std::vector<std::string> overrides = themeManager->getValue(QT_NT("textures.override.ext")).getMemberNames();
		for (unsigned int i = 0; i < overrides.size(); ++i)
		{
			QString extensionTexKey = QString::fromStdString(QT_NT("override.ext.") + overrides[i]);
			loadPersistentTexture(GLTextureObject(Load|Compress|Reload, extensionTexKey, overrideRelPathStr + themeManager->getValueAsQString(QT_NT("textures.") + extensionTexKey,QT_NT("")), HiResImage, HighPriority));
		}
	}
	QString slideshowRelPathStr = native(texPath / themeManager->getValueAsQString(QT_NT("textures.slideshow.relativeRoot"),QT_NT(""))).append('\\');
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("slideshow.next"), slideshowRelPathStr + themeManager->getValueAsQString(QT_NT("textures.slideshow.next"),QT_NT("")), HiResImage, LowPriority));
	loadPersistentTexture(GLTextureObject(Load|Compress|Reload, QT_NT("slideshow.previous"), slideshowRelPathStr + themeManager->getValueAsQString(QT_NT("textures.slideshow.previous"),QT_NT("")), HiResImage, LowPriority));
}

void GLTextureManager::onTimer()
{
	if (_suspendLoading)
		return;
	// ConsoleWriteGuard guard("onTimer");
	
	// The texture loading threads should only be alive for 2 seconds
	int maxRunTime = 2000;

	// update each of the thumbs dbs
	/* TODO: reimplement BT_ThumbsDBReader
	_thumbsDbsMutex.lock();
		ThumbsDbs::const_iterator iter = _thumbsDbs.begin();
		while (iter != _thumbsDbs.end())
		{
			iter.value()->onTimer();
			iter++;
		}
	_thumbsDbsMutex.unlock();
	*/

	// go through the posted textures list and add them to the normal list
	if (_postedTexturesMutex.tryLock())
	{
		TexturesList::iterator iter = _postedTextures.begin();
		while (iter != _postedTextures.end())
		{
			loadTexture(iter.value());
			iter++;
		}
		_postedTextures.clear();
		_postedTexturesMutex.unlock();
	}

	// go through the list of queued images and start loading on them
	if (!_imagesQueued.empty())
	{
		if (_loadingThreads.size() < _maxNumLoadingThreads)
		{
			// start a new thread with the next image image to be loaded
			GLTextureObject& obj = _textures[_imagesQueued.top().key];
			_imagesQueued.pop();

			// mark the item as loading
			assert(obj.state == ImageQueued);
			assert(obj.resultCode == NoError);
			obj.state = ImageLoading;

			
			// NOTE: if texture data is already loaded, then we should reset it
			if (obj.resultTextureData)
			{
				obj.resultTextureData->freeTexture();
				obj.resultTextureData->freeImage();
				obj.resultTextureData.reset();
			}

			// ConsoleWriteGuard guard(QString("Queuing Texture: %1").arg(obj.key));

			// start the loading thread on that image
			ThreadableTextureUnit * unit = NULL;
			QFileInfo fileInfo(obj.srcPath);

			obj.texhelperId = _texHelperManager->getPipe();
			// there might be _maxNumLoadingThreads threads using texhelper, 
			// since _loadingThreads can be erased but the real thread is not killed due to boost limitation
			// so 0xFFFFFFFF texhelperId signals createTexHelperProcess function to create a new process instead of using existing pipes
			if (0xFFFFFFFF != obj.texhelperId) 
			{
				_ASSERT(_texHelperManager->isPipeBusy(obj.texhelperId));
			}

			unit = new ThreadableTextureUnit(obj);
			unit->run(boost::bind(&GLTextureManager::loadImageThread, this, _1), ThreadableUnit::GLTextureManagerTexhelperThread);
			_loadingThreads.push_back(unit);
		}
	}

	// free images in the invalidated threads
	// NOTE: we defer this until there are no more items in the loading threads,
	//		 but we must call this before the next block since that may clear
	//		 the set of loading threads
	if (!_invalidatedLoadingThreads.empty() && _loadingThreads.empty())
	{
		TextureThreads::iterator iter = _invalidatedLoadingThreads.begin();
		while (iter != _invalidatedLoadingThreads.end())
		{
			assert(*iter);
			int imgId = (*iter)->getResult();
			if (imgId >= 0)
			{
				if (_ilMutex.tryLock())
				{
					ilDeleteImage(imgId);
					_ilMutex.unlock();
				}
			}
			delete (*iter);
			iter = _invalidatedLoadingThreads.erase(iter);
		}
	}

	// update the texture objects of the finished loading threads
	if (!_loadingThreads.empty())
	{
		TextureThreads::iterator iter = _loadingThreads.begin();
		while (iter != _loadingThreads.end())
		{
			if ((*iter)->getState() == ThreadableTextureUnit::Complete)
			{
				GLTextureObject& obj = _textures[(*iter)->getParam().key];
				assert(obj.state == ImageLoading);
				assert(obj.resultCode == NoError);
				assert(!obj.resultTextureData);

				if (_ilMutex.tryLock())
				{
					int imgId = (*iter)->getResult();
					updateTextureWithILImageId(obj, imgId);
					_ilMutex.unlock();

					// save the current file mod time so that we can check against this in the future
					if ( (imgId > -1) && exists(obj.srcPath) )
						obj.srcPathFileSize = fsManager->getFileSize(obj.srcPath);

					// mark the texture to be loaded
					obj.state = TextureQueued;
					_texturesQueued.push(TexturesListIndex(&_textures, obj.key));

					// remove the thread from the list.
					// the actual thread is not killed, so the texhelper process may still be used.
					// the texhelper pipe is freed in loadImageThread when it finishes.
					iter = _loadingThreads.erase(iter);
				}
				else
					iter++;
			}
			else if ((*iter)->getState() == ThreadableTextureUnit::Expired)
			{
				iter = _loadingThreads.erase(iter);
			}
			else
				iter++;
		}
	}
}

bool GLTextureManager::isLoadingSuspended() const
{
	return _suspendLoading;
}

void GLTextureManager::onPowerSuspend()
{
	LOG(QString_NT("Start onPowerSuspend() - %1").arg(timeGetTime()));
	// suspend loading
	if (_suspendLoading)
	{
		LOG(QString_NT("\tAlready suspended"));
		return;
	}
	
	killLoadingThreads();

	// free all of the open gl textures, and mark each of them to be loaded
	// again
	TexturesList::iterator iter = _textures.begin();
	while (iter != _textures.end())
	{
		GLTextureObject& obj = iter.value();
		if (obj.state == TextureLoaded)
		{
			assert(obj.resultTextureData);
			if (obj.resultCode == NoError)
			{
				obj.resultTextureData->freeTexture();
				obj.resultTextureData->freeImage();
				obj.resultTextureData.reset();
				// Fix for if the power suspend is re-entered before the textures are reloaded.
				obj.state = UnknownState;
				assert(!obj.resultTextureData);
			}
		}
		iter++;
	}

	SAFE_RELEASE(_defaultGLTextureId);

	// clear the loading/textures queue
	while (!_texturesQueued.empty())
		_texturesQueued.pop();

	_suspendLoading = true;
	LOG(QString_NT("Finish onPowerSuspend()"));
}

void GLTextureManager::onPowerResume()
{
	// reload all the previously freed textures

	LOG(QString_NT("Start onPowerResume() - %1").arg(timeGetTime()));

	if (!_suspendLoading)
	{
		LOG(QString_NT("\tPower was not suspended"));
		return;
	}

	// NOTE: at this point, we know that there are no image threads running,
	//		 so it's not necessary to synchronize access to the data structs

	assert(_suspendLoading);
	assert(_texturesQueued.empty());
	assert(_loadingThreads.empty());

	while (!_imagesQueued.empty())
		_imagesQueued.pop();
	
	// mark each texture for loading
	TexturesList::iterator iter = _textures.begin();
	while (iter != _textures.end())
	{
		GLTextureObject& obj = iter.value();

		// add it to the texture list and associated structures
		// NOTE: we bypass loadTexture() since we don't want to delete again before reloading
		obj.srcPathFileSize = 0;
		obj.resultCode = NoError;
		obj.state = ImageQueued;
		_imagesQueued.push(TexturesListIndex(&_textures, obj.key));
		// ConsoleWriteGuard guard(QString("Re-queuing Texture: %1").arg(obj.key));
		iter++;
	}
	
	loadDefaultTexture();

	// resume loading
	_suspendLoading = false;
	LOG(QString_NT("Finish onPowerResume()"));
}

void GLTextureManager::setSuspendLoading(bool val)
{
	_suspendLoading = val;
}

IDirect3DTexture9* GLTextureManager::loadDefaultTexture()
{
	QFileInfo defaultImage = make_file(_textureDirectory, "square.png");
	QString defaultImagePath = native(defaultImage);
	HRESULT hr = D3DXCreateTextureFromFile(dxr->device, (LPCWSTR)defaultImagePath.utf16(), &_defaultGLTextureId);
	_ASSERT(SUCCEEDED(hr));
	return _defaultGLTextureId;
}

void GLTextureManager::takeScreenshot(QString filePathOut) const
{
	_ilMutex.lock();
		// NOTE: according to open gl, glReadPixels will also read the pixels of the overlapping windows
		// because in most implementations, they are mapped to the same frame buffer pixel space.
		while(ilGetError() != IL_NO_ERROR);
		ILuint screenshot = ilGenImage();
		ilBindImage(screenshot);
#ifdef DXRENDER
		IDirect3DSurface9* screenSurface = dxr->copyBackBuffer();
		if (screenSurface)
		{
			D3DLOCKED_RECT lockedRect;
			HRESULT hr = screenSurface->LockRect(&lockedRect, 0, D3DLOCK_READONLY);
			VASSERT(D3D_OK == hr, QString_NT("Could not lock screenshot surface: hr = %1").arg(hr));
			// screenSurface->Width and screenSurface->Height seem to be messed up, but
			// if we divide the pitch by the amount of bytes per pixel, we get the correct width
			// so the width and height are being reported wrong
			int channels = lockedRect.Pitch / dxr->getBackBufferWidth();
			ilTexImage(dxr->getBackBufferWidth(), dxr->getBackBufferHeight(), 1, (ILubyte)channels, channels == 4 ? IL_BGRA : IL_BGR, IL_UNSIGNED_BYTE, lockedRect.pBits);
			
			screenSurface->UnlockRect();
			SAFE_RELEASE(screenSurface);

			iluFlipImage();
			ilEnable(IL_FILE_OVERWRITE);
			ilSaveImage((ILconst_string) filePathOut.utf16());
		}
#else
		ilutGLScreen();
		ilEnable(IL_FILE_OVERWRITE);
		ilSaveImage((ILconst_string) filePathOut.utf16());
#endif
		ilDeleteImage(screenshot);
		IL_ASSERT_NO_ERROR();
	_ilMutex.unlock();
}

QString GLTextureManager::scaleFacebookImage( QString filePath )
{
	// Maximum Facebook restrictions on photo dimensions
	// http://www.facebook.com/topic.php?uid=2483740875&topic=8727
	// (it's actually 604px)
	int w = 600;
	int h = 600;

	_ilMutex.lock();
		Gdiplus::Image * fbPhoto = new Gdiplus::Image((wchar_t *)filePath.utf16());
		UINT width = fbPhoto->GetWidth();
		UINT height = fbPhoto->GetHeight();

		float aspect = (float) width / height;
		int newWidth = width;
		int newHeight = height;
		
		// Change dimensions if larger then facebook restrictions
		if (width > height)
		{
			// bound by max
			if (width > w)
			{
				newWidth = w;
				newHeight = (int) (w / aspect);
			}
			// bound by min
			if (newHeight > h)
			{
				newHeight = h;
				newWidth = (int) (h * aspect);
			}
		}
		else
		{
			// bound by max
			if (height > h)
			{
				newHeight = h;
				newWidth = (int) (h * aspect);
			}
			// bound by min
			if (newWidth > w)
			{
				newWidth = w;
				newHeight = (int) (w / aspect);
			}
		}
		
		QString saveAsJPGName = QString("%1.%2.jpg").arg(qrand()).arg(filename(filePath));
		QString saveAsJPGPath = native(make_file(_cacheDirectory, saveAsJPGName));
		if (newWidth != width ||
			newHeight != height)
		{
			Gdiplus::Bitmap resizedFBPhoto(newWidth, newHeight, PixelFormat32bppARGB);
			Gdiplus::Graphics graphics(&resizedFBPhoto);
			// Paint the background white for transparent photos
			Gdiplus::SolidBrush whiteBrush(Gdiplus::Color(255,255,255));
			graphics.FillRectangle(&whiteBrush, 0, 0, newWidth, newHeight);
			//Populate new bitmap with resized facebook photo
			graphics.DrawImage(fbPhoto, 0, 0, newWidth, newHeight);
			
			CLSID jpgClsid;
			GetEncoderClsid(L"image/jpeg", &jpgClsid);
			//Close the file stream so the picture can be overwritten
			delete fbPhoto;
			
			// resave the scaled jpeg into a temporary file
			qsrand(time(NULL));			
			resizedFBPhoto.Save((wchar_t *)saveAsJPGPath.utf16(), &jpgClsid);
		}
		else
		{
			QFile::copy(filePath, saveAsJPGPath);
		}
	_ilMutex.unlock();

	return saveAsJPGPath;
}

bool GLTextureManager::cropPhoto( QString srcPath, QString destPath, float top_crop_pct, float right_crop_pct, float bottom_crop_pct, float left_crop_pct )
{
	ILenum err;
	bool result = false;

	_ilMutex.lock();
	IL_ASSERT_NO_ERROR();
	ILuint ilImage = ilGenImage();
	ilBindImage(ilImage);
	ilLoadImage((ILconst_string)srcPath.utf16());
	err = ilGetError();
	if (err == IL_NO_ERROR)
	{
		int width = ilGetInteger(IL_IMAGE_WIDTH);
		int height = ilGetInteger(IL_IMAGE_HEIGHT);

		int x_skip = width * left_crop_pct;
		int x_keep = width - (left_crop_pct + right_crop_pct)*width;
		int y_skip = height * top_crop_pct;
		int y_keep = height - (top_crop_pct + bottom_crop_pct)*height;

		iluCrop(x_skip, y_skip, 1, x_keep, y_keep, 0);
		err = ilGetError();
		if (err == IL_NO_ERROR)
		{
			ilEnable(IL_FILE_OVERWRITE);
			result = ilSaveImage((ILconst_string)destPath.utf16());
			IL_ASSERT_NO_ERROR();
		}
	}

	ilDeleteImage(ilImage);
	IL_ASSERT_NO_ERROR();
	_ilMutex.unlock();
	return result;
}

int GLTextureManager::createTexHelperProcess(QString texHelperAppStr, QString commandLine, unsigned int texhelperId, int lifetime, TexHelperResponse * outResponse, void ** outData)
{
	if (GLOBAL(settings).disableLibSquish)
		commandLine += " -q\"\" ";
	
	if (0xFFFFFFFF == texhelperId) 
	{
		// Some TexHelper requests come from the main thread and is not bound by _maxNumLoadingThreads,
		// so create new TexHelper process for these requests.
		PROCESS_INFORMATION ProcessInfo = {0};
		STARTUPINFO StartupInfo = {0};
		StartupInfo.cb = sizeof(StartupInfo);
		if(CreateProcess((LPWSTR) texHelperAppStr.utf16(), (LPWSTR) commandLine.utf16(),
			NULL,NULL, FALSE, CREATE_NO_WINDOW | DETACHED_PROCESS,NULL,
			NULL, &StartupInfo, &ProcessInfo))
		{
			DWORD result = WaitForSingleObject(ProcessInfo.hProcess, lifetime);
			if (result == WAIT_TIMEOUT)
			{
				TerminateProcess(ProcessInfo.hProcess, 1);
				if (outResponse)
					*outResponse = TexHelperResponse(false, true, true);
				return -1;
			}
			else
				GetExitCodeProcess(ProcessInfo.hProcess, &result);
			CloseHandle(ProcessInfo.hThread);
			CloseHandle(ProcessInfo.hProcess);

			// if the texhelper has failed, then we should disable the texhelper and try again without it
			if (result > 0 && !GLOBAL(settings).disableLibSquish)
			{
				GLOBAL(settings).disableLibSquish = true;
				winOS->SaveSettingsFile();
				// NOTE: we must set the disableLibSquish before recursing or we may get inf loop!
				result = createTexHelperProcess(texHelperAppStr, commandLine, texhelperId, lifetime, outResponse, outData);
			}
			if (outResponse)
				*outResponse = TexHelperResponse(false, true, 0 != result);
			return result;
		}
		if (outResponse)
			*outResponse = TexHelperResponse(false, true, true);
		return -1;
	}
	
	if (_texHelperManager)
		return _texHelperManager->texHelperOperation(commandLine, texhelperId, lifetime, outResponse, outData);
	else 
		return -1;
}

QString GLTextureManager::ensureCompressedImage( const GLTextureObject& obj )
{
	QString loadPath = obj.srcPath;

	// create the thumbnail and the compress file path to create
	QString cacheDir = native(_cacheDirectory);
	ensurePathSlash(cacheDir);
	QDir().mkpath(cacheDir);

	QString compressionPath = generateCacheFilePath(obj.srcPath, obj.imageDimensions);

	// check if we need to actually launch the process
	QString compressFilePath = cacheDir + compressionPath;
	QDateTime fileModTime = QFileInfo(loadPath).lastModified();
	QFileInfo cfpInfo(compressFilePath);
	if (exists(cfpInfo))
	{
		if (cfpInfo.lastModified() >= fileModTime)
			return compressFilePath;
	}

	// run the process to make sure it's up to date
	QString texHelperAppStr = native(winOS->GetExecutableDirectory() / "TexHelper.exe");
	QString qpath = loadPath;
		ensureQuoted(qpath);
	QString qdir = cacheDir;
		ensureQuoted(qdir);
	QString qcmpPath = compressionPath;
		ensureQuoted(qcmpPath);

	QString qimageDims = QString("%1").arg(obj.imageDimensions);
	QString commandLine = QString("-f%1 -d%2 -c%3 -x%4 ")
		.arg(qpath).arg(qdir).arg(qcmpPath).arg(qimageDims);

	if (GLOBAL(settings).freeOrProLevel == AL_FREE)
		commandLine += " -e\"\" ";

	int result = createTexHelperProcess(texHelperAppStr, commandLine, obj.texhelperId, INFINITE, NULL, NULL);
	if ((result == 0) && exists(compressFilePath))
	{
		return compressFilePath;
	}
	return QString();
}


QString GLTextureManager::ensureThumbnail( const GLTextureObject& obj, bool compressed )
{
	QString loadPath = obj.srcPath;

	// create the thumbnail without compression
	QString cacheDir = native(_cacheDirectory);
	ensurePathSlash(cacheDir);
	QDir().mkpath(cacheDir);
	QString loadPathHash = QCryptographicHash::hash(loadPath.toUtf8(), QCryptographicHash::Md5).toHex();

	QString freeOrProString = (GLOBAL(settings).freeOrProLevel) == AL_FREE ? "f" : "p";
	QString thumbPath = QString("images/%1/%2/{%3_%4_%5_%6}.%7")
		.arg(COMPRESSION_VERSION).arg("thumbnail")
		.arg(loadPathHash).arg(fsManager->getFileSize(loadPath))
		.arg(obj.thumbDimensions).arg(freeOrProString)
		.arg(compressed ? "dds" : "jpg");

	// check if we need to actually launch the process
	QString thumbFilePath = cacheDir + thumbPath;
	QDateTime fileModTime = QFileInfo(loadPath).lastModified();
	QFileInfo tfpInfo(thumbFilePath);
	if (exists(tfpInfo))
	{
		if (tfpInfo.lastModified() >= fileModTime)
			return thumbFilePath;
	}

	// run the process to make sure it's up to date
	QString texHelperAppStr = native(winOS->GetExecutableDirectory() / "TexHelper.exe");
	QString qpath = loadPath;
		ensureQuoted(qpath);
	QString qdir = cacheDir;
		ensureQuoted(qdir);
	QString qthumbPath = thumbPath;
		ensureQuoted(qthumbPath);
	QString qthumbDims = QString("%1").arg(obj.thumbDimensions);
	QString commandLine = QString("-f%1 -d%2 -t%3 -b%4")
		.arg(qpath).arg(qdir)
		.arg(qthumbPath).arg(qthumbDims);

	if (GLOBAL(settings).freeOrProLevel == AL_FREE)
		commandLine += " -e\"\" ";


	int result = createTexHelperProcess(texHelperAppStr, commandLine, obj.texhelperId, INFINITE, NULL, NULL);
	if ((result == 0) && exists(thumbFilePath))
	{
		return thumbFilePath;
	}
	return QString();
}

QString GLTextureManager::generateCacheFilePath(const QString& filePath, int imageDim, const QString& extension)
{
	QString loadPathHash = QCryptographicHash::hash(filePath.toUtf8(), QCryptographicHash::Md5).toHex();
	QString freeOrProString = (GLOBAL(settings).freeOrProLevel) == AL_FREE ? "f" : "p";

 	QString thumbPath = QString("images/%1/%2/{%3_%4_%5_%6}.%7")
 		.arg(COMPRESSION_VERSION).arg("images")
 		.arg(loadPathHash).arg(fsManager->getFileSize(filePath))
 		.arg(imageDim).arg(freeOrProString).arg(extension);

	return thumbPath;
}

QString GLTextureManager::generateCacheFilePath(const QString& filePath, int imageDim)
{
	return generateCacheFilePath(filePath, imageDim, QString("dds"));
}

bool GLTextureManager::swapLoadedTextureKeys(const QString& key1, const QString& key2)
{
	if (key1 == key2)
		return false;

	// Check to see if the texture keys exist
	TexturesList::iterator iter = _textures.find(key1);
	if (iter == _textures.end())
		return false;
	
	// We don't want to modify the key if it is not loaded
	if (iter->state != TextureLoaded)
		return false;
	
	iter = _textures.find(key2);
	if (iter == _textures.end())
		return false;

	if (iter->state != TextureLoaded)
		return false;

	// Remove them from the hash
	GLTextureObject obj1 = _textures.take(key1);
	GLTextureObject obj2 = _textures.take(key2);
	
	// Swap their keys
	obj1.key = key2;
	obj2.key = key1;

	// And insert them based on their new keys
	_textures.insert(obj1.key, obj1);
	_textures.insert(obj2.key, obj2);

	return true;
}

void GLTextureManager::killLoadingThreads()
{
	TextureThreads::iterator threadIter = _loadingThreads.begin();
	while (threadIter != _loadingThreads.end())
	{
		// Put the texture back on the image queue so it will
		// be loaded again
		QString key = (*threadIter)->getParam().key;
		GLTextureObject& obj = _textures[key];
		obj.state = ImageQueued;
		_imagesQueued.push(TexturesListIndex(&_textures, key));
		
		// Kill the loading thread
		(*threadIter)->join(9999);
		delete *threadIter;
		threadIter++;
	}	
	_loadingThreads.clear();
		
	// free images in the invalidated threads
	_ilMutex.lock();
	TextureThreads::iterator invThreadIter = _invalidatedLoadingThreads.begin();
	while (invThreadIter != _invalidatedLoadingThreads.end())
	{
		int imgId = (*invThreadIter)->getResult();
		if (imgId >= 0)
			ilDeleteImage(imgId);
		delete (*invThreadIter);
		invThreadIter++;
	}
	_invalidatedLoadingThreads.clear();
	_ilMutex.unlock();
}

void GLTextureManager::deleteNonPersistentTextures()
{
	killLoadingThreads();
	
	PrioritizedTextureIndices texQueue;
	while (!_texturesQueued.empty())
	{
		QString key = _texturesQueued.top().key;
		TexturesList::iterator iter = _textures.find(key);
		if (iter != _textures.end())
		{
			GLTextureObject& obj = iter.value();
			if (!obj.persistent)
			{
				if (obj.state == TextureLoaded && obj.resultCode == NoError)
				{
					assert(obj.resultTextureData);
					obj.resultTextureData->freeTexture();
					obj.resultTextureData->freeImage();
					obj.resultTextureData.reset();
				}
				// The pop operation here requires the texture to exist in the
				// _textures list, so we need to pop first, then erase
				_texturesQueued.pop();
				_textures.erase(iter);
				continue;
			}
			else
				texQueue.push(_texturesQueued.top());
		}
		_texturesQueued.pop();
	}
	_texturesQueued = texQueue;

	PrioritizedTextureIndices imgQueue;
	while (!_imagesQueued.empty())
	{
		QString key = _imagesQueued.top().key;
		TexturesList::iterator iter = _textures.find(key);
		if (iter != _textures.end())
		{
			bool persistent = iter.value().persistent;
			if (!persistent)
			{
				// The pop operation here requires the texture to exist in the
				// _textures list, so we need to pop first, then erase
				_imagesQueued.pop();
				_textures.erase(iter);
				continue;
			}
			else
				imgQueue.push(_imagesQueued.top());	
		}
		_imagesQueued.pop();
	}
	_imagesQueued = imgQueue;
}

bool isPowerOfTwo( int i )
{
	// http://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
	return !(i & (i - 1)) && i;
}

unsigned int closestPowerOfTwo( int i )
{
	assert(i > 0);

	// if i's a power of two, then return i
	if (!(i & (i - 1)) && i)
		return i;

	int j = 1;
	int prev = 1;
	while (j < i)
	{
		prev = j;
		j <<= 1;
	}
	assert(prev < i && i < j);
	if (i-prev < j-i)
		return prev;
	else
		return j;
}

unsigned int nextPowerOfTwo( int i )
{
	assert(i > 0);

	// if i's a power of two, then return i
	if (!(i & (i - 1)) && i)
		return i;

	unsigned int j = 1;
	while (j < i)
	{
		j <<= 1;
	}
	return j;
}
