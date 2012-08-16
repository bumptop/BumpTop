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

#ifdef ENABLE_WEBKIT
#include "BT_AnimationManager.h"
#include "BT_Authorization.h"
#include "BT_Camera.h"
#include "BT_EventManager.h"
#include "BT_FacebookWidgetJavaScriptAPI.h"
#include "BT_FileSystemManager.h"
#include "BT_JavaScriptAPI.h"
#include "BT_Macros.h"
#include "BT_OverlayComponent.h"
#include "BT_QtUtil.h"
#include "BT_RenderManager.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_SharingFlyout.h"
#include "BT_Util.h"
#include "BT_WebActor.h"
#include "BT_WebPage.h"
#include "BT_WindowsOS.h"

#include "moc/moc_BT_WebActor.cpp"
#include "BumpTop.pb.h"

#ifdef DXRENDER
#include "BT_DXRender.h"
#include "BT_Mesh.h"
#include "Bt_Vertex.h"
#include "BT_VideoRender.h"
#endif

class PerformanceCounter
{
	LARGE_INTEGER frequency, anchorTime;
	bool running;
public:
	PerformanceCounter(bool start = false)
	{
		QueryPerformanceFrequency(&frequency);
		running = start;
		if (start)
			QueryPerformanceCounter(&anchorTime);
	}
	void start()
	{
		running = true;
		QueryPerformanceCounter(&anchorTime);
	}
	double elapsed() // Returns 0 if not running, else returns seconds
	{
		if (!running)
			return 0;
		LARGE_INTEGER currentTime;
		QueryPerformanceCounter(&currentTime);
		return double(currentTime.QuadPart - anchorTime.QuadPart) / frequency.QuadPart;
	}
	void stop()
	{
		running = false;
	}
};

#define GL_ASSERT_NO_ERROR()	{ GLenum errorCode = glGetError(); if (GL_NO_ERROR != errorCode) \
								{ printf("GL_ERROR 0x%.4X %s(%d) \n", errorCode, __FILE__, __LINE__); _ASSERTE(!"GL_ASSERT_NO_ERROR"); } }

static const int FLYOUT_TIMEOUT_MS = 3000;

// Construct a new instance of WebActor.
WebActor::WebActor()
: Actor()
, _glTextureId(0)
, _glSpinnerTextureId(0)
, _spinnerAngle(0)
, _textureDirty(true) // so updateTexture will make the spinner texture
, _contentSize(winOS->GetWindowWidth(), winOS->GetWindowHeight())
, _fullBufferSize(winOS->GetWindowWidth(), winOS->GetWindowHeight())
, _smallBufferSize(winOS->GetWindowWidth() / 2, winOS->GetWindowHeight() / 2)
, _focused(false)
, _isPageDataURL(true)
, _prevMouseDown(NULL)
, _prevMouseUp(NULL)
, _singleClickTimer(new PerformanceCounter(false))
, _javaScriptAPI(NULL)
, _isDropSourceValid(false)
, _videoRender(NULL)
, _mesh(NULL)
, _autoUpdateTimer(NULL)
, _autoUpdateTimerDelay(0)
, _restartAutoUpdateTimerOnUnfocus(false)
, _flyout(NULL)
{
	LOG_FUNCTION_REACHED();
	pushActorType(Webpage);
	hideText();

	// give it an html icon by default
	setTextureID(winOS->GetSystemIconInfo(".html"));
	setGlobalPosition(Vec3(0, 20, 0));
	setGlobalOrientation(GLOBAL(straightIconOri));
	setDimsToDefault();
	
	_spinnerAngle = rand() % 360;
	_facebookWidgetDir = ::parent(winOS->GetLanguagesDirectory()) / "Facebook";

	_page.init(_contentSize, true);
	connect(&_page, SIGNAL(pageUpdate(const QImage&, const QRect&)), this, SLOT(pageUpdated(const QImage&, const QRect&)));
	// Need to use WebActor::reload to ensure JavaAPI is set correctly after reload
	connect(_page.getView()->pageAction(QWebPage::Reload), SIGNAL(triggered(bool)), this, SLOT(pageReloaded(bool)));

	updateExpectedSize();

	// setup auto-update (generate a delay of [5m..7m])
	const int baseDelay = 30 * 60 * 1000; // 30min
	const int offsetDelay = 30 * 1000; // 30s
	randomSeed();	
	_autoUpdateTimerDelay = baseDelay + (offsetDelay * randomInt(0, 5));
	_autoUpdateTimer = new QTimer(this);
	connect(_autoUpdateTimer, SIGNAL(timeout()), this, SLOT(autoUpdateReloadPage()));
}

JavaScriptAPI* WebActor::getJavaScriptAPI() {
	return _javaScriptAPI;
}

//Checks if there are already maxNumFreeWebWidgets+1 web widgets (+1 because this function is used to delete additional widgets, and not as a preventative check)
bool WebActor::canMakeMore() {
	LOG_LINE_REACHED();
	vector<BumpObject *> pages = scnManager->getBumpObjects(ObjectType(BumpActor, Webpage));
	int numWebActors = pages.size();
	int curNumFreeWebWidgets = 0;

	for(int x = 0; x < numWebActors; x++) {
		WebActor* p = dynamic_cast<WebActor *>(pages.at(x));
		if(p && !(p->isFacebookWidgetUrl() || (p->_isPageDataURL && isSharedFolderUrl(p->_pageData))))
			curNumFreeWebWidgets++;
	}

	return !((GLOBAL(settings).freeOrProLevel == AL_FREE) && (curNumFreeWebWidgets > GLOBAL(settings).maxNumFreeWebWidgets));
}

WebActor::~WebActor()
{
	LOG_FUNCTION_REACHED();
	_page.getView()->pageAction(QWebPage::Reload)->disconnect(this);
	_page.disconnect(this);
	SAFE_DELETE(_singleClickTimer);
	SAFE_DELETE(_prevMouseDown);
	SAFE_DELETE(_prevMouseUp);
	SAFE_DELETE(_javaScriptAPI);
	SAFE_DELETE(_flyout);
#ifdef DXRENDER
	onRelease();
#else
	if (_glSpinnerTextureId)
	{
		glDeleteTextures(1, &_glSpinnerTextureId);
		_glSpinnerTextureId = 0;
	}

	if (_glTextureId)
	{
		glDeleteTextures(1, &_glTextureId);
		_glTextureId = 0;
	}
#endif
}

const QString& WebActor::getPageData() const
{
	return _pageData;
}

Vec3 WebActor::getDefaultDims()
{
	// Make web actors bigger by default
	Vec3 dims = Actor::getDefaultDims();
	if (isSharingWidget())
	{
		dims.x *= 3.0f; 
		dims.y *= 3.0f;
	}
	else
	{
		dims.x *= 4.8f; 
		dims.y *= 4.8f;
	}
	return dims;
}

// Return true if this actor is a sharing widget
bool WebActor::isSharingWidget()
{
	return _isPageDataURL && isSharedFolderUrl(_pageData);
}

bool isFacebookWidgetUrl(const QDir& facebookWidgetDir, const QString& url, QString& pageOut)
{		
	QString localPath = QUrl::fromLocalFile(native(facebookWidgetDir)).toString();
	QString schema = QT_NT("bumpwidget-facebook://");
	if (url.startsWith(schema, Qt::CaseInsensitive) || 
		url.startsWith(localPath, Qt::CaseInsensitive))
	{
		pageOut = url.mid(schema.size());
		return true;
	}
	return false;
}

bool WebActor::isFacebookWidgetUrl()
{
	QString subPage;
	return ::isFacebookWidgetUrl(_facebookWidgetDir, _pageData, subPage);
}

#ifdef DEBUG
// For convenience during development, processes the HTML files every time a 
// web actor is reloaded. Normally, this happens during the build process.
void WebActor::processHtmlSources()
{
	QDir buildUtilsDir = ::parent(winOS->GetTexturesDirectory()) / "bin";
	QString exe = QT_NT("python.exe");
	QString scriptName = buildUtilsDir.absoluteFilePath(QT_NT("processHtmlSources.py"));
	consoleWrite(QString("Processing HTML templates ('%1 %2')...\n").arg(exe).arg(scriptName));

	QProcess process;
	DWORD startTime = timeGetTime();
	process.start(exe, QStringList(scriptName));
	process.setReadChannelMode(QProcess::ForwardedChannels);
	process.waitForFinished();
	consoleWrite(QString("Done processing HTML templates (%1ms).\n").arg(timeGetTime() - startTime));
}
#endif

// executeQueuedCodeOnThisPage is a flag used to tell the webpage whether the queued up javascript
// should be executed when it is done loading.
void WebActor::load( const QString& url , bool executeQueuedCodeOnThisPage)
{
	QString subPage;
	if (::isFacebookWidgetUrl(_facebookWidgetDir, url, subPage))
	{
		// determine which page to show
		FacebookWidgetJavaScriptAPI * fbApi = (FacebookWidgetJavaScriptAPI *) _javaScriptAPI;
		if (!_javaScriptAPI)
			_javaScriptAPI = fbApi = new FacebookWidgetJavaScriptAPI(this, &_page);

		// initialize facebook
		QString page = QT_NT("facebook_login.html");
		if (fbApi->initialize())
		{
			// try and load the last page
			page = QT_NT("facebook_newsfeed.html");
			QString storedPage = fbApi->getPersistentStoreValue(QT_NT("last_page"));
			if (!storedPage.isEmpty())
				page = QString(QT_NT("facebook_%1.html")).arg(storedPage);
		}

		// load the facebook page
		QString url = native(make_file(_facebookWidgetDir, page));
		if (exists(url)) 
		{
			QString uri = QUrl::fromLocalFile(url).toString();
			_page.load(uri, _javaScriptAPI, executeQueuedCodeOnThisPage);
		}

		// move to Qt 4.6.1 made scrollbars always display, so explicitly hide them
		_page.hideScrollBars();
	}
	else if (isSharedFolderUrl(url))
	{
		QString widgetSource = winOS->GetSharingWidgetPath();
		assert(QFileInfo(widgetSource).exists());
		QUrl sharingWidgetUrl = QUrl::fromLocalFile(widgetSource);

		// Load the page with the shared folder API enabled
		if (!_javaScriptAPI)
			_javaScriptAPI = new SharedFolderAPI(this, &_page);
#ifdef DEBUG
		// Eventually this should be done for the Facebook widget too
		processHtmlSources();
#endif
		_page.load(sharingWidgetUrl.toString(), _javaScriptAPI, executeQueuedCodeOnThisPage);

		// delete flyout in case widget was reloaded, esp. while uploading
		deleteFlyout();
	}
	else
	{
		assert(!_javaScriptAPI); // For now, no other pages should have the API
		_page.load(url);

		// start the auto updater
		_autoUpdateTimer->start(_autoUpdateTimerDelay);
	}
	_pageData = url;
	_isPageDataURL = true;
}

void WebActor::loadHTML( const QString& html )
{
	_page.loadHTML(html);
	_pageData = html;
	_isPageDataURL = false;
}

void WebActor::reload()
{
	// We could just do this:
	// _page.getView()->reload();
	// but then the page will lose the API
 	if (_isPageDataURL)
 		load(_pageData);
 	else
 		loadHTML(_pageData);
}

#ifdef DXRENDER
IDirect3DTexture9 * WebActor::getTextureNum()
{
	if (_videoRender)
	{
		unsigned int w, h;
		_videoRender->GetTextureSize(w, h);
		if (0 < w && 0 < h)
			updateAspectRatio(QSize(w, h), false);
		IDirect3DTexture9 * videoTexture = NULL;
		_videoRender->GetTexture(&videoTexture);
		return videoTexture;
	}
#else
uint WebActor::getTextureNum()
{
#endif
	if (_glTextureId)
		return _glTextureId;
	else
		return Actor::getTextureNum();
}

bool WebActor::hasEventOverride() const
{
	return false;
}

bool WebActor::wantsMouseEvent(QMouseEvent * evt) 
{
	return _page.wantsBumpTopMouseEvents();
}

bool WebActor::onMouseEvent(QMouseEvent * evt)
{
	if (!wantsMouseEvent(evt))
		return false;

	if (_page.isExternal())
	{
		SAFE_DELETE(evt);
		return false;
	}

	// we need to convert the screen coordinates to the actor coordinates
	int x, y;
	mapLocalMouseCoords(evt->x(), evt->y(), x, y);

	// skip event if it is a mouse down and the mouse is not in the "window"
	// skip event if it is a mouse move and no button has been pressed
	QSize contentSize = _page.getContentSize();
	bool isInBounds = !(x < 0 || y < 0 || x > contentSize.width() || y > contentSize.height()); 
	switch (evt->type())
	{
	case QEvent::MouseButtonPress:
		if (!isInBounds)
			return false;
		break;
	case QEvent::MouseMove:
		if (!isInBounds && !evt->buttons())
			return false;
		break;
	default: break;
	}

	QMouseEvent * newEvt = new QMouseEvent(evt->type(), QPoint(x, y), evt->button(), evt->buttons(), evt->modifiers());
	delete evt;

	if (_focused)
	{
		_page.invalidateBuffer();
		bool result = _page.getView()->event(newEvt);
		SAFE_DELETE(newEvt);
		return result;
	}
	else
	{
		if (QEvent::MouseButtonRelease == newEvt->type())
		{
			SAFE_DELETE(_prevMouseUp);
			_prevMouseUp = newEvt;
		}
		else if (QEvent::MouseButtonPress == newEvt->type())
		{
			if (!_prevMouseDown)
				_singleClickTimer->start();
			SAFE_DELETE(_prevMouseDown);
			_prevMouseDown = newEvt;
		}
		return true;
	}
}

bool WebActor::onWheelEvent( QWheelEvent * evt )
{
	if (_page.isExternal())
	{
		SAFE_DELETE(evt);
		return false;
	}

	// Allow CTRL + scroll wheel to resize actor like others
	if (Qt::ControlModifier & evt->modifiers())
	{
		if (0 < evt->delta())
			this->grow(15, 1.4f);
		else if (0 > evt->delta())
			this->shrink(15, 0.7f);
		return true;
	}

	// we need to convert the screen coordinates to the actor coordinates
	int x, y;
	mapLocalMouseCoords(evt->x(), evt->y(), x, y);

	// skip if the mouse is not in the "window"
	QSize contentSize = _page.getContentSize();
	if (x < 0 || y < 0 || x > contentSize.width() || y > contentSize.height())
		return false;

	int sign = NxMath::sign(evt->delta());
	int delta = sign * NxMath::max(120, NxMath::abs(evt->delta()));
	QWheelEvent * newEvt = new QWheelEvent(QPoint(x, y), QPoint(evt->x(), evt->y()), delta, evt->buttons(), evt->modifiers(), evt->orientation());
	delete evt;
	_page.invalidateBuffer();
	int defaultScrollLines = qApp->wheelScrollLines();
	qApp->setWheelScrollLines(1);
	bool result = _page.getView()->event(newEvt);
	qApp->setWheelScrollLines(defaultScrollLines);

	return result;
}

bool WebActor::onKeyEvent( QKeyEvent * evt )
{
	SAFE_DELETE(evt);
	return false;
}

void WebActor::onUpdate()
{	
	// Selected web actors have no update frequency limit; sharing and facebook widgets have high update frequency
	bool limitUpdates = !(sel->getSize() == 1 && sel->isInSelection(this));
	int updateInterval = 500; // ms
	if (isSharedFolderUrl(_pageData))
		updateInterval = 80;
	else if (isFacebookWidgetUrl())
		updateInterval = 80;
	
	if (!limitUpdates || (_updateTimer.elapsed() > updateInterval))
	{
		if (!_page.isLoaded())
			if (!this->isAnimationDisabled())
				rndrManager->invalidateRenderer(); // Draw actor with spinner

		// Update the size of the widget if the page requested a specific size in pageLoaded, also change page's buffer size
		if (_page.getContentSize() != _contentSize)
			updateExpectedSize();

		_page.onUpdate();
		updateTexture();
	
		if (limitUpdates)
			_updateTimer.restart();
	}

	if (_singleClickTimer->elapsed() > GLOBAL(dblClickInterval))
	{
		if (_prevMouseDown)
		{
			_page.getView()->event(_prevMouseDown);
			SAFE_DELETE(_prevMouseDown); 
		}
		if (_prevMouseUp)
		{
			_page.getView()->event(_prevMouseUp);
			SAFE_DELETE(_prevMouseUp);
		}
		_singleClickTimer->stop();
	}
}

void WebActor::updateAspectRatio(const QSize & contentSize, bool animate)
{
	Vec3 curSize = getDims();
	int width = contentSize.width();
	int height = contentSize.height();
	float biggest = curSize.x > curSize.y ? curSize.x : curSize.y;
	float aspect = curSize.x / curSize.y;
	float newAspect = (float)width/height;

	if (aspect >= (newAspect + 0.01f) || aspect <= (newAspect - 0.01f))
	{
		// Determine whether to scale the x or the y (they are always bound by the largest side)
		Vec3 newSize(0.0f);
		if (newAspect > 1.0f)
			newSize.set(biggest, biggest/newAspect, curSize.z);
		else
			newSize.set(biggest*newAspect, biggest, curSize.z);

		// Animate the dimensions of the object to its new aspect ratio
		if (animate)
			setSizeAnim(curSize, newSize, 30);
		else 
			setDims(newSize);
	}
}

// This slot is triggered after we have processed the update from WebKit.
// Propagate the updates to the object texture.
void WebActor::pageUpdated( const QImage & image, const QRect & dirtyRect )
{
	_textureDirty = true;
	
	if (_glSpinnerTextureId && _page.isLoaded())
	{
#ifdef DXRENDER
		SAFE_RELEASE(_glSpinnerTextureId);
#else
		glDeleteTextures(1, &_glSpinnerTextureId);
		_glSpinnerTextureId = 0;
		GL_ASSERT_NO_ERROR();
#endif
	}
	// update this widget dimensions based on the image aspect ratio
	updateAspectRatio(_contentSize, true);
}

void WebActor::pageReloaded(bool checked)
{
	reload();
}

void WebActor::autoUpdateReloadPage()
{
	assert(!isFocused());
	if (!isAnimationDisabled())
		reload();
}

void WebActor::updateTexture()
{
	if (!dxr->isDeviceReady())
		return;

	if (!_textureDirty)
		return;
	_textureDirty = false;

	QImage image = _page.getBuffer();
	if (image.size().isEmpty())
		return; // If page image is not valid, return without clearing _textureDirty.
	
	_ASSERT(image.size() == _fullBufferSize); // The page image should be full size
	image = image.scaled(_smallBufferSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation); //Qt::KeepAspectRatio___ results in dimension rounding error
	_ASSERT(image.width() == _smallBufferSize.width() && image.height() == _smallBufferSize.height());
	
	if (!_page.isLoaded())
	{
		int w = _smallBufferSize.width(), h = _smallBufferSize.height(); 
			
		// Make a background radial gradiant from light grey in the center to darker grey outside
		QRadialGradient backgroundGradiant(w / 2, h / 2, NxMath::max(w / 2, h / 2)); 
		backgroundGradiant.setColorAt(0, QColor(214, 225, 229));
		backgroundGradiant.setColorAt(1, QColor(141, 164, 172));
		QPainter painter(&image);
		painter.fillRect(QRect(QPoint(0,0), _smallBufferSize), QBrush(backgroundGradiant));	
		painter.end();

		if (!_glSpinnerTextureId) // Make a spinner texture with transparent background and conical gradiant
		{
			QImage spinnerImage(w, h, QImage::Format_ARGB32_Premultiplied);
			spinnerImage.fill(0);
			
			int spinnerSize = NxMath::min(w, h) / 2;
			int x = (w - spinnerSize) / 2, y = (h - spinnerSize) / 2; 
			// Make a conical spinner gradiant from light blue to transparent light grey 
			QConicalGradient spinnerGradiant(w / 2, h / 2, _spinnerAngle / 16);
			spinnerGradiant.setColorAt(0, QColor(18, 149, 203));
			spinnerGradiant.setColorAt(1, QColor(197, 233, 250, 0));
			
			QPainter spinnerPainter(&spinnerImage);
			spinnerPainter.setPen(QPen(QBrush(spinnerGradiant), 20));
			spinnerPainter.drawArc(QRectF(x, y, spinnerSize, spinnerSize), 0, 360 * 16);
			spinnerPainter.end();

#ifdef DXRENDER
			_glSpinnerTextureId = dxr->createTextureFromData(spinnerImage.width(), spinnerImage.height(), spinnerImage.bits(), spinnerImage.bytesPerLine());
#else
			glGenTextures(1, &_glSpinnerTextureId);
			glBindTexture(GL_TEXTURE_2D, _glSpinnerTextureId);
			GL_ASSERT_NO_ERROR();
			// Set up default texture properties
			if (GLOBAL(settings).useAnisotropicFiltering)
			{
				float maximumAnisotropy;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnisotropy);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maximumAnisotropy);
				GL_ASSERT_NO_ERROR();
			}
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
			GL_ASSERT_NO_ERROR();

			// manually make 1 mipmapped level
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, spinnerImage.width(), spinnerImage.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, spinnerImage.bits());
			spinnerImage = spinnerImage.scaled(image.width() / 2, image.height() / 2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, spinnerImage.width(), spinnerImage.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, spinnerImage.bits());
			GL_ASSERT_NO_ERROR();
#endif
		}
	}

#ifdef DXRENDER

	//QWidget * _widget = getViewWidget();
	//_widget->setFixedSize(_contentSize);
	//_widget->setWindowFlags(Qt::FramelessWindowHint);
	//_widget->show();

	//PWND pwnd = WindowFromDC(_widget->getDC());
	//SetParent(pwnd, winOS->GetWindowsHandle());
	//SetWindowPos(pwnd, HWND_TOP, (winOS->GetWindowWidth() - _contentSize.width()) / 2, (winOS->GetWindowHeight() - _contentSize.height()) / 2, 0, 0, SWP_NOSIZE);

	////pwnd = winOS->GetWindowsHandle();
	//HMODULE hDll = LoadLibrary(L"dwmapi.dll");
	//typedef HRESULT (__stdcall * UPDATEWINDOWSHARED)(HWND hWnd, int one, int two, int three, void * hMonitor, void * unknown);
	//typedef HRESULT (__stdcall * GETSHAREDSURFACE)(HWND hWnd, LUID adapterLuid, uint one, uint two, D3DFORMAT * pD3DFormat, void ** pSharedHandle, void * unknown);
	////typedef HRESULT (* GETSHAREDSURFACE)(char unknown [32]);
	//GETSHAREDSURFACE GetSharedSurface = (GETSHAREDSURFACE)GetProcAddress(hDll, (LPCSTR)100);
	//UPDATEWINDOWSHARED UpdateWindowShared = (UPDATEWINDOWSHARED)GetProcAddress(hDll, (LPCSTR)101);

	//struct SurfaceInfo
	//{
	//	unsigned int unknown;
	//	unsigned int shareCount;
	//} surfaceInfo;

	//D3DFORMAT d3dFormat = D3DFMT_UNKNOWN;
	//void * sharedHandle = NULL;
	//LUID luid = {0};
	//HRESULT hr = ((IDirect3D9Ex *)dxr->d3d)->GetAdapterLUID(D3DADAPTER_DEFAULT, &luid);
	//hr = GetSharedSurface(pwnd, luid, 0, 0, &d3dFormat, &sharedHandle, &surfaceInfo);
	//_ASSERT(SUCCEEDED(hr));

	//char unknown [32] = {0};
	//hr = UpdateWindowShared(pwnd, 0, 0, 0, dxr->d3d->GetAdapterMonitor(D3DADAPTER_DEFAULT), unknown);
	//_ASSERT(SUCCEEDED(hr));

	//RECT rect = {0};
	//GetWindowRect(pwnd, &rect);
	//unsigned int windowW = rect.right - rect.left, windowH = rect.bottom - rect.top;
	//IDirect3DTexture9 * texture = NULL;
	////hr = ((IDirect3DDevice9Ex *)dxr->device)->CreateTexture(windowW, windowH, 1, 0/*D3DUSAGE_RENDERTARGET*/, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &texture, &sharedHandle);
	//hr = dxr->device->CreateTexture(windowW, windowH, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &texture, &sharedHandle);
	//_ASSERT(SUCCEEDED(hr));

	//D3DXSaveTextureToFileA("window.png", D3DXIFF_PNG, texture, NULL);
	//SAFE_RELEASE(texture);
	//_ASSERTE(!"window.png");

	if (!_glTextureId)
	{
		unsigned int width = image.width(), height = image.height();
		HRESULT hr = dxr->device->CreateTexture(width, height, 0, D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &_glTextureId, NULL);
		_ASSERT(D3D_OK == hr);
	}
	{
		QImage::Format format = image.format();
		unsigned int width = image.width(), height = image.height();
		D3DLOCKED_RECT lockedRect = {0};
		HRESULT hr = _glTextureId->LockRect(0, &lockedRect, NULL, 0);
		_ASSERT(D3D_OK == hr);
		if (lockedRect.Pitch == width * 4 && lockedRect.Pitch == image.bytesPerLine())
			memcpy(lockedRect.pBits, image.bits(), width * height * 4);
		else
			for (unsigned int i = 0; i < height; i++)
				memcpy((char *)lockedRect.pBits + i * lockedRect.Pitch, image.bits() + image.bytesPerLine() * i, image.bytesPerLine());
		hr = _glTextureId->UnlockRect(0);
		_ASSERT(D3D_OK == hr);
	}
	_glTextureId->AddDirtyRect(NULL);
	_glTextureId->GenerateMipSubLevels();
#else
	if (!_glTextureId)
	{
		glGenTextures(1, &_glTextureId);
		glBindTexture(GL_TEXTURE_2D, _glTextureId);
		GL_ASSERT_NO_ERROR();
		// set up default texture properties
		if (GLOBAL(settings).useAnisotropicFiltering)
		{
			float maximumAnisotropy;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnisotropy);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maximumAnisotropy);
			GL_ASSERT_NO_ERROR();
		}
		if (GLEW_ARB_texture_border_clamp) // Clamping the uvs to ensure no texture bleeding
		{
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_ARB);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_ARB);
		}
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1); // only one mipmapped level
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, image.width() / 2, image.height() / 2, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		GL_ASSERT_NO_ERROR();
	}
	else
		glBindTexture(GL_TEXTURE_2D, _glTextureId);
	
	// manually make mipmaps because actor could appear smaller than texture and use mip map levels
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_BGRA, GL_UNSIGNED_BYTE, image.bits());
	image = image.scaled(image.width() / 2, image.height() / 2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, image.width(), image.height(), GL_BGRA, GL_UNSIGNED_BYTE, image.bits());
	GL_ASSERT_NO_ERROR();
#endif

	rndrManager->invalidateRenderer();
}

QWidget * WebActor::getViewWidget()
{
	return _page.getView();
}

void WebActor::mapLocalMouseCoords( int screenX, int screenY, int& localXOut, int& localYOut )
{
	// get the point on the actor
	float dist;
	Vec3 nearPt, farPt, dir;
	Vec3 pointOnActor;
	window2world(screenX, screenY, nearPt, farPt);
	dir = farPt - nearPt;
	dir.normalize();	
	Ray worldRay(nearPt, dir);
	NxPlane frontFace = getFrontFacePlane();
	frontFace.d = -frontFace.d;
	NxRayPlaneIntersect(worldRay, frontFace, dist, pointOnActor);
	getGlobalPose().multiplyByInverseRT(pointOnActor, pointOnActor);

	// consoleWrite(QString("PointOnActor: %1,%2,%3\n").arg(pointOnActor.x).arg(pointOnActor.y).arg(pointOnActor.z));

	// convert that to pixel coordinates for the webpage
	QSize contentSize = _page.getContentSize();
	Vec3 dims = getDims();
	Vec3 ratio((dims.x - pointOnActor.x) / (2.0f * dims.x), (dims.y - pointOnActor.y) / (2.0f * dims.y), 0.0f);
	localXOut = ratio.x * contentSize.width();
	localYOut = ratio.y * contentSize.height();
}

void WebActor::onDragBegin(FinishedDragCallBack func)
{
	float alpha = getAlpha();
	Actor::onDragBegin(func);

	// override the alpha setting in BumpObject::onDragBegin()
	setAlpha(alpha);
}

bool WebActor::isFocused() const
{
	return _focused;
}

void WebActor::setFocused( bool focused )
{
	if (_focused != focused)
		_textureDirty = true;

	_focused = focused;
	if (!_focused)
	{
		setIsExternal(false); // hide QWebView as soon as actor is unfocused
		_page.invalidateBuffer(); 
		_page.onUpdate();
		updateTexture();

		// restart the auto update timer
		if (_restartAutoUpdateTimerOnUnfocus)
			_autoUpdateTimer->start(_autoUpdateTimerDelay);
	}
	else
	{
		// stop the auto update timer
		_restartAutoUpdateTimerOnUnfocus = _autoUpdateTimer->isActive();
		_autoUpdateTimer->stop();
	}
}

void WebActor::setIsExternal(bool isExternal)
{
	_page.setIsExternal(isExternal);
	if (isExternal)
	{
		pushActorType(Invisible);
		if (_javaScriptAPI) 
			_javaScriptAPI->onWidgetFocus();
	}
	else
	{
		popActorType(Invisible);
		if (_javaScriptAPI) 
			_javaScriptAPI->onWidgetBlur();
	}
}

bool WebActor::hasFocusedWebActor()
{
	const QList<BumpObject *> & zoomedObjects = cam->getZoomedObjects();
	return (1 == zoomedObjects.size() && zoomedObjects[0]->isObjectType(ObjectType(BumpActor, Webpage)));
}

bool WebActor::zoomOutFocusedWebActor()
{
	QList<BumpObject *> & zoomedObjects = cam->getZoomedObjects();
	if (WebActor::hasFocusedWebActor())
	{
		_ASSERT(((WebActor *)zoomedObjects[0])->isFocused());
		((WebActor *)zoomedObjects[0])->setFocused(false); // For WebActors, when camera zooms out they should be unfocused.
		zoomedObjects.clear();
		cam->loadCameraFromPreset(GLOBAL(settings.cameraPreset));
		cam->restorePreviousVisibleNameables(false);
		return true;
	}
	return false;
}

void WebActor_finishLaunchingCallback(void * callbackData)
{
	if (cam->getZoomedObjects().size() == 1 && cam->getZoomedObjects().front() == (WebActor *)callbackData)
		((WebActor *)callbackData)->setIsExternal(true);
}

void WebActor::onLaunch()
{
	LOG_LINE_REACHED();

	SAFE_DELETE(_prevMouseDown);
	SAFE_DELETE(_prevMouseUp);
	
	if (!isSharingWidget())
	{
		cam->getZoomOutControl()->setText(QString("X"), Vec3(-0.04f, -0.05f, 0));

		// orient the camera correctly
		Vec3 newEye, newUp, newDir;
		this->stopAllMotion();
		this->putToSleep();
		animManager->removeAnimation(cam);
		cam->zoomToOrientedActorBoundsAttributes(this, newEye, newUp, newDir, 1, Vec2(_contentSize.width(), _contentSize.height(), 0));
		cam->setAnimationFinishCallback(&WebActor_finishLaunchingCallback, this);
		cam->animateTo(newEye, newDir, newUp);
		cam->getZoomedObjects().append(this);
		cam->storePreviousVisibleNameables(false);
		setFocused(true);
	}
}

// QSize is the desired size of the web actor.
// Pass QSize(0,0) to calculate it based on the size of the window.
void WebActor::updateExpectedSize()
{
	LOG_LINE_REACHED();

	if (!dxr->isDeviceReady())
		return;

	if (_page.getRequestedSize().isNull())	// Page didn't request size, so make it 90% of window width, 85% of height
		_contentSize = QSize(winOS->GetWindowWidth() * 0.9f, winOS->GetWindowHeight() * 0.85f);
	else if (_contentSize == _page.getRequestedSize()) // Page requested size and it's already matching
		return; 
	else // Page requested new size
		_contentSize = _page.getRequestedSize();

#ifdef DXRENDER
	if ((dxr->caps.TextureCaps & D3DPTEXTURECAPS_POW2) || (dxr->caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY))
	{
#else
	if (!(GLEW_ARB_texture_non_power_of_two && GLEW_ARB_texture_rectangle))
	{
#endif
		// For gfx cards that requires square & power-of-2 textures
		int size = nextPowerOfTwo(max(_contentSize.width(), _contentSize.height()));
		_fullBufferSize = QSize(size, size); 
	}
	else
		_fullBufferSize = _contentSize;
	
	_smallBufferSize = isSharingWidget() ? _fullBufferSize : _fullBufferSize / 2;

#ifdef DXRENDER
	SAFE_RELEASE(_glTextureId);
#else
	if (_glTextureId)
	{
		glDeleteTextures(1, &_glTextureId);
		_glTextureId = 0;
		_textureDirty = true;
	}
#endif

	updateAspectRatio(_contentSize, false);
		
	_page.setContentSize(_contentSize, _fullBufferSize);

#ifdef DXRENDER
	IDirect3DVertexBuffer9* vertexBuffer = generateVertexBuffer();
	setDisplayListId(vertexBuffer);
	vertexBuffer->Release();
#else
	float u = float(_contentSize.width()) / _fullBufferSize.width();
	float v = float(_contentSize.height()) / _fullBufferSize.height();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	setDisplayListId(glGenLists(1));//get a unique display list ID.
	
	GL_ASSERT_NO_ERROR();
	glNewList(getDisplayListId(), GL_COMPILE);  

	// make the no-side-texture box	
	glBegin(GL_QUADS);
		glNormal3f(0,0,1);
		glTexCoord2f(u,0); 	glVertex3f(1,1,1);
		glTexCoord2f(0,0); 	glVertex3f(-1,1,1);
		glTexCoord2f(0,v); 	glVertex3f(-1,-1,1);
		glTexCoord2f(u,v); 	glVertex3f(1,-1,1);

		glNormal3f(0,0,-1);
		glTexCoord2f(0,v); 	glVertex3f(1,-1,-1);
		glTexCoord2f(u,v); 	glVertex3f(-1,-1,-1);
		glTexCoord2f(u,0); 	glVertex3f(-1,1,-1);
		glTexCoord2f(0,0); 	glVertex3f(1,1,-1);
	glEnd();
	glEndList();

	GL_ASSERT_NO_ERROR();
#endif
}

#ifdef DXRENDER
IDirect3DVertexBuffer9* WebActor::generateVertexBuffer()
{
	float u = float(_contentSize.width()) / _fullBufferSize.width();
	float v = float(_contentSize.height()) / _fullBufferSize.height();

	IDirect3DVertexBuffer9 * vertexBuffer = NULL;
	HRESULT hr = dxr->device->CreateVertexBuffer(sizeof(PositionNormalTextured) * 12, D3DUSAGE_WRITEONLY, PositionNormalTextured::GetFlexibleVertexFormat(), D3DPOOL_DEFAULT, &vertexBuffer, NULL);
	_ASSERT(D3D_OK == hr);
	{
		PositionNormalTextured * vertexBufferData = NULL;
		hr = vertexBuffer->Lock(0, 0, (void **)&vertexBufferData, 0);
		_ASSERT(D3D_OK == hr);
		vertexBufferData[ 0] = PositionNormalTextured( 1, 1, 1, 0, 0, 1, u, 0);
		vertexBufferData[ 1] = PositionNormalTextured(-1, 1, 1, 0, 0, 1, 0, 0);
		vertexBufferData[ 2] = PositionNormalTextured(-1,-1, 1, 0, 0, 1, 0, v);
		vertexBufferData[ 3] = PositionNormalTextured(-1,-1, 1, 0, 0, 1, 0, v);
		vertexBufferData[ 4] = PositionNormalTextured( 1,-1, 1, 0, 0, 1, u, v);
		vertexBufferData[ 5] = PositionNormalTextured( 1, 1, 1, 0, 0, 1, u, 0);
		vertexBufferData[ 6] = PositionNormalTextured( 1,-1,-1, 0, 0,-1, 0, v);
		vertexBufferData[ 7] = PositionNormalTextured(-1,-1,-1, 0, 0,-1, u, v);
		vertexBufferData[ 8] = PositionNormalTextured(-1, 1,-1, 0, 0,-1, u, 0);
		vertexBufferData[ 9] = PositionNormalTextured(-1, 1,-1, 0, 0,-1, u, 0);
		vertexBufferData[10] = PositionNormalTextured( 1, 1,-1, 0, 0,-1, 0, 0);
		vertexBufferData[11] = PositionNormalTextured( 1,-1,-1, 0, 0,-1, 0, v);
		hr = vertexBuffer->Unlock();
		_ASSERT(D3D_OK == hr);
		vertexBufferData = NULL;
	}
	return vertexBuffer;
}
#endif

void WebActor::onClick()
{
	if (!_flyout)
		createFlyout();

	fadeInFlyout();
}

void WebActor::onSelect()
{
	Actor::onSelect();

	// check if we are supposed to be focused
	if (!_focused)
	{
		// ensure that if there is one zoomed in item and it's a web actor
		// then it is focused
		if (cam->getZoomedObjects().size() == 1 && 
			cam->getZoomedObjects().front() == this)
		{
			onLaunch();
		}

		// enable progress notifications
		_page.setProgressVisible(false);
	}
}

void WebActor::onDeselect()
{
	Actor::onDeselect();

	fadeOutFlyout();

	assert(!_focused);
}

void WebActor::onRender(uint flags)
{
	if (isActorType(Invisible) || _page.isExternal())
		return;

	// Make sure the flyout knows the current position of the actor, so it
	// can render in the correct place. If the position has not changed, this
	// is a cheap operation.
	if (_flyout && _flyout->isVisible())
	{
		if (GLOBAL(exitBumpTopFlag))
			_flyout->hide();
		else
			_flyout->setOriginRect(boundsToQRect(getScreenBoundingBox()));
	}

#ifdef DXRENDER
	if (!_glTextureId && !_glSpinnerTextureId)
	{
		_textureDirty = true;
		updateTexture();
	}

	if (_mesh)
		_mesh->Render(getGlobalPosition(), getGlobalOrientation(), getDims());
	else
#endif
	{
#ifdef DXRENDER
		if (!getCustomDisplayListId())
		{
			IDirect3DVertexBuffer9* vertexBuffer = generateVertexBuffer();
			setDisplayListId(vertexBuffer);
			vertexBuffer->Release();
		}
#endif
		Actor::onRender(flags | RenderCustomDisplayList);
	}

	if (!_page.isLoaded()) // draw the spinner textured box on top of normal actor
	{
		// Spin the texture coords to spin the spinner
		float aspect = float(_contentSize.width()) / float(_contentSize.height());
		_spinnerAngle = _spinnerAngle++ % 360;
#ifdef DXRENDER
		// TODO DXR
		D3DXMATRIX matrix;
		dxr->device->SetTransform(D3DTS_TEXTURE0, &D3DXMATRIX(
			1,    0,    0, 0,
			0,    1,    0, 0,
			+0.5, +0.5, 1, 0,
			0,    0,    0, 0));
		dxr->device->MultiplyTransform(D3DTS_TEXTURE0, &D3DXMATRIX(
			1 / aspect, 0, 0, 0,
			0,          1, 0, 0,
			0,          0, 1, 0,
			0,          0, 0, 0));
		dxr->device->MultiplyTransform(D3DTS_TEXTURE0, D3DXMatrixRotationZ(&matrix, _spinnerAngle * D3DX_PI / -180));
		dxr->device->MultiplyTransform(D3DTS_TEXTURE0, &D3DXMATRIX(
			1,    0,    0, 0,
			0,    1,    0, 0,
			-0.5, -0.5, 1, 0,
			0,    0,    0, 0));
		dxr->device->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
		dxr->renderSideLessBox(getGlobalPosition(), getGlobalOrientation(), getDims(), _glSpinnerTextureId);
		dxr->device->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
#else
		glMatrixMode(GL_TEXTURE);
		glTranslatef(0.5f, 0.5f, 0);
		glScalef(1 / aspect, 1, 1);
		glRotatef(_spinnerAngle * -2, 0, 0, 1);
		glTranslatef(-0.5f, -0.5f, 0);
		
		// render the spinner texture on the normal actor
		glBindTexture(GL_TEXTURE_2D, _glSpinnerTextureId);
		glEnable(GL_BLEND);
		glMatrixMode(GL_MODELVIEW);
		ShapeVis::renderSideLessBox(getGlobalPosition(), getGlobalOrientation(), getDims());
		
		//reset texture matrix
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		GL_ASSERT_NO_ERROR();
#endif
	}
}

#ifdef DXRENDER
void WebActor::onRelease()
{
	SAFE_DELETE(_videoRender);
	SAFE_DELETE(_mesh);
	SAFE_RELEASE(_glSpinnerTextureId);
	SAFE_RELEASE(_glTextureId);
	
	// Call base class
	Actor::onRelease();
}
#endif

bool WebActor::serializeToPb(PbBumpObject * pbObject)
{
	assert(pbObject);

	bool invisible = isActorType(Invisible); 
	popActorType(Invisible); // Actor must be visible for serialization

	// serialize the core actor properties
	if (!Actor::serializeToPb(pbObject))
	{
		if (invisible)
			pushActorType(Invisible);
		return false;
	}

	// write whether the data is a url or html page data
	pbObject->SetExtension(PbWebActor::type, (_isPageDataURL ? 1 : 0));

	// write the data (either url or html page data)
	pbObject->SetExtension(PbWebActor::content, stdString(_pageData));

	if (invisible)
		pushActorType(Invisible);

	return pbObject->IsInitialized();
}

bool WebActor::deserializeFromPb(const PbBumpObject * pbObject)
{
	assert(pbObject);

	// deserialize the core actor properties
	if (!Actor::deserializeFromPb(pbObject))
		return false;

	// read the url / html page data
	if (pbObject->HasExtension(PbWebActor::type) && 
		pbObject->HasExtension(PbWebActor::content))
	{
		bool isContentUrl = pbObject->GetExtension(PbWebActor::type) > 0;
		QString content = qstring(pbObject->GetExtension(PbWebActor::content));

		// load the page
		if (isContentUrl)
			load(content);
		else
			loadHTML(content);
	}

	return true;
}

bool WebActor::isValidDropTarget()
{
	return (_javaScriptAPI != NULL);
}

QString WebActor::resolveDropOperationString( vector<BumpObject *>& objList )
{
	return QString(QT_TR_NOOP("Upload"));
}

void WebActor::onDropEnter( vector<BumpObject *> &objList )
{
	_isDropSourceValid = _javaScriptAPI->onDropEnter(objList);

	// Call the superclass impl, which will call isSourceValid()
	Actor::onDropEnter(objList);
}

bool WebActor::isSourceValid()
{
	return _isDropSourceValid;
}

// Handle a drop operation. We assume that objList is the same
// as when onDropEnter is called.
vector<BumpObject *> WebActor::onDrop( vector<BumpObject *> &objList )
{
	vector<BumpObject *> failedObjs;
	if (!_javaScriptAPI->onDrop())
	{
		// It's all or nothing
		failedObjs = objList;
	}
	animateObjectsBackToPreDropPose(objList);
	return failedObjs;
}

void WebActor::onDropExit()
{
	// We must call the parent impl BEFORE resetting _isDropSourceValid, because
	// things get screwed up if isSourceValid() returns false before the entire
	// drop operation is complete.
	Actor::onDropExit();
	_isDropSourceValid = false;
}

bool WebActor::isAnimationDisabled()
{
	return GLOBAL(settings).disableAnimationsOnBattery && EventManager::Unplugged == evtManager->getACPowerStatus();
}

bool WebActor::isUpdatingDisabled()
{
	return GLOBAL(settings).disablePhotoframesOnBattery && EventManager::Unplugged == evtManager->getACPowerStatus();
}

// When the flyout is invoked automatically (i.e. not because the user clicked on
// the actor), we set a timer to make it disappear after a while. But don't let it
// disappear if the actor is still selected.
void WebActor::flyoutTimeout()
{
	if (_flyout && !sel->isInSelection(this) || sel->getSize() != 1)
		fadeOutFlyout();
}


void WebActor::createFlyout()
{
	assert(_flyout == NULL);
	if (isSharingWidget())
	{
		_flyout = new SharingFlyout((SharedFolderAPI *)_javaScriptAPI, boundsToQRect(getScreenBoundingBox()));
		connect(_flyout, SIGNAL(done()), this, SLOT(deleteFlyout()));
	}
}

// Show the flyout. This is NOT meant to be used when the flyout is being
// displayed because the actor was selected.
void WebActor::showFlyout(QVariantMap data)
{
	if (!_flyout)
		createFlyout();

	if (_flyout)
	{
		_flyout->setData(&data);
		fadeInFlyout();
		QTimer::singleShot(FLYOUT_TIMEOUT_MS, this, SLOT(flyoutTimeout()));
	}
}

// fade flyout in, if it exists
void WebActor::fadeInFlyout()
{
	if (_flyout)
		_flyout->fadeIn();
}

// fade flyout out, if it exists
void WebActor::fadeOutFlyout()
{
	if (_flyout)
		_flyout->fadeOut();
}

// return whether the flyout exists and is visible
bool WebActor::isFlyoutVisible()
{	
	return _flyout && _flyout->isVisible();
}

// Delete flyout and return true. If there is no flyout, just return false
bool WebActor::deleteFlyout()
{
	if (_flyout && isSharingWidget()) {
		// cancel any uploads in progress
		_javaScriptAPI->deleteFlyout();
		
		SAFE_DELETE(_flyout);
		return true;
	}
	return false;
}

#endif
