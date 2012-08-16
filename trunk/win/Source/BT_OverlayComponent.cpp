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
#include "BT_AnimationManager.h"
#include "BT_Camera.h"
#include "BT_ColorVal.h"
#include "BT_FileSystemManager.h"
#include "BT_FileSystemPile.h"
#include "BT_FontManager.h"
#include "BT_GLTextureManager.h"
#include "BT_Logger.h"
#include "BT_Nameable.h"
#ifdef DXRENDER
	#include "BT_DXRender.h"
#endif
#include "BT_OverlayComponent.h"
#include "BT_Pile.h"
#include "BT_RenderManager.h"
#include "BT_SceneManager.h"
#include "BT_TextManager.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"
#include "TextPixmapBuffer.h"


const int DEFAULT_TEXTOVERLAY_CACHESIZE = 4;
//
//
//
void * RemoveLayoutItemAfterAnim( AnimationEntry * animEntry )
{
	OverlayLayout * layout = (OverlayLayout *) animEntry->getCustomData();
	OverlayComponent * item = (OverlayComponent *) animEntry->getObject();
	layout->deleteItem(item);
	return NULL;
}

void * HideComponentAfterAnim( AnimationEntry * animEntry )
{
	OverlayComponent * component = (OverlayComponent *) animEntry->getObject();
	component->getStyle().setVisible(false);
	component->getStyle().setAlpha(1.0f);
	return NULL;
}


//
// OverlayStyle implementation
//
OverlayStyle::OverlayStyle()
: _color(255,255,255,255)
, _backgroundColor(0,0,0,0)
, _spacing(0.0f)
, _visible(true)
, _floating(false)
, _boundEdgesMask(0)
, _componentEdgeAlignment(0)
, _overflowBehaviour(Visible)
{
	_padding[0] = _padding[1] = _padding[2] = _padding[3] = 0.0f;
	_padding[1] = _padding[3] = 2.0f;
	_padding[2] = 1.0f;
	_cornerRadius[0] = _cornerRadius[2] = _cornerRadius[1] = _cornerRadius[3] = 8.0f;
	_offset.x = _offset.y = _offset.z = -1;
	_scaledDimensions.setNotUsed();
}

void OverlayStyle::setColor( const ColorVal& col )
{
	_color = col;
}

const ColorVal& OverlayStyle::getColor() const
{
	return _color;
}

void OverlayStyle::setAlpha( float alpha )
{
	_color.bigEndian.a = int(alpha * 255.0f);
}

float OverlayStyle::getAlpha() const
{
	return (float(_color.bigEndian.a) / 255.0f);
}

void OverlayStyle::setBackgroundColor( const ColorVal& col )
{
	_backgroundColor = col;
}

const ColorVal& OverlayStyle::getBackgroundColor() const
{
	return _backgroundColor;
}

void OverlayStyle::setPadding( int edgeMask, float value )
{
	for (int i = 0; i < 4; ++i)
	{
		if (edgeMask & (1 << i))
		{
			_padding[i] = value;
		}
	}
}

float OverlayStyle::getPadding( OverlayEdge edge )
{
	float totalPadding = 0.0f;
	for (int i = 0; i < 4; ++i)
	{
		if (edge & (1 << i))
		{
			totalPadding += _padding[i];
		}
	}
	return totalPadding;
}

void OverlayStyle::setCornerRadius( int cornerMask, float value )
{
	for (int i = 0; i < 4; ++i)
	{
		if (cornerMask & (1 << i))
		{
			_cornerRadius[i] = value;
		}
	}
}

float OverlayStyle::getCornerRadius( OverlayCorner corner )
{
	float maxRadius = 0.0f;
	for (int i = 0; i < 4; ++i)
	{
		if (corner & (1 << i))
		{
			maxRadius = NxMath::max(maxRadius, _cornerRadius[i]);
		}
	}
	return maxRadius;
}

void OverlayStyle::setSpacing( float value )
{
	_spacing = value;
}

float OverlayStyle::getSpacing()
{
	return _spacing;
}

void OverlayStyle::setVisible( bool visible )
{
	_visible = visible;
}

bool OverlayStyle::isVisible() const
{
	return _visible;
}

void OverlayStyle::setOverflow(OverlayOverflow behaviour)
{
	_overflowBehaviour = behaviour;
}

OverlayStyle::OverlayOverflow OverlayStyle::getOverflow() const
{
	return _overflowBehaviour;
}

void OverlayStyle::setFloating( bool floating )
{
	_floating = floating;
}

bool OverlayStyle::isFloating() const
{
	return _floating;
}

void OverlayStyle::setBoundEdges( int edgeBitMask )
{
	_boundEdgesMask = edgeBitMask;
}

bool OverlayStyle::isBoundOnEdge( OverlayEdge edge ) const
{
	return (_boundEdgesMask & edge) > 0;
}

void OverlayStyle::setAlignedEdges(int orthogonalEdgeBitMask)
{
	_componentEdgeAlignment = orthogonalEdgeBitMask;
}

bool OverlayStyle::isAlignedOnEdge(OverlayEdge edge) const
{
	return (_componentEdgeAlignment & edge) > 0;
}

void OverlayStyle::setOffset( const Vec3& pos )
{
	_offset = pos;

	// NOTE: once positions are enabled, we set floating to true
	_floating = true;
}

const Vec3& OverlayStyle::getOffset() const
{
	return _offset;
}

void OverlayStyle::setScaledDimensions(const Vec3& scaledDimensions)
{
	_scaledDimensions = scaledDimensions;
}

const Vec3& OverlayStyle::getScaledDimensions() const
{
	return _scaledDimensions;
}


//
// OverlayComponent implementation
//
OverlayComponent::OverlayComponent()
: _parent(0)
, _isBackgroundDirty(true)
{
	_position.zero();
	_size.zero();
}

OverlayComponent::~OverlayComponent()
{
}

Vec3 OverlayComponent::getAbsolutePosition()
{
	Vec3 pos = getPosition();
	OverlayComponent * parent = getParent();
	while (parent)
	{
		pos += parent->getPosition();
		parent = parent->getParent();
	}
	pos.y = winOS->GetWindowHeight() - pos.y;
	return pos;
}

Bounds OverlayComponent::getBounds()
{
	Vec3 pos = getPosition();
	Vec3 dims = getSize();
	Bounds bounds;
	bounds.set(pos, pos + dims);
	return bounds;
}

void OverlayComponent::setPosition( const Vec3& relativePos )
{
	_position = relativePos;
}

const Vec3& OverlayComponent::getPosition()
{
	return _position;
}

void OverlayComponent::setSize( Vec3 newSize )
{
	// reposition the component centered on the new size
	Vec3 diffSize = (newSize - _size) / 2.0f;
	Vec3 newPos = getPosition();
	if (newSize.x < winOS->GetWindowWidth())
		newPos.x += diffSize.x;
	if (newSize.y < winOS->GetWindowHeight())
		newPos.y -= diffSize.y;
	setPosition(newPos);

	// set the size
	_size = newSize;
	
	// relayout this parent
	if (getParent()) getParent()->reLayout();
	
	markBackgroundAsDirty();
}

const Vec3& OverlayComponent::getSize()
{
	return _size;
}

void OverlayComponent::setParent( OverlayLayout * parent )
{
	// create a new scene node with the parent
	static int i = 0;

	// set new parent
	_parent = parent;
}

OverlayLayout * OverlayComponent::getParent() const
{
	return _parent;
}

bool OverlayComponent::isReady() const
{
	return true;
}

bool OverlayComponent::isFinished() const
{
	return false;
}

OverlayStyle& OverlayComponent::getStyle()
{
	return _style;
}

BackgroundPixmapBuffer& OverlayComponent::getBackgroundBuffer()
{
	return _background;
}

void OverlayComponent::onAnimTick()
{
	Vec3 size;

	if (alphaAnim.size() > 0)
	{
		// Animate Alpha
		setAlpha(alphaAnim.front());
		alphaAnim.pop_front();
	}

	if (sizeAnim.size() > 0)
	{
		size = sizeAnim.front();
		sizeAnim.pop_front();

		// Animate Sizes
		// directly set the size
		setSize(size);
	}
}

void OverlayComponent::killAnimation()
{
	alphaAnim.clear();
	sizeAnim.clear();

	// Remove ourself from the animation queue
	animManager->removeAnimation(this);

	setAnimationState(NoAnim);
}

void OverlayComponent::finishAnimation()
{
	// Jump to the end of each animation
	if (!alphaAnim.empty()) getStyle().setAlpha(alphaAnim.back());
	if (!sizeAnim.empty()) OverlayComponent::setSize(sizeAnim.back());

	killAnimation();
}

void OverlayComponent::setSizeAnim(Vec3 &startSize, Vec3 &endSize, uint steps, FinishedCallBack func, void *customData)
{
	sizeAnim.clear();

	// Create a nice Bounce animation
	sizeAnim = lerpRange(startSize, endSize, steps, NoEase);
	deque<Vec3>::iterator iter = sizeAnim.begin();
	while (iter != sizeAnim.end())
	{
		roundOffDecimals(*iter);
		iter++;
	}

	// Add to the animation manager
	animManager->addAnimation(AnimationEntry(this, func, customData));
}

void OverlayComponent::setSizeAnim(deque<Vec3> &customSizeAnim, FinishedCallBack func, void *customData)
{
	sizeAnim = customSizeAnim;
	animManager->addAnimation(AnimationEntry(this, func, customData));
}

bool OverlayComponent::areAlphaAnimParametersValid(float startAlpha, float endAlpha, uint& animationSteps)
{
	if (qFuzzyCompare(startAlpha, endAlpha))
	{
		// if the end alpha is the same as the current, break, otherwise, just continue with the animation
		if (qFuzzyCompare(endAlpha, getStyle().getAlpha()))
			return false;

		// minimize the number of animation steps since the start and end alpha are the same, we still do 
		// this due to potential animation callbacks which might need to be executed
		animationSteps = 1;
	}
	return true;
}

void OverlayComponent::setAlphaAnim(float startAlpha, float endAlpha, uint steps, FinishedCallBack func, void *customData)
{
	// Added because when adding an overlay component to a layout, the layout gives the component an alpha animation
	// from 0 to 0. Sometimes, this animation unwantedly overrides the components alpha.
	if (!areAlphaAnimParametersValid(startAlpha, endAlpha, steps))
		return;

	float tickAmount = (endAlpha - startAlpha) / steps;

	// Clear the alpha animation if there is one
	alphaAnim.clear();

	for (uint i = 0; i < steps - 1; i++)
	{
		alphaAnim.push_back(startAlpha + (tickAmount * i));
	}

	// Last amount is the max amount for this run
	alphaAnim.push_back(endAlpha);
	AnimationEntry alphaAnimEntry(this, func, customData);
	alphaAnimEntry.setIgnoreBattery(true);
	animManager->addAnimation(alphaAnimEntry);
}

bool OverlayComponent::isAnimating( uint animType /*= SizeAnim | AlphaAnim | PoseAnim*/ )
{
	// Check the different types of anims
	if (animType & AlphaAnim && alphaAnim.size() > 0) return true;
	if (animType & SizeAnim && sizeAnim.size() > 0) return true;

	return false;
}

void OverlayComponent::setAlpha( float alpha )
{
	getStyle().setAlpha(alpha);
}

/*	This method takes a 2D bounding box (A) and clips it against another 2D bounding box (B).
	Then end result is a clipped bounding box, if the first box originally intersected
	the second box. This is method is special, however, since it recalculates texture
	coordinates.

	|----------X                                     |----------|
	|          |		                             |          |
    |      |----------|     clipToRenderableArea()   |      |---|
	|   B  |    A     |    ----------------------->  |   B  | A |
	|      |----------|                              |      |---|
	|		   |                                     |          |
	Y----------|                                     |----------|
*/
void OverlayComponent::clipToRenderableArea(Bounds & boundsInOut, const Vec3 & maxUVs, const Bounds & renderingBounds, Vec3 & texPosOut, Vec3 & uvOut)
{
	Vec3 position = boundsInOut.getMin();
	Vec3 size = boundsInOut.getMax() - position;

	float top = position.y + size.y;
	float bottom = position.y;
	float left = position.x;
	float right = position.x + size.x;
	
	const Vec3& boundsMax = renderingBounds.getMax(); // X: In diagram ^
	const Vec3& boundsMin = renderingBounds.getMin(); // Y: In diagram ^
	
	// Calculates proper bounds, ex: For the top, picks the smallest out of
	// the top y value of the box to clip (A) and the rendering bounds (B)
	top = NxMath::min(top, boundsMax.y);
	bottom = NxMath::max(bottom, boundsMin.y);
	left = NxMath::max(left, boundsMin.x);
	right = NxMath::min(right, boundsMax.x);
	
	float width = right - left;
	float height = top - bottom;
	
	// Finds the ratio between the new size and the old size of the box (A)
	// and scales the texture coordinates accordingly
	uvOut.x = (width / size.x) * maxUVs.x;
	uvOut.y = (height / size.y) * maxUVs.y;
	
	//       fix comment and explain 0.4 (left over from size) + change in offset / old width * 1.0
	texPosOut.x = maxUVs.x - (uvOut.x + ((left - position.x) / size.x) * maxUVs.x);
	texPosOut.y = maxUVs.y - (uvOut.y + ((bottom - position.y) / size.y) * maxUVs.y);

	boundsInOut.set(left, bottom, 0.0f, right, top, 0.0f);	
}

#ifdef DXRENDER
void OverlayComponent::onRenderBackground(const Vec3 & offset, const Bounds & renderingBounds)
#else
void OverlayComponent::onRenderBackground()
#endif
{
	// skip if we are not visible
	if (!getStyle().isVisible())
		return;

	// skip if the background has no alpha
	if (!getStyle().getBackgroundColor().bigEndian.a)
		return;

	const Vec3& maxDims = getSize();

	if (_isBackgroundDirty)
	{
		// update the background
		_background.setSize(QSize(maxDims.x, maxDims.y));
		if (_background.hasFlag(BackgroundPixmapBuffer::SolidColor))
		{
			const ColorVal& bgCol = getStyle().getBackgroundColor();
			_background.setColor(QColor(bgCol.bigEndian.r, bgCol.bigEndian.g, bgCol.bigEndian.b));
		}
		_background.setCornerRadius(getStyle().getCornerRadius(TopLeftCorner));
		_background.update();
		_isBackgroundDirty = false;
	}

	float alpha = getStyle().getAlpha();
	if (_background.hasFlag(BackgroundPixmapBuffer::SolidColor))
		alpha = NxMath::min(getStyle().getBackgroundColor().bigEndian.a / 255.0f, getStyle().getAlpha());

	const QSize& textSize = _background.getActualSize();
	
	Bounds backgroundBounds;
	backgroundBounds.set(offset.x, offset.y, 0.0f, offset.x + (float)textSize.width(), offset.y + (float)textSize.height(), 0.0f);
	
	// Check if we should even render any of this
	if (!backgroundBounds.intersects(renderingBounds))
		return;
		
	Vec3 maxUVs;
	_background.bindAsGLTexture(maxUVs);
	
	// Trim the background so we only render what is visible (i.e: what is in the 
	// rendering bounds)
	Vec3 textureOffset;
	Vec3 textureUVs;
	clipToRenderableArea(backgroundBounds, maxUVs, renderingBounds, textureOffset, textureUVs);
	
	// getMin returns the bottom left corner of the bounds, which is, consequently,
	// the offset
	Vec3 finalOffset = backgroundBounds.getMin();
	Vec3 finalSize = backgroundBounds.getMax() - finalOffset;

	// render the background	
#ifdef DXRENDER
	dxr->renderBillboard(roundOffDecimals(finalOffset),
						 roundOffDecimals(finalSize),
						 D3DXCOLOR(1,1,1,alpha),
						 textureOffset,
						 textureUVs);
#else
	const QImage& bufferSize = _background.getBuffer();
	float xOffset = (0.5f / bufferSize.width());
	float yOffset = (0.5f / bufferSize.height());

	glPushMatrix();
	glPushAttribToken token(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glColor4f(1.0f, 1.0f, 1.0f, alpha);
		
		glTranslatef(0, textSize.height() - bufferSize.height(), 0);
		glBegin(GL_QUADS);
			glTexCoord2f(1,0); 	glVertex2f(bufferSize.width() + xOffset,bufferSize.height() + yOffset);
			glTexCoord2f(0,0); 	glVertex2f(xOffset,bufferSize.height() + yOffset);
			glTexCoord2f(0,1); 	glVertex2f(xOffset,yOffset);
			glTexCoord2f(1,1);	glVertex2f(bufferSize.width() + xOffset,yOffset);
		glEnd();
	glPopMatrix();
#endif
}

#ifdef DXRENDER
void OverlayComponent::onRelease()
{
	_background.onRelease();
}
#endif

bool OverlayComponent::onTimer( TimerOverlayEvent& timerEvent )
{
	// pass through to the callbacks first
	for (int i = 0; i < _timerEventHandlers.size(); ++i)
		_timerEventHandlers[i]->onTimer(timerEvent);
	return false;
}

bool OverlayComponent::onMouseDown( MouseOverlayEvent& mouseEvent )
{
	// ensure that we are a valid control first
	if (!(getStyle().isVisible() && (getStyle().getAlpha() > 0.0f)))
		return false;

	// pass to the custom event handlers
	bool handledByCustomHandler = false;
	for (int i = 0; i < _mouseEventHandlers.size() && !handledByCustomHandler; ++i)
	{
		handledByCustomHandler = _mouseEventHandlers[i]->onMouseDown(mouseEvent);
	}

	return handledByCustomHandler;
}

bool OverlayComponent::onMouseUp( MouseOverlayEvent& mouseEvent )
{
	// pass to the custom event handlers
	bool handledByCustomHandler = false;
	for (int i = 0; i < _mouseEventHandlers.size() && !handledByCustomHandler; ++i)
	{
		handledByCustomHandler = _mouseEventHandlers[i]->onMouseUp(mouseEvent);
	}

	return handledByCustomHandler;
}

bool OverlayComponent::onMouseMove( MouseOverlayEvent& mouseEvent )
{
	// pass to the custom event handlers
	bool handledByCustomHandler = false;
	for (int i = 0; i < _mouseEventHandlers.size() && !handledByCustomHandler; ++i)
	{
		handledByCustomHandler = _mouseEventHandlers[i]->onMouseMove(mouseEvent);
	}
	
	return handledByCustomHandler;
}

void OverlayComponent::addMouseEventHandler( MouseOverlayEventHandler * handler )
{
	vector<MouseOverlayEventHandler *>::iterator iter = 
		find(_mouseEventHandlers.begin(), _mouseEventHandlers.end(), handler);
	if (iter == _mouseEventHandlers.end())
		_mouseEventHandlers.push_back(handler);
}

void OverlayComponent::addTimerEventHandler( TimerOverlayEventHandler * handler )
{
	vector<TimerOverlayEventHandler *>::iterator iter = 
		find(_timerEventHandlers.begin(), _timerEventHandlers.end(), handler);
	if (iter == _timerEventHandlers.end())
		_timerEventHandlers.push_back(handler);
}

void OverlayComponent::removeMouseEventHandler( MouseOverlayEventHandler * handler )
{
	vector<MouseOverlayEventHandler *>::iterator iter = 
		find(_mouseEventHandlers.begin(), _mouseEventHandlers.end(), handler);
	if (iter != _mouseEventHandlers.end())
		_mouseEventHandlers.erase(iter);
}

void OverlayComponent::removeTimerEventHandler( TimerOverlayEventHandler * handler )
{
	vector<TimerOverlayEventHandler *>::iterator iter = 
		find(_timerEventHandlers.begin(), _timerEventHandlers.end(), handler);
	if (iter != _timerEventHandlers.end())
		_timerEventHandlers.erase(iter);
}

void OverlayComponent::markBackgroundAsDirty()
{
	_isBackgroundDirty = true;
}


//
// OverlayLayout implementation
// 
OverlayLayout::OverlayLayout()
: OverlayComponent()
, _mouseDownComponent(0)
, _mouseDownButton(0)
{
	/* XXX: DEBUG
	getStyle().setBackgroundColor(ColorVal(60, 200, 25, 25));
	*/
}

OverlayLayout::~OverlayLayout()
{
	clearItems();
}

void OverlayLayout::addItem( OverlayComponent * component)
{
	// ensure the component does not yet exist in the overlay
	for (int i = 0; i < _components.size(); ++i)
	{
		if (_components[i] == component)
		{
			return;
		}
	}

	// add the new component
	component->setParent(this);
	component->setAlphaAnim(0.0f, NxMath::min(component->getStyle().getAlpha(), getStyle().getAlpha()), 25);
	_components.push_back(component);

	// XXX: animate to the new pref bounds
	setSize(getPreferredDimensions());

	// make this component visible if it is not already
	if (items().size() > 0)
		getStyle().setVisible(true);
}

void OverlayLayout::insertItem(OverlayComponent * component, int index)
{
	// ensure the component does not yet exist in the overlay
	for (int i = 0; i < _components.size(); ++i)
	{
		if (_components[i] == component)
			return;
	}

	// add the new component
	component->setParent(this);
	component->setAlphaAnim(0.0f, NxMath::min(component->getStyle().getAlpha(), getStyle().getAlpha()), 25);

	vector<OverlayComponent *>::iterator iter = _components.end();
	if (index < _components.size())
		iter = _components.begin() + index;
	_components.insert(iter, component);

	// XXX: animate to the new pref bounds
	setSize(getPreferredDimensions());

	// make this component visible if it is not already
	if (items().size() > 0)
		getStyle().setVisible(true);
}

void OverlayLayout::removeItem( OverlayComponent * component )
{
	// remove the component if it exists
	vector<OverlayComponent *>::iterator iter = _components.begin();
	while (iter != _components.end())
	{
		if (*iter == component)
		{
			_components.erase(iter);
			return;
		}
		iter++;
	}
}

void OverlayLayout::deleteItem( OverlayComponent * component )
{
	// trigger the mouse up if we are in the middle of a mouse-down
	if (component == _mouseDownComponent)
	{
		_mouseDownComponent->onMouseUp(MouseOverlayEvent(_mouseDownComponent, Vec3(0.0f), Vec3(0.0f), _mouseDownButton));
		_mouseDownComponent = NULL;
	}

	// remove the component if it exists
	vector<OverlayComponent *>::iterator iter = _components.begin();
	while (iter != _components.end())
	{
		if (*iter == component)
		{
			(*iter)->setParent(NULL);
			_components.erase(iter);
			delete component;

			// relayout
			setSize(getPreferredDimensions());

			// hide this component if there are no items
			if (_components.size() == 0)
			{
				getStyle().setVisible(false);
			}

			return;
		}
		iter++;
	}
}

void OverlayLayout::clearItems()
{
	// delete all container elements
	for (int i = 0; i < _components.size(); ++i)
	{
		delete _components[i];
	}
	_components.clear();
}

const vector<OverlayComponent *>& OverlayLayout::items() const
{
	return _components;
}

void OverlayLayout::setSize( Vec3 newSize )
{
	OverlayComponent::setSize(newSize);
	// XXX: set the components' allowable dimensions
	for (int i = 0; i < _components.size(); ++i)
	{
		if (_components[i]->getSize().isZero())
			_components[i]->setSize(_components[i]->getPreferredDimensions());
	}
	reLayout();
}

Vec3 OverlayLayout::getPreferredDimensions()
{
	// return the preferred bounds of all the other contained components
	Vec3 dims(0,0,0);

	Bounds fullPrefBounds;
	fullPrefBounds.setEmpty();

	// get the dimensions of the bounds of all the children items
	for (int i = 0; i < _components.size(); ++i)
	{
		OverlayComponent * c = _components[i];
		Vec3 pos = c->getPosition();
		Bounds componentBounds;
		componentBounds.set(pos, pos + _components[i]->getPreferredDimensions());
		fullPrefBounds.combine(componentBounds);
	}
	fullPrefBounds.getDimensions(dims);

	// add the current padding to the dimension bounds
	dims.x += getStyle().getPadding(LeftRightEdges);
	dims.y += getStyle().getPadding(TopBottomEdges);

	return dims;
}

void OverlayLayout::reLayout()
{
	// ensure components exist
	if (_components.empty())
		return;

	// get this layout's bounds
	Vec3 dims = getSize();

	// get the max children bounds
	Vec3 maxDims;
	Bounds maxBounds;
	for (int i = 0; i < _components.size(); ++i)
	{
		OverlayComponent * c = _components[i];
		maxBounds.combine(c->getBounds());
	}
	maxBounds.getDimensions(maxDims);

	if (!getStyle().isFloating())
	{
		// reset the children to 0,0
		for (int i = 0; i < _components.size(); ++i)
		{
			OverlayComponent * c = _components[i];
			c->setPosition(Vec3(0.0f));
		}
	}
	else
	{
		Vec3 pos = getStyle().getOffset();
		if (pos.magnitudeSquared() > 0.0f)
		{
			Vec3 absPos;
			absPos.x = abs(pos.x);
			absPos.y = abs(pos.y);

			// determine the position based on the absolute or percentage offset
			// x percentage check
			bool isXPercentage = (0.0f <= absPos.x && absPos.x <= 1.0f);
			if (isXPercentage)
			{
				if (pos.x < 0.0f)
					pos.x = dims.x * (1.0f - absPos.x);
				else
					pos.x = dims.x * absPos.x;
			}
			// y percentage check
			bool isYPercentage = (0.0f <= absPos.y && absPos.y <= 1.0f);
			if (isYPercentage)
			{
				if (pos.y < 0.0f)
					pos.y = dims.y * (1.0f - absPos.y);
				else
					pos.y = dims.y * absPos.y;
			}

			// position them according to the absolute positions (if such exist)
			for (int i = 0; i < _components.size(); ++i)
			{
				OverlayComponent * c = _components[i];
				Vec3 size = c->getSize();
				Vec3 newPos = pos;

				// center the component on the percentage positions 
				if (isXPercentage)
					newPos.x = (newPos.x - (size.x/2.0f));
				if (isYPercentage)
					newPos.y = (newPos.y - (size.y/2.0f));

				// bound the components positions by the dimensions of this layout component
				newPos.max(Vec3(0.0f));
				newPos.min(Vec3(dims.x - size.x, dims.y - size.y, 0));
				c->setPosition(newPos);
			}
		}
		else
		{
			// otherwise, check the bound edges and position along those
		}
	}
}
#ifdef DXRENDER
	void OverlayLayout::onRender(const Vec3 & offset, const Bounds & renderingBounds)
#else
	void OverlayLayout::onRender()
#endif
{
	if (getStyle().getAlpha() > 0.0f)
	{
		// render the background before rendering the messages
		// offset by the padding
#ifdef DXRENDER
		onRenderBackground(offset, renderingBounds);
#else
		onRenderBackground();
#endif

		Bounds newRenderingBounds = renderingBounds;
		
		// Check to see if we need to modify the rendering constraints for this
		// overlay component
		if (getStyle().getOverflow() == OverlayStyle::Hidden)
		{
			// Perform an intersection of the two bounds
			Vec3 parentDims = renderingBounds.getMax() - renderingBounds.getMin();
			Vec3 parentOffset = renderingBounds.getMin();
			Vec3 thisDims = getBounds().getMax() - getBounds().getMin();
			Vec3 thisOffset = getBounds().getMin();

			float top = NxMath::min(parentOffset.y + parentDims.y, thisOffset.y + thisDims.y);
			float bottom = NxMath::max(parentOffset.y, thisOffset.y);
			float left = NxMath::max(parentOffset.x, thisOffset.x);
			float right = NxMath::min(parentOffset.x + parentDims.x, thisOffset.x + thisDims.x);
			newRenderingBounds.set(left, bottom, 0.0f, right, top, 0.0f);
		}

		// render each of the contained components in place
		for (int i = 0; i < _components.size(); ++i)
		{
			OverlayComponent * comp = _components[i];
			if (comp->getStyle().isVisible())
			{
				const Vec3& compPos = comp->getPosition();				
#ifdef DXRENDER
				comp->onRender(offset + compPos, newRenderingBounds);
#else
				glPushMatrix();
					glTranslatef(compPos.x, compPos.y, 0.0f);
					comp->onRender();
				glPopMatrix();
#endif
			}
		}
	}
}

#ifdef DXRENDER
void OverlayLayout::onRelease()
{
	vector<OverlayComponent *>::iterator it = _components.begin();
	while (it != _components.end())
	{
		(*it)->onRelease();
		it++;
	}
	
	OverlayComponent::onRelease();
}
#endif

bool OverlayLayout::onTimer(TimerOverlayEvent& timerEvent)
{
	// call the custom timer event handlers
	OverlayComponent::onTimer(timerEvent);

	// update each of the contained components
	for (int i = 0; i < _components.size(); ++i)
	{
		OverlayComponent * c = _components[i];
		c->onTimer(timerEvent);
	}
	return false;
}

void OverlayLayout::onSize( const Vec3& newSize )
{
	setSize(newSize);
}	

bool OverlayLayout::onMouseMove(MouseOverlayEvent& mouseEvent)
{
	// XXX: always return false so that _all_ components get the move event?

	// call the custom mouse event handlers
	if (!OverlayComponent::onMouseMove(MouseOverlayEvent(this, mouseEvent.getPosition() - getBounds().getMin(), mouseEvent.getAbsolutePosition(), mouseEvent.getButton(), mouseEvent.intersectsTarget())))
	{
		// pass the mouse move to items whose bounds are intersected by the 
		// mouse move position
		for (int i = 0; i < _components.size(); ++i)
		{
			OverlayComponent * c = _components[i];
			Bounds componentBounds = c->getBounds();
			if (intersects(mouseEvent.getPosition(), componentBounds))
			{
				c->onMouseMove(MouseOverlayEvent(c, mouseEvent.getPosition() - componentBounds.getMin(), mouseEvent.getAbsolutePosition(), mouseEvent.getButton(), true));
			}
			else
			{
				// NOTE: all components can get the mouse move events, even if the current cursor
				// does not intersect the component's space.  However, the components who do not
				// intersect the cursor can not cancel the propogation of the mouse event back up
				// to the mouse manager.
				c->onMouseMove(MouseOverlayEvent(c, Vec3(0,0,0), mouseEvent.getAbsolutePosition(), mouseEvent.getButton(), false));
			}
		}
	}
	return false;
}

bool OverlayLayout::onMouseDown(MouseOverlayEvent& mouseEvent)
{
	// call the custom mouse event handlers
	if (!OverlayComponent::onMouseDown(MouseOverlayEvent(this, mouseEvent.getPosition() - getBounds().getMin(), mouseEvent.getAbsolutePosition(), mouseEvent.getButton())))
	{
		// pass the mouse move to items whose bounds are intersected by the 
		// mouse move position
		for (int i = 0; i < _components.size(); ++i)
		{
			OverlayComponent * c = _components[i];
			Bounds componentBounds = c->getBounds();
			/*
			// XXX: DEBUG
			consoleWrite("mousePos: (%f,%f)\n", mouseEvent.getPosition().x, mouseEvent.getPosition().y);
			if (dynamic_cast<NameableOverlay *>(c))
			{
				consoleWrite("\tcomponent: %s \t(%f,%f - %f,%f)\n", dynamic_cast<NameableOverlay *>(c)->getText().c_str(),
					componentBounds.getMin().x, componentBounds.getMin().y,
					componentBounds.getMax().x, componentBounds.getMax().y);
			}
			*/
			if (intersects(mouseEvent.getPosition(), componentBounds))
			{
				if (c->onMouseDown(MouseOverlayEvent(c, mouseEvent.getPosition() - componentBounds.getMin(), mouseEvent.getAbsolutePosition(), mouseEvent.getButton())) &&
					c->getStyle().isVisible() && (c->getStyle().getAlpha() > 0.0f))
				{
					_mouseDownComponent = c;
					_mouseDownButton |= mouseEvent.getButton();
					return true;
				}
			}
		}
		return false;
	}
	return true;
}

bool OverlayLayout::onMouseUp(MouseOverlayEvent& mouseEvent)
{
	// call the custom mouse event handlers
	if (!OverlayComponent::onMouseUp(MouseOverlayEvent(this, mouseEvent.getPosition() - getBounds().getMin(), mouseEvent.getAbsolutePosition(), mouseEvent.getButton())))
	{
		// pass the mouse move to items whose bounds are intersected by the 
		// mouse move position
		for (int i = 0; i < _components.size(); ++i)		
		{
			OverlayComponent * c = _components[i];
			Bounds componentBounds = c->getBounds();			
			if (intersects(mouseEvent.getPosition(), componentBounds))
			{
				c->onMouseUp(MouseOverlayEvent(c, mouseEvent.getPosition() - componentBounds.getMin(), mouseEvent.getAbsolutePosition(), mouseEvent.getButton())) &&
					c->getStyle().isVisible() && (c->getStyle().getAlpha() > 0.0f);
			}						
		}

		bool result = false;
		if (_mouseDownComponent)
		{
			Bounds bounds = _mouseDownComponent->getBounds();
			if (intersects(mouseEvent.getPosition(), bounds))
				result = _mouseDownComponent->onMouseUp(MouseOverlayEvent(_mouseDownComponent, mouseEvent.getPosition() - bounds.getMin(), mouseEvent.getAbsolutePosition(), mouseEvent.getButton()));
			else
			{
				// NOTE: if the mouse up is outside the bounds of the mouse down, the position is reset to 0,0
				result = _mouseDownComponent->onMouseUp(MouseOverlayEvent(_mouseDownComponent, Vec3(0,0,0), mouseEvent.getAbsolutePosition(), mouseEvent.getButton()));
			}
			_mouseDownButton &= !mouseEvent.getButton();
			if (!_mouseDownButton)
				_mouseDownComponent = NULL;
		}
		return result;
	}
	return true;
}

bool OverlayLayout::intersects( const Vec3& pos, const Bounds& bounds )
{
	const Vec3& min = bounds.getMin();
	const Vec3& max = bounds.getMax();

	return (pos.x >= min.x) && (pos.x <= max.x) && (pos.y > min.y) && (pos.y < max.y);
}

bool OverlayLayout::isReady() const
{
	bool ready = true;
	for (int i = 0; i < _components.size() && ready; ++i)
	{
		ready = ready && _components[i]->isReady();
	}
	return ready;
}

void OverlayLayout::setAlpha( float alpha )
{
	OverlayComponent::setAlpha(alpha);
	for (int i = 0; i < _components.size(); ++i)
	{
		_components[i]->setAlpha(alpha);
	}
}

bool OverlayLayout::areAlphaAnimParametersValid(float startAlpha, float endAlpha, uint& animationSteps)
{
	// Added because when adding an overlay component to a layout, the layout gives the component an alpha animation
	// from 0 to 0. Sometimes, this animation unwantedly overrides the components alpha.
	if (qFuzzyCompare(startAlpha, endAlpha))
	{
		// check if all the component's alphas at equal to this (otherwise, we continue with the animation)
		for (int i = 0; i < _components.size(); ++i)
		{
			if (!qFuzzyCompare(endAlpha, _components[i]->getStyle().getAlpha()))
				return true;
		}
	}
	return OverlayComponent::areAlphaAnimParametersValid(startAlpha, endAlpha, animationSteps);
}

void OverlayLayout::killAnimation()
{
	OverlayComponent::killAnimation();
	for (int i = 0; i < _components.size(); ++i)
		_components[i]->killAnimation();
}

void OverlayLayout::finishAnimation()
{
	OverlayComponent::finishAnimation();
	for (int i = 0; i < _components.size(); ++i)
		_components[i]->finishAnimation();
}

//
// VerticalOverlayLayout implementation
//
VerticalOverlayLayout::VerticalOverlayLayout()
: OverlayLayout()
{
	getStyle().setBackgroundColor(ColorVal(160, 25, 25, 30));
	getStyle().setPadding(AllEdges, 5.0f);
	getStyle().setSpacing(2.5f);
	/* XXX: DEBUG
	getStyle().setPadding(TopEdge, 20.0f);
	getStyle().setPadding(BottomEdge, 10.0f);
	getStyle().setPadding(LeftEdge, 10.0f);
	getStyle().setPadding(RightEdge, 10.0f);
	*/
}

VerticalOverlayLayout::~VerticalOverlayLayout()
{}

void VerticalOverlayLayout::reLayout()
{
	const vector<OverlayComponent *>& items = OverlayLayout::items();

	// get the dimension of this container
	Vec3 dims = getSize();

	// set the position of each of the items
	float tmpX = 0.0f;
	Vec3 finalPos(0,getStyle().getPadding(BottomEdge),0), itemPos, itemDims;
	vector<OverlayComponent *>::const_reverse_iterator riter = items.rbegin();
	while (riter != items.rend())
	{
		OverlayComponent * item = *riter;
		itemPos = item->getPosition();
		itemDims = item->getPreferredDimensions();
		if (getStyle().isAlignedOnEdge(TopEdge))
			tmpX = dims.x - itemDims.x;
		else if (getStyle().isAlignedOnEdge(BottomEdge))
			tmpX = 0.0f;
		else	// center align
			tmpX = (dims.x - itemDims.x) / 2.0f;
		item->setPosition(Vec3(finalPos.x + tmpX, finalPos.y, 0));
		finalPos.y += itemDims.y + getStyle().getSpacing();
		riter++;
	}
}

void VerticalOverlayLayout::setSize( Vec3 newSize )
{
	// animate the size vertically
	Vec3 curSize = getSize();
	curSize.x = newSize.x;
	// XXX: TEMP setSizeAnim(curSize, newSize, 15 /*, (FinishedCallBack) HideLayoutIfEmptyAfterAnim, (void *) this */);

	//
	OverlayLayout::setSize(newSize);
}

Vec3 VerticalOverlayLayout::getPreferredDimensions()
{
	// return the preferred bounds of all the other contained components
	Vec3 dims(0,0,0);

	// get the dimensions of the bounds of all the children items
	const vector<OverlayComponent *>& components = items();
	for (int i = 0; i < components.size(); ++i)
	{
		OverlayComponent * c = components[i];
		Vec3 cPrefDims = c->getPreferredDimensions();

		dims.x = NxMath::max(dims.x, cPrefDims.x);
		dims.y += cPrefDims.y;
		if (i < (components.size() - 1))
		{
			dims.y += getStyle().getSpacing();
		}
	}

	// add the current padding to the dimension bounds
	dims.x += getStyle().getPadding(LeftRightEdges);
	dims.y += getStyle().getPadding(TopBottomEdges);

	return dims;
}


//
// HorizontalOverlayLayout implementation
//
HorizontalOverlayLayout::HorizontalOverlayLayout()
: OverlayLayout()
{
	/* XXX: DEBUG
	getStyle().setBackgroundColor(ColorVal(160, 25, 25, 90));
	getStyle().setPadding(TopEdge, 20.0f);
	getStyle().setPadding(BottomEdge, 10.0f);
	getStyle().setPadding(LeftEdge, 10.0f);
	getStyle().setPadding(RightEdge, 10.0f);
	*/
	getStyle().setSpacing(7.0f);
}

HorizontalOverlayLayout::~HorizontalOverlayLayout()
{}

void HorizontalOverlayLayout::reLayout()
{
	const vector<OverlayComponent *>& items = OverlayLayout::items();

	// set the position of each of the items
	// center the messages horizontally
	float tmpY = 0.0f;
	Vec3 dims = getSize();
	Vec3 finalPos(getStyle().getPadding(LeftEdge),0,0), itemPos, itemDims;
	for (int i = 0; i < items.size(); ++i)
	{
		OverlayComponent * item = items[i];
		itemPos = item->getPosition();
		itemDims = item->getPreferredDimensions();
		if (getStyle().isAlignedOnEdge(TopEdge))
			tmpY = dims.y - itemDims.y;
		else if (getStyle().isAlignedOnEdge(BottomEdge))
			tmpY = 0.0f;
		else	// center align
			tmpY = (dims.y - itemDims.y) / 2.0f;
		item->setPosition(Vec3(finalPos.x, finalPos.y + tmpY, 0));
		finalPos.x += itemDims.x + getStyle().getSpacing();
	}
}

void HorizontalOverlayLayout::setSize( Vec3 newSize )
{
	// animate the size vertically
	Vec3 curSize = getSize();
	curSize.y = newSize.y;
	// setSizeAnim(curSize, newSize, 15 /*, (FinishedCallBack) HideLayoutIfEmptyAfterAnim, (void *) this */);
	OverlayLayout::setSize(newSize);
}

Vec3 HorizontalOverlayLayout::getPreferredDimensions()
{
	// return the preferred bounds of all the other contained components
	Vec3 dims(0,0,0);

	// get the dimensions of the bounds of all the children items
	const vector<OverlayComponent *>& components = items();
	for (int i = 0; i < components.size(); ++i)
	{
		OverlayComponent * c = components[i];
		Vec3 cPrefDims = c->getPreferredDimensions();

		dims.y = NxMath::max(dims.y, cPrefDims.y);
		dims.x += cPrefDims.x;
		if (i < (components.size() - 1))
		{
			dims.x += getStyle().getSpacing();
		}
	}

	// add the current padding to the dimension bounds
	dims.x += getStyle().getPadding(LeftRightEdges);
	dims.y += getStyle().getPadding(TopBottomEdges);

	return dims;
}


//
// TextOverlay implementation
//
TextOverlay::TextOverlay( QString txt, bool truncate )
: OverlayComponent()
, _font("Tahoma", 24)
, _textBuffer(DEFAULT_TEXTOVERLAY_CACHESIZE)
{
	init(txt, truncate);
}

TextOverlay::TextOverlay()
: OverlayComponent()
, _font("Tahoma", 24)
, _textBuffer(DEFAULT_TEXTOVERLAY_CACHESIZE)
{
	init("", false);
}

void TextOverlay::init(QString txt, bool truncate)
{
	_prefDims.zero();

	// getStyle().setBackgroundColor(ColorVal(60, 200, 25, 25));
	getStyle().setColor(ColorVal(0, 255, 255, 255));
	getStyle().setPadding(AllEdges, 0.0f);	
	getStyle().setSpacing(4.0f);

	_textBuffer.pushFlag(TextPixmapBuffer::RenderShadow);
	_textBuffer.setFont(fontManager->getFont(_font));
	_textBuffer.setMaxBounds(QSize(winOS->GetWindowWidth(), 0));
	
	// set the text and update the text buffer
	setText(txt, truncate);
}

TextOverlay::~TextOverlay()
{}

Vec3 TextOverlay::getPreferredDimensions()
{
	QSize size = _textBuffer.getActualSize();
	_prefDims = Vec3(size.width(), size.height(), 0);

	_prefDims.x += getStyle().getPadding(LeftRightEdges);
	_prefDims.y += getStyle().getPadding(TopBottomEdges);
	return _prefDims;
}

#ifdef DXRENDER
void TextOverlay::onRender(const Vec3 & offset, const Bounds & renderingBounds)
{
	// render the background before rendering the messages
	onRenderBackground(offset, renderingBounds);
#else
void TextOverlay::onRender()
{
	// render the background before rendering the messages
	onRenderBackground();
#endif

	const QSize& textSize = _textBuffer.getActualSize();
	Vec3 realOffset = offset + Vec3(getStyle().getPadding(LeftEdge), getStyle().getPadding(BottomEdge), 0);

	Bounds textBounds;
	textBounds.set(realOffset.x, realOffset.y, 0.0f, realOffset.x + (float)textSize.width(), realOffset.y + (float)textSize.height(), 0.0f);
	
	// Check if we should even render any of this
	if (!textBounds.intersects(renderingBounds))
		return;

	Vec3 maxUVs;
	_textBuffer.bindAsGLTexture(maxUVs);
	
	// Trim the texture so we only render what is visible (i.e: what is in the 
	// rendering bounds)
	Vec3 textureOffset;
	Vec3 textureUVs;
	clipToRenderableArea(textBounds, maxUVs, renderingBounds, textureOffset, textureUVs);
	
	Vec3 finalOffset = textBounds.getMin();
	Vec3 finalSize = textBounds.getMax() - finalOffset;
	
#ifdef DXRENDER
	dxr->renderBillboard(roundOffDecimals(finalOffset), 
						 roundOffDecimals(finalSize),
						 D3DXCOLOR(getStyle().getColor()),
						 textureOffset,
						 textureUVs);
#else
	const QImage& bufferSize = _textBuffer.getBuffer();
	float xOffset = (0.5f / bufferSize.width());
	float yOffset = (0.5f / bufferSize.height());

	glPushAttribToken token(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);		
		glEnable(GL_TEXTURE_2D);
		getStyle().getColor().setAsOpenGLColor();

		glTranslatef(getStyle().getPadding(LeftEdge), getStyle().getPadding(BottomEdge) + (textSize.height() - bufferSize.height()), 0);
		glBegin(GL_QUADS);
			glTexCoord2f(1,0); 	glVertex2f(bufferSize.width() + xOffset,bufferSize.height() + yOffset);
			glTexCoord2f(0,0); 	glVertex2f(xOffset,bufferSize.height() + yOffset);
			glTexCoord2f(0,1); 	glVertex2f(xOffset,yOffset);
			glTexCoord2f(1,1);	glVertex2f(bufferSize.width() + xOffset,yOffset);
		glEnd();
#endif
}

#ifdef DXRENDER
void TextOverlay::onRelease()
{
	_textBuffer.onRelease();
	OverlayComponent::onRelease();
}
#endif

TextPixmapBuffer& TextOverlay::getTextBuffer()
{
	return _textBuffer;
}

void TextOverlay::setText( QString text, bool truncate )
{
	if (_textBuffer.getText() != text)
	{
		_prefDims.zero();
		_textBuffer.setText(text);	
		_textBuffer.update();

		// mark dirty
		syncTextBounds();
	}
}

QString TextOverlay::getText() const
{
	return _textBuffer.getText();
}

bool TextOverlay::setFont( const FontDescription& font )
{
	_font = font;
	_textBuffer.setFont(fontManager->getFont(_font));
	_textBuffer.update();
	syncTextBounds();
	return true;
}

const FontDescription& TextOverlay::getFont() const
{
	return _font;
}

void TextOverlay::syncTextBounds()
{
	setSize(getPreferredDimensions());
}

void TextOverlay::enableTextShadows()
{
	_textBuffer.pushFlag(TextPixmapBuffer::RenderShadow);
	_textBuffer.update();
}

void TextOverlay::disableTextShadows()
{
	_textBuffer.popFlag(TextPixmapBuffer::RenderShadow);
	_textBuffer.update();
}


//
// ImageControlOverlay implementation
//
ImageOverlay::ImageOverlay( QString textureId, TextureNameType textureType)
: OverlayComponent()
, _reloadDimsLater(false)
, _textureId(textureId)
, _textureType(textureType)
, _active(true)
{
	_prefDims = Vec3(0.0f);
	_texDims = _prefDims;
	if (textureType == TEXTURE_ANIMATED_GIF_PATH)
	{
		_animatedTextureSource.setPath(_textureId);
		addTimerEventHandler(this);
	}
	getStyle().setScaledDimensions(texMgr->getTexturePow2Dims(textureId));
	// XXX: DEBUG getStyle().setBackgroundColor(ColorVal(60, 200, 25, 25));
}

ImageOverlay::~ImageOverlay()
{
	// delete the texture from the texture manager
	// XXX: we shouldn't delete this texture unless specified (ie. with flag)
}

void ImageOverlay::setImage( QString textureId, TextureNameType textureType )
{
	_textureId = textureId;
	_textureType = textureType;
	if (textureType == TEXTURE_ANIMATED_GIF_PATH)
	{
		_animatedTextureSource.setPath(_textureId);
		addTimerEventHandler(this);
	}
	setSize(getPreferredDimensions());
	getStyle().setScaledDimensions(texMgr->getTexturePow2Dims(textureId));
}

void ImageOverlay::setActive(bool active)
{
	_active = active;
}

Vec3 ImageOverlay::getPreferredDimensions()
{
	if (!_textureId.isEmpty())
	{
		// get the dimensions of the image
		bool reloadDimsLater = _reloadDimsLater;
		if (_textureType == TEXTURE_ID)
			_reloadDimsLater = !texMgr->isTextureState(_textureId, TextureLoaded);
		else if (_textureType == TEXTURE_ANIMATED_GIF_PATH)
			_reloadDimsLater = !_animatedTextureSource.isLoaded();

		// Dims can be loaded now, so we should relayout the parent
		if (!reloadDimsLater && _reloadDimsLater)
		{
			if (getParent())
				getParent()->reLayout();
		}

		_reloadDimsLater = reloadDimsLater;

		if (!_reloadDimsLater)
		{
			if (_textureType == TEXTURE_ID)
			{	
				const Vec3& scaledDims = getStyle().getScaledDimensions();
				if (texMgr->isTextureState(_textureId, TextureLoaded))
					_texDims = texMgr->getTextureDims(_textureId);	
				if (!scaledDims.isNotUsed())
					_texDims = scaledDims;
			} else if (_textureType == TEXTURE_ANIMATED_GIF_PATH) {
				_texDims = Vec3(_animatedTextureSource.getCurrentTextureFrame()->width,
					_animatedTextureSource.getCurrentTextureFrame()->height,
					1);
			}
			_prefDims.x = (_texDims.x) + getStyle().getPadding(LeftRightEdges);
			_prefDims.y = (_texDims.y) + getStyle().getPadding(TopBottomEdges);
		}
	}
	return _prefDims;
}

#ifdef DXRENDER
void ImageOverlay::onRender(const Vec3 & offset, const Bounds & renderingBounds)
#else
void ImageOverlay::onRender()
#endif
{
	if (_reloadDimsLater)
		getPreferredDimensions();
	if ((_textureType == TEXTURE_ID && !texMgr->isTextureState(_textureId, TextureLoaded)) ||
		(_textureType == TEXTURE_ANIMATED_GIF_PATH && !_animatedTextureSource.load()))
		return;

	// render the background before rendering the messages
#ifdef DXRENDER
	onRenderBackground(offset, renderingBounds);
#else
	onRenderBackground();
#endif

	if (!_textureId.isEmpty() && !(_textureType == TEXTURE_ANIMATED_GIF_PATH && !_animatedTextureSource.isLoaded()))
	{
		Vec3 realOffset = offset + Vec3(getStyle().getPadding(LeftEdge), getStyle().getPadding(BottomEdge), 0);
		
		Bounds imageBounds;
		imageBounds.set(realOffset.x, realOffset.y, 0.0f, realOffset.x + _texDims.x, realOffset.y + _texDims.y, 0.0f);
	
		// Check if we should even render any of this
		if (!imageBounds.intersects(renderingBounds))
			return;
	
		Vec3 textureOffset;
		Vec3 textureUVs;
		clipToRenderableArea(imageBounds, Vec3(1.0f), renderingBounds, textureOffset, textureUVs);
	
		Vec3 finalOffset = imageBounds.getMin();
		Vec3 finalSize = imageBounds.getMax() - finalOffset;
	
#ifdef DXRENDER
		dxr->device->SetTexture(0, texMgr->getGLTextureId(_textureId));
		dxr->renderBillboard(roundOffDecimals(finalOffset), roundOffDecimals(finalSize), D3DXCOLOR(getStyle().getColor()), textureOffset, textureUVs);
#else
		uint textureNum;
		float w, h;

		glPushAttribToken token(GL_ENABLE_BIT);
			if (_textureType == TEXTURE_ID) {
				textureNum = texMgr->getGLTextureId(_textureId);
				glBindTexture(GL_TEXTURE_2D, textureNum);
				w = 1.0;
				h = 1.0;
			} else if	(_textureType == TEXTURE_ANIMATED_GIF_PATH) {
				AnimatedTextureFrame * frame = _animatedTextureSource.getCurrentTextureFrame();
				textureNum = _animatedTextureSource.GLId();
				glBindTexture(GL_TEXTURE_2D, textureNum);			
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->width, frame->height, GL_BGRA, GL_UNSIGNED_BYTE, frame->pixelData);

				// Scale the image
				uint squareSize = closestPowerOfTwo(NxMath::max(frame->width, frame->height));
				w = (float) frame->width / squareSize;
				h = (float) frame->height / squareSize;
				//h = 1.0f - (h * w);
			}


			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			getStyle().getColor().setAsOpenGLColor();
			
			if(!_active)
				glColor4f(0.2f,0.2f,0.2f,0.2f);

			glTranslatef(getStyle().getPadding(LeftEdge), getStyle().getPadding(BottomEdge), 0);
			glScalef(_texDims.x, _texDims.y, _texDims.z);
			glBegin(GL_QUADS);
				glTexCoord2f(w,0); 	glVertex3f(1,1,0);
				glTexCoord2f(0,0); 	glVertex3f(0,1,0);
				glTexCoord2f(0,h); 	glVertex3f(0,0,0);
				glTexCoord2f(w,h); 	glVertex3f(1,0,0);
			glEnd();
#endif
	}	
}

bool ImageOverlay::isReady() const
{
	return texMgr->isTextureState(_textureId, TextureQueued | TextureLoaded);
}

bool ImageOverlay::onTimer( TimerOverlayEvent& timerEvent )
{
	if (_textureType == TEXTURE_ANIMATED_GIF_PATH)
		_animatedTextureSource.onUpdate();
	return true;
}

//
// MessageRemovalPolicy implementation
//
MessageClearPolicy::MessageClearPolicy( unsigned int removalTypeMask )
: _activeType(removalTypeMask)
, _setType(removalTypeMask)
, _duration(3)
, _deadzone(10.0f)
{
	_initialPos.setNotUsed();
}

MessageClearPolicy::~MessageClearPolicy()
{}

void MessageClearPolicy::setType( unsigned int removalTypeMask )
{
	_activeType = removalTypeMask;
	_setType = removalTypeMask;
}

unsigned int MessageClearPolicy::getType() const
{
	return _activeType;
}

void MessageClearPolicy::setTimeout( unsigned int duration )
{
	_timer.restart();
	_duration = duration;
}

unsigned int MessageClearPolicy::getTimeout() const
{
	return _duration;
}

void MessageClearPolicy::setInitialPoint( const Vec3& absoluteScreenPos )
{
	_initialPos = absoluteScreenPos;
}

void MessageClearPolicy::setDeadzone( float deadzone )
{
	_initialPos.setNotUsed();
	_deadzone = deadzone;
}

float MessageClearPolicy::getDeadzone() const
{
	return _deadzone;
}

bool MessageClearPolicy::onTimer( TimerOverlayEvent& timerEvent )
{
	if (_activeType & MessageClearPolicy::Timer)
	{
		// check if the timer has elapsed
		if (_duration > 0)
		{
			if (_timer.elapsed() > _duration)
			{
				_activeType = _activeType & ~MessageClearPolicy::Timer;
				return true;
			}
		}
	}
	return false;
}

bool MessageClearPolicy::onMouseDown( MouseOverlayEvent& mouseEvent )
{
	if (_activeType & MessageClearPolicy::MouseDown)
	{
		_activeType = _activeType & ~MessageClearPolicy::MouseDown;
		return true;
	}
	return false;
}

bool MessageClearPolicy::onMouseMove( MouseOverlayEvent& mouseEvent )
{
	if (_activeType & MessageClearPolicy::MouseMove) 
	{
		if (_initialPos.isNotUsed())
		{
			_initialPos = mouseEvent.getAbsolutePosition();
			return false;
		}
		else if (mouseEvent.getAbsolutePosition().distance(_initialPos) > _deadzone)
		{
			_activeType = _activeType & ~MessageClearPolicy::MouseMove;
			return true;
		}
	}
	return false;
}

bool MessageClearPolicy::onMouseUp( MouseOverlayEvent& mouseEvent )
{
	if (_activeType & (MessageClearPolicy::MouseUp | MessageClearPolicy::MouseClick))
	{
		_activeType = _activeType & ~(MessageClearPolicy::MouseUp | MessageClearPolicy::MouseClick);
		return true;
	}
	return false;
}

void MessageClearPolicy::reset()
{
	_activeType = _setType;
}

bool MessageClearPolicy::operator==( const MessageClearPolicy& policy )
{
	return (_activeType == policy._activeType &&
			_setType == policy._setType &&
			_duration == policy._duration &&
			_deadzone == policy._deadzone);
}

//
// Message implementation
//
Message::Message( QString key, QString message, int messageTypeMask, const MessageClearPolicy& policy )
: HorizontalOverlayLayout()
, themeManagerRef(themeManager)
, _key(key)
, _text(0)
, _messageTypeMask(messageTypeMask)
, _policy(policy)
{
	// set the style
	// getStyle().setBackgroundColor(ColorVal(128, 0, 255, 0));
	getStyle().setAlignedEdges(TopEdge);
	getStyle().setSpacing(0.0f);

	// clear items
	clearItems();
	_text = NULL;

	// create the label if necessary
	if (messageTypeMask & Message::Error)			// higher priority
	{
		TextOverlay * label = new TextOverlay(QT_TR_NOOP("Error"));
		label->getStyle().setColor(ColorVal(0, 255, 20, 0));
		label->disableTextShadows();
		label->getTextBuffer().pushFlag(TextPixmapBuffer::RenderFastShadow);
		addItem(label);
	}
	else if (messageTypeMask & Message::Warning)	// lower priority
	{
		TextOverlay * label = new TextOverlay(QT_TR_NOOP("Warning"));
		label->getStyle().setColor(ColorVal(0, 255, 128, 0));
		label->disableTextShadows();
		label->getTextBuffer().pushFlag(TextPixmapBuffer::RenderFastShadow);
		addItem(label);
	}	
	else if (messageTypeMask & Message::Dot)		// just a dot
	{
		TextOverlay * label = new TextOverlay(QChar(8226));
		label->getStyle().setColor(ColorVal(0, 255, 128, 0));
		label->disableTextShadows();
		label->getTextBuffer().pushFlag(TextPixmapBuffer::RenderFastShadow);
		addItem(label);
	}

	// create the text
	if (!message.isEmpty())
	{
		bool truncate = (messageTypeMask & Message::ForceTruncate) > 0;
		_text = new TextOverlay(message, truncate);
		// _text->getStyle().setBackgroundColor(ColorVal(128, 255, 0, 0));
		addItem(_text);

		_text->getTextBuffer().pushFlag(TextPixmapBuffer::RenderFastShadow);
		_text->getTextBuffer().pushFlag(TextPixmapBuffer::ForceLinearFiltering);
		_text->getTextBuffer().pushFlag(TextPixmapBuffer::DisableClearType);
		_text->getTextBuffer().pushFlag(TextPixmapBuffer::GradientColor);
		_text->getTextBuffer().setGradientColors(QColor(255, 255, 255), QColor(210, 210, 210));
		_text->disableTextShadows();
	}

	if (shared_ptr<ThemeManager> tm = themeManagerRef.lock())
		tm->registerThemeEventHandler(this);
}

Message::~Message()
{
	if (shared_ptr<ThemeManager> tm = themeManagerRef.lock())
		tm->unregisterThemeEventHandler(this);
}

bool Message::onTimer(TimerOverlayEvent& timerEvent)
{
	// call default timer behaviour
	if (HorizontalOverlayLayout::onTimer(timerEvent) ||
		_policy.onTimer(timerEvent))
	{
		fadeAway();
		return true;
	}
	return false;
}

bool Message::onMouseDown(MouseOverlayEvent& mouseEvent)
{	 
	// let the mouse manager know that we want to receive the mouse up events
	// by telling it that we handled the mouse down.  We also want the default
	// custom handlers to be called, so call the parent onMouseDown as well 
	// (just return true either way)

	if (HorizontalOverlayLayout::onMouseDown(mouseEvent) ||
		_policy.onMouseDown(mouseEvent))
	{
		fadeAway();
		return true;
	}
	return true;
}

bool Message::onMouseMove( MouseOverlayEvent& mouseEvent )
{
	if (HorizontalOverlayLayout::onMouseMove(mouseEvent) ||
		_policy.onMouseMove(mouseEvent))
	{
		fadeAway();
		return true;
	}
	return false;
}

bool Message::onMouseUp(MouseOverlayEvent& mouseEvent)
{
	if (HorizontalOverlayLayout::onMouseUp(mouseEvent) ||
		_policy.onMouseUp(mouseEvent))
	{
		fadeAway();
		return true;
	}
	return false;
}

QString Message::getKey() const
{
	return _key;
}

void Message::fadeAway()
{
	// fade the parent as well if we are the last item
	if (getParent()->items().size() <= 1)
		getParent()->setAlphaAnim(getParent()->getStyle().getAlpha(), 0.0f, 25, (FinishedCallBack) HideComponentAfterAnim);

	// fade away
	setAlphaAnim(getStyle().getAlpha(), 0.0f, 25, (FinishedCallBack) RemoveLayoutItemAfterAnim, (void *) getParent());
}

TextOverlay * Message::getText() const
{
	return _text;
}

void Message::onThemeChanged()
{
	if (!_text)
		return;
	QString fontName = themeManager->getValueAsFontFamilyName("ui.message.font.family","");
	const vector<OverlayComponent *>& components = items();
	for (int i = 0; i < components.size(); ++i)
	{	
		if (dynamic_cast<TextOverlay *>(components[i]))
		{
			TextOverlay * overlay = dynamic_cast<TextOverlay *>(components[i]);
			overlay->setFont(FontDescription(fontName, themeManager->getValueAsInt("ui.message.font.size",12)));
		}
	}
	ColorVal fontColor = themeManager->getValueAsColor("ui.message.font.color",ColorVal(255,255,255,255));
	fontColor.bigEndian.a = 0;
	_text->getStyle().setColor(fontColor);
	setSize(getPreferredDimensions());

	// relayout
	reLayout();
	markBackgroundAsDirty();
}

bool Message::operator==( const Message& msg )
{
	return (_key == msg._key &&
			_text->getText() == msg._text->getText() && 
			_messageTypeMask == msg._messageTypeMask &&
			_policy == msg._policy);
}

//
// MessageContainer implmentation
//
MessageContainer::MessageContainer()
: themeManagerRef(themeManager)
{
	themeManager->registerThemeEventHandler(this);
}

MessageContainer::~MessageContainer()
{
	if (shared_ptr<ThemeManager> tm = themeManagerRef.lock())
		tm->unregisterThemeEventHandler(this);
}

void MessageContainer::addMessage( Message * newMessage )
{
	if (!newMessage)
		return;
	assert(newMessage);
	assert(newMessage->getText());

	// check if we have this item already
	const vector<OverlayComponent *>& messages = items();
	for (int i = 0; i < messages.size(); ++i)
	{
		if (dynamic_cast<Message *>(messages[i]))
		{
			Message * message = dynamic_cast<Message *>(messages[i]);
			if (message->getKey() == newMessage->getKey())
			{
				// skip this message if it is exactly the same as before
				if (*message == *newMessage)
					return;

				deleteItem(message);
				/*
				message->setText(newMessage->getText());
				return;
				*/
			}
		}
	}

	// fade the parent as well if we are the last item
	if (isAnimating(AlphaAnim))
	{
		if (animManager->isObjAnimating(this))
			animManager->removeAnimation(this);
		setAlpha(1.0f);
		reLayout();
	}
	addItem(newMessage);
}

void MessageContainer::clearMessages()
{
	const vector<OverlayComponent *>& messages = items();
	for (int i = 0; i < messages.size(); ++i)
	{
		if (dynamic_cast<Message *>(messages[i]))
		{
			deleteItem(messages[i]);
		}
	}
	getStyle().setVisible(false);
}

bool MessageContainer::hasMessage( QString messageKey )
{
	const vector<OverlayComponent *>& messages = items();
	for (int i = 0; i < messages.size(); ++i)
	{
		if (dynamic_cast<Message *>(messages[i]))
		{
			Message * message = dynamic_cast<Message *>(messages[i]);
			if (message->getKey() == messageKey)
				return true;
		}
	}
	return false;
}

Message * MessageContainer::getMessage(QString messageKey)
{
	const vector<OverlayComponent *>& messages = items();
	for (int i = 0; i < messages.size(); ++i)
	{
		if (dynamic_cast<Message *>(messages[i]))
		{
			Message * message = dynamic_cast<Message *>(messages[i]);
			if (message->getKey() == messageKey)
				return message;
		}
	}
	return NULL;
}

void MessageContainer::dismissMessage( QString messageKey )
{
	const vector<OverlayComponent *>& messages = items();
	for (int i = 0; i < messages.size(); ++i)
	{
		Message * message = dynamic_cast<Message *>(messages[i]);
		if (message && message->getKey() == messageKey)
		{
			message->fadeAway();
			return;
		}
	}
}

bool MessageContainer::onTimer( TimerOverlayEvent& timerEvent )
{
	bool returnVal = OverlayLayout::onTimer(timerEvent);

	// remove invisible messages
	const vector<OverlayComponent *>& messages = items();
	for (int i = 0; i < messages.size(); ++i)
	{
		Message * message = dynamic_cast<Message *>(messages[i]);
		if (message)
		{
			if (message->getStyle().getAlpha() <= 0.0f && !message->isAnimating(AlphaAnim))
				deleteItem(message);
		}
	}

	return returnVal;

}

void MessageContainer::onThemeChanged()
{
	LOG("MessageContainer::onThemeChanged");
	getStyle().setBackgroundColor(themeManager->getValueAsColor("ui.message.color.background",ColorVal()));

	// relayout
	reLayout();
}

//
// NameableIconOverlay implementation
//
NameableIconOverlay::NameableIconOverlay(QString textureName, Nameable * nameable)
: ImageOverlay(textureName)
, _nameable(nameable)
{
	_isFirstClick = true;
	_clickTimer.restart();
}

bool NameableIconOverlay::onMouseDown(MouseOverlayEvent& mouseEvent)
{	 
	// let the mouse manager know that we want to receive the mouse up events
	// by telling it that we handled the mouse down. 
	if (!_isFirstClick)
	{
		// reset the state if it's been too long since the last click
		double elapsed = _clickTimer.elapsed();
		double interval = GLOBAL(dblClickInterval);
		if (elapsed > interval)
			_isFirstClick = true;
	}

	ImageOverlay::onMouseDown(mouseEvent);
	return true;
}

bool NameableIconOverlay::onMouseUp(MouseOverlayEvent& mouseEvent)
{
	if (_isFirstClick)
	{
		// set the timer
		_isFirstClick = false;
		_clickTimer.restart();
	}
	else
	{
		double elapsed = _clickTimer.elapsed();
		double interval = GLOBAL(dblClickInterval);
		if (elapsed <= interval)
		{
			// this is a double click, so just handle it and return true
			
			// find the pile that's associated with this nameable, and 
			vector<Pile *> piles = scnManager->getPiles();
			for (int i = 0; i < piles.size(); ++i)
			{			
				Pile * pile = piles[i];
				if (pile->getObjectType() == ObjectType(BumpPile, HardPile))
				{
					Nameable * pileNameable = dynamic_cast<Nameable *>(pile);
					if (pileNameable && (pileNameable == _nameable))
					{
						// launch this pile in explorer
						FileSystemPile * fsPile = dynamic_cast<FileSystemPile *>(pile);
						fsPile->getOwner()->onLaunch();
					}
				}
			}
		}
		_isFirstClick = true;
	}
	return false;
}


//
// NameableOverlay implementation
//
NameableOverlay::NameableOverlay(Nameable * nameable)
: HorizontalOverlayLayout()
, themeManagerRef(themeManager)
, _textOverlay(NULL)
, _nameable(nameable)
, _isSelected(false)
{	
	getStyle().setAlignedEdges(TopEdge);

	_textOverlay = new TextOverlay;
		this->addItem(_textOverlay);
	updateTextFromNameable();

	getBackgroundBuffer().setTextPixmapBufferAsMask(&(_textOverlay->getTextBuffer()));
	_textOverlay->getTextBuffer().setFont(QFont(QT_NT("Segoe UI"), 9));
	_textOverlay->getTextBuffer().setFlags(TextPixmapBuffer::Filename);
	_textOverlay->getTextBuffer().pushFlag(TextPixmapBuffer::FlexibleMaxBounds);
	_textOverlay->getTextBuffer().setMaxBounds(QSize(getDefaultTextWidth(), 0));
	_textOverlay->getTextBuffer().update();
	setSize(getPreferredDimensions());
	
	if (shared_ptr<ThemeManager> tm = themeManagerRef.lock())
		tm->registerThemeEventHandler(this);
}

NameableOverlay::~NameableOverlay()
{
	if (shared_ptr<ThemeManager> tm = themeManagerRef.lock())
		tm->unregisterThemeEventHandler(this);
}

int NameableOverlay::getDefaultTextWidth()
{
	const int defaultTextWidth = 64;
	return (defaultTextWidth * FontManager::getDPIScale());
}

bool NameableOverlay::updateTextFromNameable()
{
	if (!_nameable)
		return false;

	QString text = _nameable->getFullText();
	bool isVisible = getStyle().isVisible() || getStyle().getAlpha() <= 0.0f;

	// update the text flags (do this before we set the text)
	if (!GLOBAL(settings).showIconExtensions && _nameable->hasExtension())
		_textOverlay->getTextBuffer().pushFlag(TextPixmapBuffer::HideFileExtension);
	else
		_textOverlay->getTextBuffer().popFlag(TextPixmapBuffer::HideFileExtension);
	// set the text
	if (!text.isEmpty())
		_textOverlay->setText(text, _nameable->isTextTruncated());

	setSize(getPreferredDimensions());
	if (isVisible)
		markBackgroundAsDirty();
	if (getParent())
		getParent()->reLayout();
	return true;
}

Nameable * NameableOverlay::getNameable()
{
	return _nameable;
}

TextOverlay * NameableOverlay::getTextOverlay()
{
	return _textOverlay;
}

void NameableOverlay::onThemeChanged()
{
	LOG_START("NameableOverlay::onThemeChanged()");
	QString fontName = themeManager->getValueAsFontFamilyName("ui.icon.font.family","");
	_textOverlay->setFont(FontDescription(fontName, themeManager->getValueAsInt("ui.icon.font.size",12)));

	// alpha is ignored in this font color, so make sure it's not INVISIBLE!
	ColorVal col = themeManager->getValueAsColor("ui.icon.font.color",ColorVal(255,255,255,255));
	col.bigEndian.a = 255;
	_textOverlay->getStyle().setColor(col);
	_backgroundColor = themeManager->getValueAsColor("ui.icon.font.background.default",ColorVal());
	_selectedBackgroundColor = themeManager->getValueAsColor("ui.icon.font.background.selection",ColorVal());
	markSelected(_isSelected);
		
	// disable text relayout when selecting a large group of items
	textManager->disableForceUpdates();

	// relayout
	reLayout();
	
	// disable text relayout when selecting a large group of items
	textManager->enableForceUpdates();

	markBackgroundAsDirty();
	LOG_FINISH("NameableOverlay::onThemeChanged()");
}

void NameableOverlay::markSelected( bool selected )
{
	if (_isSelected != selected)
	{
		if (GLOBAL(settings).PrintMode)
			getStyle().setBackgroundColor(_selectedBackgroundColor);
		else
			getStyle().setBackgroundColor(selected ? _selectedBackgroundColor : _backgroundColor);

		_isSelected = selected;

		if (selected)
		{
			_textOverlay->getTextBuffer().popFlag(TextPixmapBuffer::Filename);
			_textOverlay->getTextBuffer().pushFlag(TextPixmapBuffer::SelectedFilename);
		}
		else
		{
			_textOverlay->getTextBuffer().popFlag(TextPixmapBuffer::SelectedFilename);
			_textOverlay->getTextBuffer().pushFlag(TextPixmapBuffer::Filename);
		}
			
 		_textOverlay->getTextBuffer().update();

		setSize(getPreferredDimensions());
		
		getBackgroundBuffer().setColor(QColor(30, 35, 40));
		getBackgroundBuffer().setFlags(BackgroundPixmapBuffer::GradientColor);
		markBackgroundAsDirty();
		
		textManager->forceUpdate();
		rndrManager->invalidateRenderer();
	}
}

#ifdef DXRENDER
void NameableOverlay::onRender(const Vec3 & offset, const Bounds & renderingBounds)
#else
void NameableOverlay::onRender()
#endif
{
	if (GLOBAL(settings).RenderText)
#ifdef DXRENDER
		HorizontalOverlayLayout::onRender(offset, renderingBounds);
#else
		HorizontalOverlayLayout::onRender();
#endif
}

/*
 * AbsoluteOverlayLayout implementation
 */
AbsoluteOverlayLayout::AbsoluteOverlayLayout()
: OverlayLayout()
{}

AbsoluteOverlayLayout::~AbsoluteOverlayLayout()
{}

Vec3 AbsoluteOverlayLayout::getPreferredDimensions()
{
	// return full window bounds
	if (getParent())
		return getParent()->getSize();	
	return Vec3((float)winOS->GetWindowWidth(), (float)winOS->GetWindowHeight(),0);
}

void AbsoluteOverlayLayout::reLayout()
{
	const vector<OverlayComponent *>& items = OverlayLayout::items();

	// set the position of each of the items according to their offset
	uint size = items.size();
	for (int i = 0; i < size; ++i)
	{
		OverlayComponent * item = items[i];
		Vec3 itemOffset = item->getStyle().getOffset();
		item->setPosition(itemOffset);
	}
}

NamableOverlayLayout::NamableOverlayLayout()
{
}

NamableOverlayLayout::~NamableOverlayLayout()
{
}

void NamableOverlayLayout::reLayout()
{
	const vector<OverlayComponent *>& items = OverlayLayout::items();

	// set the position of each of the items according to their offset
	uint size = items.size();
	for (int i = 0; i < size; ++i)
	{
		OverlayComponent * item = items[i];
		Vec3 itemOffset = item->getStyle().getOffset();
		// inverse the coordinates, from y+ down to y+ up
		itemOffset.y -= item->getSize().y;
		item->setPosition(itemOffset);
	}
}

#ifdef DXRENDER
void NamableOverlayLayout::onRender(const Vec3 & offset, const Bounds & renderingBounds)
#else
void NamableOverlayLayout::onRender()
#endif
{
#ifdef DXRENDER
	AbsoluteOverlayLayout::onRender(offset, renderingBounds);
#else
	AbsoluteOverlayLayout::onRender();
#endif

}

void NamableOverlayLayout::determineVisibility()
{	
	static int numPiles = 0;
	const vector<OverlayComponent *>& components = items();
	vector<Pile *> piles = scnManager->getPiles();
	int windowHeight = winOS->GetWindowHeight();

	QRegion pilesRegion;
	Vec3 tlPt(0.0f), brPt(0.0f);
	Vec3 pileExtents(0.0f);
	int curNumPiles = 0;
	for (int i = 0; i < piles.size(); ++i)
	{
		if (piles[i]->getPileState() == Grid)
		{
			// Get the top left/bottom right coords
			const float bottomBuffer = 20.0f;
			Bounds b = piles[i]->getPileBounds(true);
			tlPt = world2window(b.getMax());
			brPt = world2window(b.getMin());
#ifdef DXRENDER
			QRect pileRect(tlPt.x, tlPt.y, brPt.x - tlPt.x, ((brPt.y - tlPt.y) + (bottomBuffer)));
#else
			QRect pileRect(tlPt.x, windowHeight - tlPt.y, brPt.x - tlPt.x, (tlPt.y - brPt.y) + (bottomBuffer));
#endif
			pilesRegion += pileRect;
			++curNumPiles;
		}
	}

	// return early if we don't have any gridded piles and
	// the number of piles changes (to catch the case where 
	// a gridded pile is closed)
	if (pilesRegion.isEmpty() && (curNumPiles == numPiles))
		return;

	// loop through all the nameable components
	NameableOverlay * overlay = NULL;
	Nameable * nameable = NULL;
	for (unsigned int i = 0; i < components.size(); ++i)
	{
		overlay = (NameableOverlay *) components[i];
		nameable = overlay->getNameable();
		BumpObject *obj = dynamic_cast<BumpObject*>(nameable);
		if (obj)
		{
			Pile *pile = obj->getPileParent();

			// ignore this if it is already hidden
			if (nameable && !nameable->isTextHidden() && (!pile || 
				(pile->getPileState() != Grid && pile->getPileState() != Stack)))
			{
				Vec3 dims = overlay->getSize();
				Vec3 pos = overlay->getPosition();
				QPoint p1(pos.x, windowHeight - pos.y);
				QPoint p2(pos.x, windowHeight - (pos.y + dims.y));
				QPoint p3(pos.x + dims.x, windowHeight - pos.y);
				QPoint p4(pos.x + dims.x, windowHeight - (pos.y + dims.y));

				if (pilesRegion.contains(p1) || pilesRegion.contains(p2) ||
					pilesRegion.contains(p3) || pilesRegion.contains(p4))
				{
					// hide
					if (overlay->getStyle().isVisible())
					{
						overlay->getStyle().setVisible(false);
						invisibleItems.insert(overlay);
					}
				}
				else if (invisibleItems.contains(overlay))
				{
					// show
					overlay->getStyle().setVisible(true);
					invisibleItems.remove(overlay);
				}
			}
		}
	}
	numPiles = curNumPiles;
}

bool NamableOverlayLayout::isInvisible( NameableOverlay * obj )
{
	return invisibleItems.contains(obj);
}
