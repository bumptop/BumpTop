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

#ifndef _BT_ACTOR_
#define _BT_ACTOR_

// -----------------------------------------------------------------------------

#include "BT_BumpObject.h"

// -----------------------------------------------------------------------------

class MousePointer;

// -----------------------------------------------------------------------------

// The Type of Actor this Object Represents [Uses secondaryType - (1 << 0) to (1 << 11)]
enum ActorType
{
	Logical		= (1 << 0),
	FileSystem	= (1 << 1),
	Invisible	= (1 << 2),
	Temporary	= (1 << 3),
	Text		= (1 << 4),
	Bumpspose	= (1 << 6),
	Custom		= (1 << 7),
	Webpage		= (1 << 8)
};

// -----------------------------------------------------------------------------

class Actor : public BumpObject
{
protected:

	// Mimic an Actor only if Temporary
	BumpObject * actorToMimic;

	// Texture
	QString textureID;
	
	// Crumpling, Peeling, etc
	bool crumpled;
	float crumpleSizeX;
	float crumpleSizeY;
	bool peeled;
	Vec3 peelBeginPoint;
	Vec3 peelEndPoint;
	bool maximized;
	Mat34 preMaxOrient;
	
	vector<MousePointer *> touches;
	float initialPinchDist;
	float pinchScaleFactor;

	// Material color override
	bool _useMaterialColorOverride;
#ifdef DXRENDER
	IDirect3DVertexBuffer9 * getCustomDisplayListId();
#endif

private:
#ifdef DXRENDER
	D3DMATERIAL9 _materialColorOverride;
	IDirect3DVertexBuffer9 * _displayListId;
#else
	uint _displayListId; //used if onRender flag contains RenderCustomDisplayList
	unsigned int getCustomDisplayListId();
#endif

public:
	Actor();
	virtual ~Actor();
	
	// Member Functions
	void				pushActorType(ActorType aType);
	void				popActorType(ActorType aType);
	unsigned int		getActorType() const;
	void				setActorType(unsigned int typeBitMask);
	virtual void		onLaunch();
	void				hideAndDisable();
	void				showAndEnable(Vec3& pos);

	// Events
	virtual void		onRender(uint flags = RenderSideless);
#ifdef DXRENDER
	virtual void		onRelease();
#endif
	virtual void		onDragEnd();

	// Setters
	void				setTextureID(QString newTextureID);
#ifdef DXRENDER
	void				setDisplayListId(IDirect3DVertexBuffer9 * displayListId);
#else
	void				setDisplayListId(uint displayListId);
#endif
	void				setSidelessRendering(bool val);
	inline void			setObjectToMimic(BumpObject *obj);
	virtual void		setSizeAnim(Vec3 &startSize, Vec3 &lastSize, uint steps);
	virtual void		setSizeAnim(deque<Vec3> &customSizeAnim);

	// Material overrides
	void				enableMaterialColorOverride(const QColor& color);
	void				disableMaterialColorOverride();

	// Getters	
	Actor *				getActorToMimic();
	bool				getSidelessRendering() const;
	QString				getTextureID();
#ifdef DXRENDER
	virtual inline IDirect3DTexture9 * getTextureNum();
	inline IDirect3DVertexBuffer9 * getDisplayListId() const;
#else
	virtual inline uint	getTextureNum();
	inline uint			getDisplayListId() const;
#endif
	inline QString		getTextureString();
	inline BumpObject	*getObjectToMimic() const;
	inline bool			isActorType(uint actorType);
	inline bool			isMaximized();
	virtual bool		isPilable(uint pileType);
	virtual bool		shouldRenderText();
	
	void onTouchDown(Vec2 &pt, MousePointer* mousePointer);
	void onTouchUp(Vec2 &pt, MousePointer* mousePointer);
	void onTouchMove(Vec2 &pt);
};

// -----------------------------------------------------------------------------

#include "BT_Actor.inl"

// -----------------------------------------------------------------------------

#else
	class Actor;
	enum ActorType;
#endif
