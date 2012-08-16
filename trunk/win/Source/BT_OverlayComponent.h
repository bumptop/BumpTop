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

#pragma once

#ifndef _BT_OVERLAYCOMPONENT_
#define _BT_OVERLAYCOMPONENT_

// -----------------------------------------------------------------------------

#include "BT_Animatable.h"
#include "BT_AnimatedTextureSource.h"
#include "BT_AnimationEntry.h"
#include "BT_ColorVal.h"
#include "BT_FontManager.h"
#include "BT_OverlayEvent.h"
#include "BT_Stopwatch.h"
#include "BT_ThemeManager.h"
#include "TextPixmapBuffer.h"
#include "BackgroundPixmapBuffer.h"

// -----------------------------------------------------------------------------

// forward declarations
class OverlayComponent;
class OverlayLayout;
class Nameable;

// -----------------------------------------------------------------------------

	//
	// Free functions
	//
	void * RemoveLayoutItemAfterAnim(AnimationEntry * animEntry);
	void * HideComponentAfterAnim(AnimationEntry * animEntry);

	//
	// Edges enum
	//
	enum OverlayEdge
	{
		TopEdge = (1 << 0),
		RightEdge = (1 << 1),
		BottomEdge = (1 << 2),
		LeftEdge = (1 << 3),
		LeftRightEdges = (LeftEdge | RightEdge),
		TopBottomEdges = (TopEdge | BottomEdge),
		AllEdges = (LeftRightEdges | TopBottomEdges)
	};

	//
	// Corners enum
	// 
	enum OverlayCorner
	{
		TopLeftCorner = (1 << 0),
		TopRightCorner = (1 << 1),
		BottomRightCorner = (1 << 2),
		BottomLeftCorner = (1 << 3),
		TopCorners = (TopLeftCorner | TopRightCorner),
		BottomCorners = (BottomLeftCorner | BottomRightCorner),
		AllCorners = (TopCorners | BottomCorners)
	};


	/*
	 * Overlay Styles
	 *
	 * - Note that not all styles are implemented or used by every component and layout.  In particular, the
	 *	 floating, boundEdges, and position parameters are only used for free-layouts such as the base OverlayLayout,
	 *   and not tight-layouts such as the Vertical/Horizontal layouts. 
 	 */
	class OverlayStyle
	{
	public:
		enum OverlayOverflow
		{
			Hidden,
			Visible
		};
	
	private:
		float _padding[4];
		float _spacing;
		float _cornerRadius[4];
		ColorVal _color;
		ColorVal _backgroundColor;
		bool _visible;

		bool _floating;
		int _boundEdgesMask;
		int _componentEdgeAlignment;
		Vec3 _offset;
		Vec3 _scaledDimensions;		
		OverlayOverflow _overflowBehaviour;

		// border style, border width, etc.

	public:
		OverlayStyle();

		// visibility
		void setVisible(bool visible);
		bool isVisible() const;

		void setOverflow(OverlayOverflow behaviour);
		OverlayOverflow getOverflow() const;

		// floating / bound edges/alignment
		void setBoundEdges(int edgeBitMask);
		bool isBoundOnEdge(OverlayEdge edge) const;
		// NOTE: proper edge binding and layout will require significant
		//		 work, for now, we should just stick with preferred edge
		//		 alignment
		// NOTE: aligned edges must be orthogonal (ie. both L&R edges can
		//		 not be aligned)
		void setAlignedEdges(int orthogonalEdgeBitMask);
		bool isAlignedOnEdge(OverlayEdge edge) const;
		void setFloating(bool floating);
		bool isFloating() const;
		void setOffset(const Vec3& pos);
		const Vec3& getOffset() const;

		// colors
		void setColor(const ColorVal& col);
		const ColorVal& getColor() const;		
		void setAlpha(float alpha);
		float getAlpha() const;
		void setBackgroundColor(const ColorVal& col);
		const ColorVal& getBackgroundColor() const;

		// corner radius
		void setCornerRadius(int cornerMask, float value);
		float getCornerRadius(OverlayCorner corner);

		// box model
		void setPadding(int edgeMask, float value);
		float getPadding(OverlayEdge edge);		
		void setSpacing(float value);
		float getSpacing();

		// transform
		void setScaledDimensions(const Vec3& scaledDimensions);
		const Vec3& getScaledDimensions() const;
	};


	/*
	 * Overlay Component
	 * 
	 * - The base component for all overlays, which supports basic size/alpha animations, as well as
	 *   handling of events (such as timer, mouse, etc.)
	 */
	class OverlayComponent : public Animatable,
							 public MouseOverlayEventHandler
	{
		Vec3 _position;
		Vec3 _size;
		OverlayLayout * _parent;
		OverlayStyle _style;

		// Animation Frame Lists
		deque<Vec3> sizeAnim;
		deque<float> alphaAnim;

		// custom event callbacks
		vector<MouseOverlayEventHandler *> _mouseEventHandlers;
		vector<TimerOverlayEventHandler *> _timerEventHandlers;

		// background rendering
		BackgroundPixmapBuffer _background;
		bool _isBackgroundDirty;

	protected:
		OverlayComponent();

		virtual bool areAlphaAnimParametersValid(float startAlpha, float endAlpha, uint& animationSteps);
		void clipToRenderableArea(Bounds & boundsInOut, const Vec3 & maxUVs, const Bounds & renderingBounds, Vec3 & texPosOut, Vec3 & uvOut);

	public:
		virtual ~OverlayComponent();
		
		// parent
		void setParent(OverlayLayout * parent);
		OverlayLayout * getParent() const;

		// dimensions
		void setPosition(const Vec3& relativePos);
		const Vec3& getPosition();
		Vec3 getAbsolutePosition();
		virtual void setSize(Vec3 newSize);
		const Vec3& getSize();
		virtual Bounds getBounds();
		virtual Vec3 getPreferredDimensions() = 0;
		void markBackgroundAsDirty();

		// styles
		OverlayStyle& getStyle();
		BackgroundPixmapBuffer& getBackgroundBuffer();

		//
		virtual bool isReady() const;
		virtual bool isFinished() const;

		// event registration
		void addMouseEventHandler(MouseOverlayEventHandler * handler);
		void addTimerEventHandler(TimerOverlayEventHandler * handler);
		void removeMouseEventHandler(MouseOverlayEventHandler * handler);
		void removeTimerEventHandler(TimerOverlayEventHandler * handler);

		// events
#ifdef DXRENDER
		virtual void onRender(const Vec3 & offset, const Bounds & renderingBounds) {}
		virtual void onRenderBackground(const Vec3 & offset, const Bounds & renderingBounds);
		virtual void onRelease();
#else
		virtual void onRender() {}
		virtual void onRenderBackground();
#endif
		virtual bool onTimer(TimerOverlayEvent& timerEvent);
		virtual void onSize(const Vec3& newSize) {}

		// mouse events
		virtual bool onMouseDown(MouseOverlayEvent& mouseEvent);
		virtual bool onMouseUp(MouseOverlayEvent& mouseEvent);
		virtual bool onMouseMove(MouseOverlayEvent& mouseEvent);

		// animatable
		virtual void onAnimTick();
		virtual void killAnimation();
		virtual void finishAnimation();
		virtual bool isAnimating(uint animType = SizeAnim | AlphaAnim | PoseAnim);
		void setSizeAnim(Vec3 &startSize, Vec3 &lastSize, uint steps, FinishedCallBack func = NULL, void *customData = NULL);
		void setSizeAnim(deque<Vec3> &customSizeAnim, FinishedCallBack func = NULL, void *customData = NULL);
		void setAlphaAnim(float startAlpha, float endAlpha, uint steps, FinishedCallBack func = NULL, void *customData = NULL);
		virtual void setAlpha(float alpha);
	};

	/*
	 * Overlay Layout
	 *
	 * - The base overlay layout which positions and sizes items to fit it's specified bounds.
	 *   It allows users to manage a collection of components without having to deal with each
	 *   component menually.
	 */
	class OverlayLayout : public OverlayComponent
	{
		// private vars
		vector<OverlayComponent *> _components;
		// mouse button tracking
		OverlayComponent * _mouseDownComponent;
		int	_mouseDownButton;

	protected:
		virtual bool areAlphaAnimParametersValid(float startAlpha, float endAlpha, uint& animationSteps);
		bool intersects(const Vec3& pos, const Bounds& bounds);

	public:
		OverlayLayout();
		virtual ~OverlayLayout();

		// contained items
		virtual void addItem(OverlayComponent * component);
		virtual void insertItem(OverlayComponent * component, int index);
		virtual void removeItem(OverlayComponent * component);
		virtual void deleteItem(OverlayComponent * component);
		void clearItems();
		const vector<OverlayComponent *>& items() const;
		virtual void setSize(Vec3 newSize);
		virtual void reLayout();

		//
		virtual bool isReady() const;

		// properties / styles
		virtual Vec3 getPreferredDimensions();

		// events
#ifdef DXRENDER
		virtual void onRender(const Vec3 & offset, const Bounds & renderingBounds);
		virtual void onRelease();
#else
		virtual void onRender();
#endif
		virtual bool onTimer(TimerOverlayEvent& timerEvent);
		virtual void onSize(const Vec3& newSize);
		virtual void killAnimation();
		virtual void finishAnimation();

		// input events
		virtual bool onMouseDown(MouseOverlayEvent& mouseEvent);
		virtual bool onMouseUp(MouseOverlayEvent& mouseEvent);
		virtual bool onMouseMove(MouseOverlayEvent& mouseEvent);

		// animatable
		virtual void setAlpha(float alpha);
	};


	/*
	 * Horizontal Overlay Layout
	 * 
	 * - A derivative of the free overlay layout, which lines items up horizontally.
	 */
	class HorizontalOverlayLayout : public OverlayLayout
	{
	public:
		HorizontalOverlayLayout();
		virtual ~HorizontalOverlayLayout();

		// overlay
		virtual Vec3 getPreferredDimensions();
		virtual void setSize(Vec3 newSize);
		virtual void reLayout();
	};


	/*
	 * Vertical Overlay Layout
	 * 
	 * - A derivative of the free overlay layout, which lines items up vertically.
	 */
	class VerticalOverlayLayout : public OverlayLayout
	{
	public:
		VerticalOverlayLayout();
		virtual ~VerticalOverlayLayout();

		// overlay
		virtual Vec3 getPreferredDimensions();
		virtual void setSize(Vec3 newSize);
		virtual void reLayout();
	};


	/*
	 * Text Overlay
	 * 
	 * - Represents a piece of rendered text, multiple pieces of which can be appended
	 *   to a layout to create lines of various styles.
	 */
	class TextOverlay : public OverlayComponent
	{
		Vec3 _prefDims;
		FontDescription _font;
		TextPixmapBuffer _textBuffer;

	private:
		void syncTextBounds();

	public:
		TextOverlay();
		TextOverlay(QString text, bool truncate = false);
		virtual ~TextOverlay();
		void init(QString text, bool truncate);

		// message
		void setText(QString text, bool truncate=false);
		QString getText() const;

		void enableTextShadows();
		void disableTextShadows();

		bool setFont(const FontDescription& font);
		const FontDescription& getFont() const;

		// overlay
		virtual Vec3 getPreferredDimensions();
#ifdef DXRENDER
		virtual void onRender(const Vec3 & offset, const Bounds & renderingBounds);
		virtual void onRelease();
#else
		virtual void onRender();
#endif

		TextPixmapBuffer& getTextBuffer();
	};


	enum TextureNameType
	{
		TEXTURE_ID,
		TEXTURE_ANIMATED_GIF_PATH
	};
	/*
	 * Image Overlay
	 * 
	 * - Represents a rendered image, which can be attached to various layouts.
	 */
	class ImageOverlay : public OverlayComponent, public TimerOverlayEventHandler
	{
		Vec3 _prefDims;
		Vec3 _texDims;
		QString _textureId;
		TextureNameType _textureType;
		AnimatedTextureSource _animatedTextureSource;

		bool _active;
		bool _reloadDimsLater;

	public:
		ImageOverlay(QString textureName, TextureNameType textureType = TEXTURE_ID);
		virtual ~ImageOverlay();

		// operations
		void setImage(QString textureId, TextureNameType textureType = TEXTURE_ID);

		// overlay
		virtual bool isReady() const;
		virtual Vec3 getPreferredDimensions();
#ifdef DXRENDER
		virtual void onRender(const Vec3 & offset, const Bounds & renderingBounds);
#else
		virtual void onRender();
#endif
		void setActive(bool active);

		virtual bool onTimer(TimerOverlayEvent& timerEvent);
	};


	/* 
	 * Message removal policy
	 *
	 * - Represents the way in which messages are removed from the visible view
	 */
	class MessageClearPolicy : public MouseOverlayEventHandler,
							   public TimerOverlayEventHandler								
	{
	public:
		enum MessageClearType
		{
			Timer		= (1 << 0),
			MouseDown	= (1 << 1),
			MouseUp		= (1 << 2),
			MouseClick	= MouseUp,
			MouseMove	= (1 << 3),
			KillFocus	= (1 << 4)		// occurs when a new message arrives in the message container?
		};

	private:
		unsigned int _activeType;
		unsigned int _setType;
		StopwatchInSeconds _timer;
		unsigned int _duration;
		float _deadzone;
		Vec3 _initialPos;

	public:
		MessageClearPolicy(unsigned int removalTypeMask=(MessageClearPolicy::MouseClick | MessageClearPolicy::Timer));
		~MessageClearPolicy();

		bool operator==(const MessageClearPolicy& policy);

		//
		void reset();

		// accessors
		void setType(unsigned int removalTypeMask);
		unsigned int getType() const;

		// timer related functions
		void setTimeout(unsigned int duration);
		unsigned int getTimeout() const;
		
		// mouse move 
		void setInitialPoint(const Vec3& absoluteScreenPos);
		void setDeadzone(float deadzone);
		float getDeadzone() const;

		// event handlers
		// - if the policy handles the event, then the message should be removed
		virtual bool onTimer(TimerOverlayEvent& timerEvent);
		virtual bool onMouseDown(MouseOverlayEvent& mouseEvent);
		virtual bool onMouseMove(MouseOverlayEvent& mouseEvent);
		virtual bool onMouseUp(MouseOverlayEvent& mouseEvent);
	};

	
	/*
	 * Message Overlay
	 * 
	 * - Represents a message box message.  Has the ability to time-out, and respond to clicks
	 *   to close itself.
	 */
	class Message : public HorizontalOverlayLayout,
					public ThemeEventHandler
	{
		Q_DECLARE_TR_FUNCTIONS(Message)

	public:
		enum MessageType
		{
			Ok =		(1 << 0),
			Warning =	(1 << 1),
			Error =		(1 << 2),
			Dot =		(1 << 3),
			Alert =		(1 << 4),

			Sticky =	(1 << 9),
			Custom =	(1 << 10),
			ForceTruncate = (1 << 11)
		};

	private:
		weak_ptr<ThemeManager> themeManagerRef;

		QString _key;
		TextOverlay * _text;
		int _messageTypeMask;
		MessageClearPolicy _policy;

	public:
		Message(QString key, QString message, int messageTypeMask, const MessageClearPolicy& policy=MessageClearPolicy());
		virtual ~Message();

		// operator overloads
		bool operator==(const Message& msg);

		// message
		QString getKey() const;
		TextOverlay * getText() const;
		void fadeAway();

		// overlay overloads
		virtual bool onTimer(TimerOverlayEvent& timerEvent);
		virtual bool onMouseDown(MouseOverlayEvent& mouseEvent);
		virtual bool onMouseMove(MouseOverlayEvent& mouseEvent);
		virtual bool onMouseUp(MouseOverlayEvent& mouseEvent);
		virtual void onThemeChanged();
	};


	/*
	 * Message Overlay Container
	 * 
	 * - Manages a series of messages, aligning them vertically on the screen
	 */
	class MessageContainer : public VerticalOverlayLayout,
							 public ThemeEventHandler
	{
		weak_ptr<ThemeManager> themeManagerRef;

	public:
		MessageContainer();
		virtual ~MessageContainer();
		
		// messages
		void addMessage(Message * message);
		bool hasMessage(QString messageKey);
		Message * getMessage(QString messageKey);
		void dismissMessage(QString messageKey);
		void clearMessages();

		// overrides
		virtual bool onTimer(TimerOverlayEvent& timerEvent);
		virtual void onThemeChanged();
	};


	/*
	 * Actor Name Icon Overlay
	 * 
	 * - Represents an actor's name's icon.
	 */
	class NameableIconOverlay : public ImageOverlay
	{
		Nameable *	_nameable;
		bool		_isFirstClick;
		StopwatchInSeconds		_clickTimer;

	public:
		NameableIconOverlay(QString textureName, Nameable * nameable);

		// image overlay overrides
		virtual bool onMouseDown(MouseOverlayEvent& mouseEvent);
		virtual bool onMouseUp(MouseOverlayEvent& mouseEvent);
	};


	/* 
	 * Actor Name Overlay
	 * 
	 * - Represents an actor's name.
	 */
	class NameableOverlay : public HorizontalOverlayLayout,
							public ThemeEventHandler
	{
		weak_ptr<ThemeManager> themeManagerRef;

		TextOverlay * _textOverlay;
		Nameable * _nameable;
		bool _isSelected;

		ColorVal _backgroundColor;
		ColorVal _selectedBackgroundColor;

	public:
		NameableOverlay(Nameable * nameable);
		virtual ~NameableOverlay();
		
		// update
		bool updateTextFromNameable();
		void markSelected(bool selected);

		// getters
		Nameable * getNameable();
		TextOverlay * getTextOverlay();
		int getDefaultTextWidth();

		// events
		virtual void onThemeChanged();
#ifdef DXRENDER
		virtual void onRender(const Vec3 & offset, const Bounds & renderingBounds);
#else
		virtual void onRender();
#endif
	};


	/*
	 * Absolute Overlay Layout 
	 * 
	 * - This is a top level layout which lays all components out
	 *   by their absolute offsets.
	 */
	class AbsoluteOverlayLayout : public OverlayLayout
	{
	public:
		AbsoluteOverlayLayout();
		virtual ~AbsoluteOverlayLayout();

		// overlay
		virtual Vec3 getPreferredDimensions();
		virtual void reLayout();
	};

	/*
	* Namable Overlay Layout
	* 
	* - This functions much like the AboluteOverlayLayout but has some 
	*   deterministic logic to fish out text that is obscured
	*/

	class NamableOverlayLayout : public AbsoluteOverlayLayout
	{
		QSet<NameableOverlay *> invisibleItems;

	public:
		NamableOverlayLayout();
		virtual ~NamableOverlayLayout();

		virtual void reLayout();

		void determineVisibility();
#ifdef DXRENDER
		void onRender(const Vec3 & offset, const Bounds & renderingBounds);
#else
		void onRender();
#endif
		bool isInvisible(NameableOverlay * obj);
	};

#else
	enum OverlayEdge;
	class OverlayComponent;
	class OverlayLayout;
	class VerticalOverlayLayout;
	class HorizontalOverlayLayout;
	class TextOverlay;
	class ImageOverlay;
	class Message;
	class MessageContainer;
#endif