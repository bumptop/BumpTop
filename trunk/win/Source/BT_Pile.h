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

#ifndef _BT_PILE_
#define _BT_PILE_

#include "BT_BumpObject.h"

// -----------------------------------------------------------------------------

class Actor;
class NxActorWrapper;
class PbBumpObject;

// -----------------------------------------------------------------------------

#define PILE_SLATE_THICKNESS		0.2f
#define PILE_SLATE_SPACE_ADJUSTMENT	0.1f
#define PILE_GRID_ROWSIZE 4
#define PILE_GRID_COLSIZE 4

// -----------------------------------------------------------------------------

// Pile types are complimentary to BumpObjectType [Use secondaryType (1 << 0) to (1 << 11)]
enum PileType
{
	SoftPile		= (1 << 0),
	HardPile		= (1 << 1),
};

// -----------------------------------------------------------------------------

// Pile State is complimentary to BumpObjectType [Use ternaryType (1 << 0) to (1 << 15)
enum PileState
{
	NoState			= (1 << 0),
	Stack			= (1 << 1),
	Grid			= (1 << 2),
	LayingOut		= (1 << 3),
	LaidOut			= (1 << 4),
	Leaf			= (1 << 5)
};

enum UpdateFlag
{
	Update,
	NoUpdate
};

// -----------------------------------------------------------------------------

template<class T>
class ContainerView
{
	const vector<T>&	_container;
	int					_rowSize;
	int					_colSize;
	// note: this represents the index of the first visible item in this view
	int					_visibleItemOffsetIndex;

public:
	ContainerView(const vector<T>& container, int rowSize, int colSize)
	: _container(container)
	, _rowSize(rowSize)
	, _colSize(colSize)
	, _visibleItemOffsetIndex(0)
	{}

	// operations
	void resync()
	{
		// validate all of the indices/members depending on the container contents
	}

	void next(int numRows)
	{
		int newAbsOffset = NxMath::max(0, NxMath::min(_container.size(), _visibleItemOffsetIndex + (numRows * _rowSize)));
		_visibleItemOffsetIndex = newAbsOffset - (newAbsOffset % _rowSize);
	}

	// accessors
	const vector<T>& getContainer() const
	{
		return _container;
	}

	vector<T> getVisibleContainer() const
	{
		vector<T> set;
		int endPos = NxMath::min((int)_container.size(), _visibleItemOffsetIndex + (_rowSize * _colSize));
		for (int i = _visibleItemOffsetIndex; i < endPos; ++i)
			set.push_back(_container[i]);
		return set;
	}

	int getRowSize() const
	{
		return _rowSize;
	}
	int getColSize() const
	{
		return _colSize;
	}
	int getRowCount() const
	{
		return (int) ceil(((float)_container.size()) / _rowSize);

	}

	int getVisibleItemOffsetIndex() const
	{
		return _visibleItemOffsetIndex;
	}
	void setVisibleItemOffsetIndex(int index)
	{
		_visibleItemOffsetIndex = index;
	}

};

// -----------------------------------------------------------------------------

class Pile : public BumpObject
{
	Q_DECLARE_TR_FUNCTIONS(Pile)

protected:
	// Items in this Pile
	vector<BumpObject *> pileItems;
	Actor *closeWidget;

	// Phantom Actor Attributes
	bool phUnhinged;
	float bufferZone;
	Vec3 phOldCent;
	Vec3 phOldDims;
	Vec3 phLastPos;
	hash_map<uint, Vec3> relPosToPhantomCentroid;
	vector<BumpObject *> shuffleGroup;

	// Pre-Pile States
	hash_map<BumpObject *, Mat34> savedMessyPoses;
	Vec3 savedStackPosition;

	// Leafing a pile
	int leafIndex;

	// Grid
	hash_map<BumpObject *, Vec3> itemDimsBeforeGrid; // For restoring item dims in case they get temporarily resized for grid
	float _gridItemsScale; //the scaling factor used to make items fit on grid
	ContainerView<BumpObject *> gridView;
	int animationStepOverride;
	bool displayLaunchOwnerWidget;
	Actor * launchOwnerWidget;
	Actor * prevPageWidget;
	Actor * nextPageWidget;

	// Currently this is only used the GLOBAL(enableSharingMode) is true,
	// for demo purposes. When clicked, it launches a new instance of
	// BumpTop pointed this folder.
	Actor * launchExternalWidget;

	boost::function<void()> _onGridHandler;
	boost::function<void()> _onGridCloseHandler;
	Vec3 getGlobalPositionForItem(Vec3 relativePosition);
	void adjustToVisibleGridItems(Vec3& currentDims);

	// Fanning Out A Pile
	bool fanningOut;
	float fanoutLen;
	QList<Vec3> fanoutPts;

	// Private Actions
	void restoreDimsBeforeGrid();
	Vec3List		resamplePath();
	bool			getLogHeightSteps(float &logHeightStep);
	void			saveRelativePositions(Vec3 centroid);
	void			unhingePhantomActor();
	void			rehingePhantomActor();
	void			createPileWidgets();
	void			updateCloseWidgetPos();
	void			updateGridWidgets();
	void			syncScrollWidgets();
	void			destroyPileWidgets();
	bool			canScrollGridRow(int numRows);
	virtual void	onBreakPin();

public:
	Pile();
	virtual ~Pile();
	
	virtual void		updateObject();
	virtual bool		addToPile(BumpObject *obj);
	virtual bool		removeFromPile(BumpObject *obj, UpdateFlag updateFlag = Update);
	virtual bool		breakPile();
	virtual void		close();
	inline void			sortByType();
	inline void			sortBySize();
	void				sortByName();
	void				updatePileState();
	void				clear(int indx = -1);
	bool				stack(Vec3 stackLocation, bool reOrientActors=true);
	bool				grid(Vec3 gridCenter);
	bool				scrollGridRow(int numRows, bool animateBeforeGridWorkaround=false);
	bool				scrollToGridItem(BumpObject * obj);
	virtual void		grow(uint numSteps = 25, float growFactor = 1.6f);
	virtual void		shrink(uint numSteps = 25, float shrinkFactor = 0.6f);
	virtual void		scaleFromReferenceDims(float scaleFactor);
	void				leafTo(int index);
	void				leafTo(BumpObject * pileItem);
	void				leafUp(int numItems = 1);
	void				leafDown(int numItems = 1);
	void				leafToBottom();
	void				tearLeafedItem();
	void				beginFanout();
	bool				fanoutTick(Vec3 nextPt);
	void				endFanout();
	virtual void		finishAnimation();
	void				updatePileItems(bool forceUpdate = false);
	Vec3				updatePhantomActorDims();
	bool				shiftItems(int index, int moveDistance);
	void				animateItemsToRelativePos();
	void				insertShuffleGroup(uint indx);
	void				clearShuffleGroup();
	BumpObject *		leafItemHitTest(int& indexOut);
	bool				isHorizontal();

	// Overloaded Operators
	BumpObject *operator[](const uint indx);
	
	// Getters
	Vec3				getDimsOfLargestPileItem();
	hash_map<uint, Vec3>getRelPositions();
	Bounds				getPileBounds(bool includeWidgets=false);
	const vector<BumpObject *>& getPileItems();
	inline Vec3			getPosBeforeLayout();
	inline BumpObject	*getLastItem();
	inline BumpObject	*getFirstItem();
	inline uint			getNumItems();
	inline PileType		getPileType();
	inline PileState	getPileState();
	inline const QList<Vec3>& getFanoutLasso() const;
	vector<BumpObject *>getShuffleGroup();
	inline BumpObject *	getActiveLeafItem();
	inline int			getLeafIndex();
	const ContainerView<BumpObject *>& getGridView();
	inline bool			isPilable(uint pileType);
	inline bool			isAnimating(uint animType = SizeAnim | AlphaAnim | PoseAnim);
	inline bool			isFanningOut();
	virtual int			isInPile(BumpObject *obj);
	virtual QString resolveDropOperationString(vector<BumpObject *>& objList);
	virtual bool		isValidDropTarget();
	virtual bool		isSourceValid();
	virtual bool		shouldRenderText();
	virtual bool		isValidTossTarget();
	virtual bool		isValidToss(vector<BumpObject *> tossedObjs);

	// Setters
	inline void			setPileState(PileState pState);
	inline void			setPileType(PileType pType);
	virtual void		setShuffleGroup(vector<BumpObject *> shufGroup);
	void				setOnGridHandler(boost::function<void()> onGridHandler);
	void				setOnGridCloseHandler(boost::function<void()> onGridCloseHandler);
	virtual inline void	updateReferenceDims();

	// Events
	virtual void		onDropEnter(vector<BumpObject *> &objList);
	virtual vector<BumpObject *> onDrop(vector<BumpObject *> &objList);
	virtual void		onRender(uint flags = RenderSideless);
	virtual void		onTossRecieve(vector<BumpObject *> tossedObjs);
	virtual void		onDragBegin(FinishedDragCallBack func = NULL);
	virtual void		onDragEnd();
	virtual void		onLaunch();
	virtual void		onWidgetClick(BumpObject * widget);
	
	// protocol buffers
	virtual bool		serializeToPb(PbBumpObject * pbPile);
	virtual bool		deserializeFromPb(const PbBumpObject * pbPile);
};

// -----------------------------------------------------------------------------

struct SortByTextureNum : public std::binary_function<BumpObject *, BumpObject *, bool>
{
	inline bool operator()(BumpObject *x, BumpObject *y);
};

// -----------------------------------------------------------------------------

struct SortByActorSize : public std::binary_function<BumpObject *, BumpObject *, bool>
{
	inline bool operator()(BumpObject *x, BumpObject *y);
};

// -----------------------------------------------------------------------------

struct SortByActorHeight : public std::binary_function<BumpObject *, BumpObject *, bool>
{
	inline bool operator()(BumpObject *x, BumpObject *y);
};

// -----------------------------------------------------------------------------

#include "BT_Pile.inl"

// -----------------------------------------------------------------------------

#endif
