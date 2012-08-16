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

#ifndef BT_WEBACTOR
#define BT_WEBACTOR

#ifdef ENABLE_WEBKIT

#include "BT_Actor.h"
#include "BT_WebPage.h"
#include "BT_Stopwatch.h"

class PerformanceCounter;
class PbBumpObject;
class Flyout;

class WebActor : public QObject, 
				 public Actor
{
	Q_OBJECT
	bool _focused; // Must only be changed by setFocused(), which updates dirty flag
	bool _textureDirty;
	QSize _contentSize, _fullBufferSize, _smallBufferSize;
#ifdef DXRENDER
	IDirect3DTexture9* _glTextureId;
	IDirect3DTexture9* _glSpinnerTextureId;
#else
	unsigned int _glTextureId, _glSpinnerTextureId;
#endif
	int _spinnerAngle; // 1/16 deg, used to animate spinner while page is loading

	WebPage _page;
	Stopwatch _updateTimer;

	QString _pageData;
	bool _isPageDataURL;
	QDir _facebookWidgetDir;

	QMouseEvent * _prevMouseDown, * _prevMouseUp;
	PerformanceCounter * _singleClickTimer;	// Stopwatch does not have the right behaviour.

	JavaScriptAPI * _javaScriptAPI;

	bool _isDropSourceValid;

	friend void Key_Test();
	class VideoRender * _videoRender;
	class Mesh * _mesh;

	// auto-updating the page
	QTimer * _autoUpdateTimer;
	int	_autoUpdateTimerDelay;
	bool _restartAutoUpdateTimerOnUnfocus;

	Flyout *_flyout;

#ifdef DEBUG
	void processHtmlSources(); 
#endif

private slots:
	void flyoutTimeout();

public:
	WebActor();
	virtual ~WebActor();
	
	void load(const QString& url, bool executeQueuedCodeOnThisPage = false);
	void loadHTML(const QString& html);
	void reload();

	JavaScriptAPI* getJavaScriptAPI();

	static bool canMakeMore();

	// overrides
	const QString& getPageData() const;
	virtual Vec3 getDefaultDims();
#ifdef DXRENDER
	virtual IDirect3DTexture9 * getTextureNum();
	IDirect3DVertexBuffer9* generateVertexBuffer();
#else
	virtual uint getTextureNum();
#endif
	QWidget * getViewWidget();
	bool isFocused() const;
	bool isAnimationDisabled();
	bool isUpdatingDisabled();
	void setFocused(bool focused);
	void setIsExternal(bool isExternal);

	void mapLocalMouseCoords(int screenX, int screenY, int& localXOut, int& localYOut);

	// Implement DropObject
	virtual bool isValidDropTarget();
	virtual QString resolveDropOperationString(vector<BumpObject *>& objList);
	virtual void onDropEnter(vector<BumpObject *> &objList);
	virtual bool isSourceValid();
	virtual vector<BumpObject *> onDrop(vector<BumpObject *> &objList);
	virtual void onDropExit();

	// event handling
	
	void updateExpectedSize(); 
	void updateAspectRatio(const QSize & contentSize, bool animate);
	bool hasEventOverride() const;
	bool wantsMouseEvent(QMouseEvent * evt);
	bool onMouseEvent(QMouseEvent * evt); // Returns true if event is handled; deletes event.
	bool onWheelEvent(QWheelEvent * evt); // Returns true if event is handled; deletes event.
	bool onKeyEvent(QKeyEvent * evt); // Returns true if event is handled; deletes event.
	void onUpdate();
	virtual void onDragBegin(FinishedDragCallBack func = NULL);
	virtual void onLaunch();
	virtual void onSelect();
	virtual void onDeselect();
	virtual void onRender(uint flags = RenderSideless);
	virtual void onClick();
#ifdef DXRENDER
	virtual void onRelease();
#endif
	void updateTexture();

	bool isSharingWidget();
	bool isFacebookWidgetUrl();
	static bool hasFocusedWebActor();
	static bool zoomOutFocusedWebActor();

	void createFlyout();
	void showFlyout(QVariantMap data);
	bool isFlyoutVisible();

	// protocol buffers
	virtual bool serializeToPb(PbBumpObject * pbObject);
	virtual bool deserializeFromPb(const PbBumpObject * pbObject);

public slots:
	void pageUpdated(const QImage&, const QRect&);
	void pageReloaded(bool);
	void autoUpdateReloadPage();
	bool deleteFlyout();
	void fadeInFlyout();
	void fadeOutFlyout();
};

#endif
#endif
