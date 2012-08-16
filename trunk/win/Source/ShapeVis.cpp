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
#include "ShapeVis.h"
#include "BT_SceneManager.h"

// static initializations
int ShapeVis::dispListPlate = 0;
int ShapeVis::dispListBox = 0;
int ShapeVis::dispListColorSideBox = 0;
int ShapeVis::dispListNoSideTexBox = 0;
int ShapeVis::dispListPhotoFrame = 0;
/*
int ShapeVis::dispListSphere = 0;
int ShapeVis::dispListCylinder = 0;
*/

#ifndef DXRENDER
void ShapeVis::init()
{
	glLoadIdentity();
	(dispListBox) = glGenLists(1);//get a unique display list ID.
	glNewList(dispListBox, GL_COMPILE);      

	glBegin(GL_QUADS);
	glNormal3f(0,0,1);
	glTexCoord2f(1,0); 	glVertex3f(1,1,1);
	glTexCoord2f(0,0); 	glVertex3f(-1,1,1);
	glTexCoord2f(0,1); 	glVertex3f(-1,-1,1);
	glTexCoord2f(1,1); 	glVertex3f(1,-1,1);

	glNormal3f(0,0,-1);
	glTexCoord2f(0,1); 	glVertex3f(1,-1,-1);
	glTexCoord2f(1,1); 	glVertex3f(-1,-1,-1);
	glTexCoord2f(1,0); 	glVertex3f(-1,1,-1);
	glTexCoord2f(0,0); 	glVertex3f(1,1,-1);

	glNormal3f(0,1,0);
	glTexCoord2f(0,1); 	glVertex3f(1,1,-1);
	glTexCoord2f(1,1); 	glVertex3f(-1,1,-1);
	glTexCoord2f(1,0); 	glVertex3f(-1,1,1);
	glTexCoord2f(0,0); 	glVertex3f(1,1,1);

	glNormal3f(0,-1,0);
	glTexCoord2f(0,1); 	glVertex3f(1,-1,1);
	glTexCoord2f(1,1); 	glVertex3f(-1,-1,1);
	glTexCoord2f(1,0); 	glVertex3f(-1,-1,-1);
	glTexCoord2f(0,0); 	glVertex3f(1,-1,-1);

	glNormal3f(1,0,0);
	glTexCoord2f(0,1); 	glVertex3f(1,1,1);
	glTexCoord2f(1,1); 	glVertex3f(1,-1,1);
	glTexCoord2f(1,0); 	glVertex3f(1,-1,-1);
	glTexCoord2f(0,0); 	glVertex3f(1,1,-1);

	glNormal3f(-1,0,0);
	glTexCoord2f(0,1); 	glVertex3f(-1,-1,1);
	glTexCoord2f(1,1); 	glVertex3f(-1,1,1);
	glTexCoord2f(1,0); 	glVertex3f(-1,1,-1);
	glTexCoord2f(0,0); 	glVertex3f(-1,-1,-1);

	glEnd();
	glEndList();

	// Make a Box with sides that have the UV of the first pixel of the image
	(dispListColorSideBox) = glGenLists(1);
	glNewList(dispListColorSideBox, GL_COMPILE);     
	glBegin(GL_QUADS);
		glNormal3f(0,0,1);
		glTexCoord2f(1,0); 	glVertex3f(1,1,1);
		glTexCoord2f(0,0); 	glVertex3f(-1,1,1);
		glTexCoord2f(0,1); 	glVertex3f(-1,-1,1);
		glTexCoord2f(1,1); 	glVertex3f(1,-1,1);

		glNormal3f(0,0,-1);
		glTexCoord2f(0,1); 	glVertex3f(1,-1,-1);
		glTexCoord2f(1,1); 	glVertex3f(-1,-1,-1);
		glTexCoord2f(1,0); 	glVertex3f(-1,1,-1);
		glTexCoord2f(0,0); 	glVertex3f(1,1,-1);

		glNormal3f(0,1,0);
		glTexCoord2f(0,0); 	glVertex3f(1,1,-1);
		glTexCoord2f(0,0); 	glVertex3f(-1,1,-1);
		glTexCoord2f(0,0); 	glVertex3f(-1,1,1);
		glTexCoord2f(0,0); 	glVertex3f(1,1,1);

		glNormal3f(0,-1,0);
		glTexCoord2f(0,0); 	glVertex3f(1,-1,1);
		glTexCoord2f(0,0); 	glVertex3f(-1,-1,1);
		glTexCoord2f(0,0); 	glVertex3f(-1,-1,-1);
		glTexCoord2f(0,0); 	glVertex3f(1,-1,-1);

		glNormal3f(1,0,0);
		glTexCoord2f(0,0); 	glVertex3f(1,1,1);
		glTexCoord2f(0,0); 	glVertex3f(1,-1,1);
		glTexCoord2f(0,0); 	glVertex3f(1,-1,-1);
		glTexCoord2f(0,0); 	glVertex3f(1,1,-1);

		glNormal3f(-1,0,0);
		glTexCoord2f(0,0); 	glVertex3f(-1,-1,1);
		glTexCoord2f(0,0); 	glVertex3f(-1,1,1);
		glTexCoord2f(0,0); 	glVertex3f(-1,1,-1);
		glTexCoord2f(0,0); 	glVertex3f(-1,-1,-1);
	glEnd();
	glEndList();


	// make the no-side-texture box	
	(dispListNoSideTexBox) = glGenLists(1);//get a unique display list ID.
	glNewList(dispListNoSideTexBox, GL_COMPILE);     
	glBegin(GL_QUADS);
		glNormal3f(0,0,1);
		glTexCoord2f(1,0); 	glVertex3f(1,1,1);
		glTexCoord2f(0,0); 	glVertex3f(-1,1,1);
		glTexCoord2f(0,1); 	glVertex3f(-1,-1,1);
		glTexCoord2f(1,1); 	glVertex3f(1,-1,1);

		glNormal3f(0,0,-1);
		glTexCoord2f(0,1); 	glVertex3f(1,-1,-1);
		glTexCoord2f(1,1); 	glVertex3f(-1,-1,-1);
		glTexCoord2f(1,0); 	glVertex3f(-1,1,-1);
		glTexCoord2f(0,0); 	glVertex3f(1,1,-1);
	glEnd();
	glEndList();

	// make the photo frame
	float innerDims = 0.9f;
	(dispListPhotoFrame) = glGenLists(1);
	glNewList(dispListPhotoFrame, GL_COMPILE);      
	glBegin(GL_QUADS);

		// back face
		glNormal3f(0,0,1);
		glTexCoord2f(1,0); 	glVertex3f(innerDims,innerDims,1);
		glTexCoord2f(0,0); 	glVertex3f(-innerDims,innerDims,1);
		glTexCoord2f(0,1); 	glVertex3f(-innerDims,-innerDims,1);
		glTexCoord2f(1,1); 	glVertex3f(innerDims,-innerDims,1);

		// back face border
		glNormal3f(0,0,1);
			// left
			glTexCoord2f(0,0); 	glVertex3f(1,1,1);
			glTexCoord2f(0,0); 	glVertex3f(innerDims,innerDims,1);
			glTexCoord2f(0,0); 	glVertex3f(innerDims,-innerDims,1);
			glTexCoord2f(0,0); 	glVertex3f(1,-1,1);
			// top
			glTexCoord2f(0,0); 	glVertex3f(1,1,1);
			glTexCoord2f(0,0); 	glVertex3f(-1,1,1);
			glTexCoord2f(0,0); 	glVertex3f(-innerDims,innerDims,1);
			glTexCoord2f(0,0); 	glVertex3f(innerDims,innerDims,1);
			// right
			glTexCoord2f(0,0); 	glVertex3f(-1,1,1);
			glTexCoord2f(0,0); 	glVertex3f(-1,-1,1);
			glTexCoord2f(0,0); 	glVertex3f(-innerDims,-innerDims,1);
			glTexCoord2f(0,0); 	glVertex3f(-innerDims,innerDims,1);
			// bottom
			glTexCoord2f(0,0); 	glVertex3f(-1,-1,1);
			glTexCoord2f(0,0); 	glVertex3f(1,-1,1);
			glTexCoord2f(0,0); 	glVertex3f(innerDims,-innerDims,1);
			glTexCoord2f(0,0); 	glVertex3f(-innerDims,-innerDims,1);

		// front face
		glNormal3f(0,0,-1);
		glTexCoord2f(0,1); 	glVertex3f(innerDims,-innerDims,-1);
		glTexCoord2f(1,1); 	glVertex3f(-innerDims,-innerDims,-1);
		glTexCoord2f(1,0); 	glVertex3f(-innerDims,innerDims,-1);
		glTexCoord2f(0,0); 	glVertex3f(innerDims,innerDims,-1);

		// front face border
		glNormal3f(0,0,-1);
			// left
			glTexCoord2f(0,0); 	glVertex3f(-1,1,-1);
			glTexCoord2f(0,0); 	glVertex3f(-innerDims,innerDims,-1);
			glTexCoord2f(0,0); 	glVertex3f(-innerDims,-innerDims,-1);
			glTexCoord2f(0,0); 	glVertex3f(-1,-1,-1);
			// top
			glTexCoord2f(0,0); 	glVertex3f(-1,1,-1);
			glTexCoord2f(0,0); 	glVertex3f(1,1,-1);
			glTexCoord2f(0,0); 	glVertex3f(innerDims,innerDims,-1);
			glTexCoord2f(0,0); 	glVertex3f(-innerDims,innerDims,-1);
			// right
			glTexCoord2f(0,0); 	glVertex3f(1,1,-1);
			glTexCoord2f(0,0); 	glVertex3f(1,-1,-1);
			glTexCoord2f(0,0); 	glVertex3f(innerDims,-innerDims,-1);
			glTexCoord2f(0,0); 	glVertex3f(innerDims,innerDims,-1);
			// bottom
			glTexCoord2f(0,0); 	glVertex3f(1,-1,-1);
			glTexCoord2f(0,0); 	glVertex3f(-1,-1,-1);
			glTexCoord2f(0,0); 	glVertex3f(-innerDims,-innerDims,-1);
			glTexCoord2f(0,0); 	glVertex3f(innerDims,-innerDims,-1);

		glNormal3f(0,1,0);
		glTexCoord2f(0,0); 	glVertex3f(1,1,-1);
		glTexCoord2f(0,0); 	glVertex3f(-1,1,-1);
		glTexCoord2f(0,0); 	glVertex3f(-1,1,1);
		glTexCoord2f(0,0); 	glVertex3f(1,1,1);

		glNormal3f(0,-1,0);
		glTexCoord2f(0,0); 	glVertex3f(1,-1,1);
		glTexCoord2f(0,0); 	glVertex3f(-1,-1,1);
		glTexCoord2f(0,0); 	glVertex3f(-1,-1,-1);
		glTexCoord2f(0,0); 	glVertex3f(1,-1,-1);

		glNormal3f(1,0,0);
		glTexCoord2f(0,0); 	glVertex3f(1,1,1);
		glTexCoord2f(0,0); 	glVertex3f(1,-1,1);
		glTexCoord2f(0,0); 	glVertex3f(1,-1,-1);
		glTexCoord2f(0,0); 	glVertex3f(1,1,-1);

		glNormal3f(-1,0,0);
		glTexCoord2f(0,0); 	glVertex3f(-1,-1,1);
		glTexCoord2f(0,0); 	glVertex3f(-1,1,1);
		glTexCoord2f(0,0); 	glVertex3f(-1,1,-1);
		glTexCoord2f(0,0); 	glVertex3f(-1,-1,-1);
	glEnd();
	glEndList();

	//make the plate
	(dispListPlate) = glGenLists(1);
	glNewList(dispListPlate, GL_COMPILE);      
	glBegin(GL_QUADS);
	glNormal3f(0,1,0);
		glTexCoord2f(0,1); 	glVertex3f(1,-1,1);
		glTexCoord2f(1,1); 	glVertex3f(-1,-1,1);
		glTexCoord2f(1,0); 	glVertex3f(1,-1,-1);
		glTexCoord2f(0,0); 	glVertex3f(-1,-1,-1);
	glEnd();
	glEndList();

	/*
	(dispListSphere) = glGenLists(1);
	glNewList(dispListSphere, GL_COMPILE);      
	GLUquadricObj * quadObj = gluNewQuadric ();
	gluQuadricDrawStyle (quadObj, GLU_FILL);
	gluQuadricNormals (quadObj, GLU_SMOOTH); 
	gluQuadricOrientation(quadObj,GLU_OUTSIDE);
	gluSphere (quadObj, 1.0f, 9, 7);	//unit sphere
	glEndList();
	gluDeleteQuadric(quadObj);

	(dispListCylinder) = glGenLists(1);
	glNewList(dispListCylinder, GL_COMPILE);      
	quadObj = gluNewQuadric ();
	gluQuadricDrawStyle (quadObj, GLU_FILL);
	gluQuadricNormals (quadObj, GLU_SMOOTH); 
	gluQuadricOrientation(quadObj,GLU_OUTSIDE);
	gluCylinder  (quadObj, 1.0f, 1.0f, 1.0f, 18, 1);	//unit cylinder
	glEndList();
	gluDeleteQuadric(quadObj);
	*/
}

void ShapeVis::release()
{
	// Destroy the display lists used by ShapeVis
	glDeleteLists(dispListBox, 1);
	dispListBox = 0;

	glDeleteLists(dispListNoSideTexBox, 1);
	dispListNoSideTexBox = 0;

	glDeleteLists(dispListPlate, 1);
	dispListPlate = 0;

	glDeleteLists(dispListColorSideBox, 1);
	dispListColorSideBox = 0;

	glDeleteLists(dispListPhotoFrame, 1);
	dispListPhotoFrame = 0;

	/*
	glDeleteLists(dispListSphere, 1);
	dispListSphere = 0;

	glDeleteLists(dispListCylinder, 1);
	dispListCylinder = 0;
	*/
}

ShapeVis::~ShapeVis()
{
	release();
}

void ShapeVis::setupGLMatrix(const Vec3& pos, const Mat33& orient)
{
	float glmat[16] = {0};	//4x4 column major matrix for OpenGL.
	orient.getColumnMajorStride4(&(glmat[0]));
	pos.get(&(glmat[12]));

	//clear the elements we don't need:
	glmat[15] = 1.0f;

	glMultMatrixf(&(glmat[0]));
}

void ShapeVis::render(NxBoxShape & shape)
{
	Box worldBox;
	shape.getWorldOBB(worldBox);

	// Render the OBB
	renderBox(worldBox.center, worldBox.rot, worldBox.extents);
}

/*
void ShapeVis::render(NxSphereShape & shape)
{
	//rescale our unit sphere:
	glPushMatrix();
	Mat34 pose;
	shape.getShape().getGlobalPose(pose);
	setupGLMatrix(pose.t, pose.M);
	NxReal r = shape.getRadius();
	glScaled(r,r,r);
	glCallList(dispListSphere);
	glPopMatrix();
}

void ShapeVis::render(NxCapsuleShape & capsule)
{
	//rescale unit spheres:
	Mat34 pose;
	capsule.getShape().getGlobalPose(pose);

	const NxReal & r = capsule.getRadius();
	const NxReal & h = capsule.getHeight();

	glPushMatrix();

	float glmat[16];	//4x4 column major matrix for OpenGL.
	pose.M.getColumnMajorStride4(&(glmat[0]));
	pose.t.get(&(glmat[12]));

	//clear the elements we don't need:
	glmat[3] = glmat[7] = glmat[11] = 0.0f;
	glmat[15] = 1.0f;

	glMultMatrixf(&(glmat[0]));

	float Red[4] = {1,0.7f,0.7f,0};

	glPushMatrix();
	glTranslated(0.0f, h*0.5f, 0.0f);
	glScaled(r,r,r);
	glCallList(dispListSphere);
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0f,-h*0.5f, 0.0f);
	glScaled(r,r,r);
	glCallList(dispListSphere);
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0f,h*0.5f, 0.0f);
	glScaled(r,h,r);
	glRotated(90.0f,1.0f,0.0f,0.0f);
	glCallList(dispListCylinder);
	glPopMatrix();

	glPopMatrix();
}
*/

bool ShapeVis::renderBox(const Vec3 & pos, const Mat33 & orient, const Vec3  & halfWidths, bool renderPlate)
{
	if(renderPlate)
		return renderDisplayList(pos, orient, halfWidths, dispListPlate);
	else
		return renderDisplayList(pos, orient, halfWidths, dispListBox);
}

bool ShapeVis::renderDisplayList(const Vec3 & pos, const Mat33 & orient, const Vec3 & halfWidths, unsigned int displayList)
{
	assert(displayList);

	//transform our unit cube:
	glPushMatrix();

	//glTranslated(pos.x(), pos.y(), pos.z());
	setupGLMatrix(pos, orient);

	//protect against infinitely thin leaf-AABBs, which do not shade correctly:
	Vec3 mess;
	mess = halfWidths;

	bool bad = false;
	if (mess.x <= 0.001f)
	{
		mess.setx(0.001f);
		bad = true;
	}
	if (mess.y <= 0.001f)
	{
		mess.sety(0.001f);
		bad = true;
	}
	if (mess.z <= 0.001f)
	{
		mess.setz(0.001f);
		bad = true;
	}

	float Red[4] = {1,0.7f,0.7f,0};
	if (bad)
		glMaterialfv(GL_FRONT, GL_DIFFUSE, Red);

	glScaled(mess.x, mess.y, mess.z);
	glCallList(displayList);
	glPopMatrix();

	return true;
}

bool ShapeVis::renderSideLessBox(const Vec3 & pos, const Mat33 & orient, const Vec3  & halfWidths)
{
	return renderDisplayList(pos, orient, halfWidths, dispListNoSideTexBox);
}

bool ShapeVis::renderPhotoFrame(const Vec3 & pos, const Mat33 & orient, const Vec3  & halfWidths)
{
	return renderDisplayList(pos, orient, halfWidths, dispListPhotoFrame);
}

/*
bool ShapeVis::renderSphere(const Vec3  & pos, NxReal r)
{
	//rescale our unit sphere:
	glPushMatrix();

	const Vec3 * t = &pos;
	glTranslated(t->x, t->y, t->z);

	glScaled(r,r,r);
	glCallList(dispListSphere);
	glPopMatrix();
	return true;
}
*/


void ShapeVis::renderShape(NxShape* shape)
{
	if(!shape)	
		return;
	switch (shape->getType())
		{
		case NX_SHAPE_BOX:
			{
			NxBoxShape * box = shape->isBox();
			if (box)
				render(*box);
			}
			break;
		/*
		case NX_SHAPE_SPHERE:
			{
			NxSphereShape * sphere = shape->isSphere();
			if (sphere)
				render(*sphere);
			}
			break;
		case NX_SHAPE_CAPSULE:
			{
			NxCapsuleShape * capsule = shape->isCapsule();
			if (capsule)
				render(*capsule);
			}
			break;
		*/
		}
}

bool ShapeVis::renderColorSidedBox( const NxVec3 & pos, const NxMat33 & orient, const NxVec3 & halfWidths )
{
	//transform our unit cube:
	glPushMatrix();

	//glTranslated(pos.x(), pos.y(), pos.z());
	setupGLMatrix(pos, orient);

	//glScaled(halfWidths.x(), halfWidths.y(), halfWidths.z());

	//protect against infinitely thin leaf-AABBs, which do not shade correctly:
	Vec3 mess;
	mess = halfWidths;

	bool bad = false;

	if (mess.x <= 0.001f)
	{
		mess.setx(0.001f);
		bad = true;
	}
	if (mess.y <= 0.001f)
	{
		mess.sety(0.001f);
		bad = true;
	}
	if (mess.z <= 0.001f)
	{
		mess.setz(0.001f);
		bad = true;
	}

	float Red[4] = {1,0.7f,0.7f,0};

	if (bad)
		glMaterialfv(GL_FRONT, GL_DIFFUSE, Red);

	glScaled(mess.x, mess.y, mess.z);
	glCallList(dispListColorSideBox);
	glPopMatrix();

	return true;
}
#endif