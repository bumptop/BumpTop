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

#ifndef SHAPE_VIS_H
#define SHAPE_VIS_H

// -----------------------------------------------------------------------------

class ShapeVis
{	
	// Shape vis stuff
	static int dispListPlate;
	static int dispListBox;
	static int dispListColorSideBox;
	static int dispListNoSideTexBox;
	static int dispListPhotoFrame;
	/*
	static int dispListSphere;
	static int dispListCylinder;
	*/
		
	static void render(NxBoxShape & shape);
	/*
	static void render(NxSphereShape & shape);
	static void render(NxCapsuleShape & shape);
	static bool renderSphere(const NxVec3 & pos, NxReal r);
	*/

public:
	~ShapeVis();

	static void renderShape(NxShape* shape);
	static void init();
	static void release();
	static bool renderBox(const NxVec3 & pos, const NxMat33 & orient, const NxVec3  & halfWidths, bool renderPlate=false);
	static bool renderDisplayList(const NxVec3 & pos, const NxMat33 & orient, const NxVec3 & halfWidths, unsigned int displayList);
	static bool renderColorSidedBox(const NxVec3 & pos, const NxMat33 & orient, const NxVec3  & halfWidths);
	static bool renderSideLessBox(const NxVec3 & pos, const NxMat33 & orient, const NxVec3  & halfWidths);
	static bool renderPhotoFrame(const NxVec3& pos, const NxMat33& orient, const NxVec3& halfWidths);
	static void setupGLMatrix(const NxVec3& pos, const NxMat33& orient);
};

// -----------------------------------------------------------------------------

#else
	class ShapeVis;
#endif