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
#include "BT_Actor.h"
#include "BT_AnimationEntry.h"
#include "BT_AnimationManager.h"
#include "BT_Authorization.h"
#include "BT_AutomatedDemo.h"
#include "BT_AutomatedJSONTesting.h"
#include "BT_AutomatedTest.h"
#include "BT_AutomatedTests.h"
#include "BT_AutomatedTradeshowDemo.h"
#include "BT_Bubbles.h"
#include "BT_BumpObject.h"
#include "BT_Camera.h"
#include "BT_Cluster.h"
#include "BT_CustomActor.h"
#include "BT_CustomizeWizard.h"
#include "BT_DialogManager.h"
#include "BT_EllipsisMenu.h"
#include "BT_EventManager.h"
#include "BT_FileSystemActorFactory.h"
#include "BT_FileSystemManager.h"
#include "BT_FileSystemPile.h"
#include "BT_FileTransferManager.h"
#include "BT_Find.h"
#include "BT_FontManager.h"
#include "BT_GLTextureManager.h"
#include "BT_KeyboardManager.h"
#include "BT_LassoMenu.h"
#include "BT_LegacyPersistenceManager.h"
#include "BT_LocalPhotoFrameSource.h"
#include "BT_Logger.h"
#include "BT_Macros.h"
#include "BT_MarkingMenu.h"
#include "BT_MouseEventManager.h"
#include "BT_NovodexOutputStream.h"
#include "BT_OverlayComponent.h"
#include "BT_PbPersistenceManager.h"
#include "BT_PhotoFrameActor.h"
#include "BT_PhotoFrameDialog.h"
#include "BT_Pile.h"
#include "BT_Profiler.h"
#include "BT_QtUtil.h"
#include "BT_RSSPhotoFrameSource.h"
#include "BT_RaycastReports.h"
#include "BT_Rename.h"
#include "BT_RenderManager.h"
#include "BT_Replayable.h"
#include "BT_RepositionManager.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_Settings.h"
#include "BT_SlideShow.h"
#include "BT_StatsManager.h"
#include "BT_StickyNoteActor.h"
#include "BT_StickyNoteMenuActionCustomizer.h"
#include "BT_Struct.h"
#include "BT_TextManager.h"
#include "BT_UndoStack.h"
#include "BT_UndoStackEntry.h"
#include "BT_Util.h"
#include "BT_WatchedObjects.h"
#include "BT_WebActor.h"
#include "BT_WebThumbnailActor.h"
#include "BT_WidgetManager.h"
#include "BT_WindowsOS.h"
#include "BumpTop.h"
#include "BT_PhotoFrameSource.h"
#include "BT_FlickrPhotoFrameSource.h"
#include "BT_LibraryManager.h"

#ifdef DXRENDER
#include "BT_DXRender.h"
#include "BT_Mesh.h"
#include "BT_VideoRender.h"
#endif

const QString originalPhotoFolderName(QT_NT("BT_OriginalPhotos"));
const float kWallPositionBuffer(10.0f);

// see http://www.halcyon.com/~ast/dload/guicon.htm
void startConsoleWin(int width, int height, char* fname)
{	
	if (GLOBAL(settings).showConsoleWindow)
	{
		int hConHandle;
		long lStdHandle;
		CONSOLE_SCREEN_BUFFER_INFO coninfo;

		FILE *fp;
		// allocate a console for this app
		AllocConsole();
		SetConsoleTitle(L"Debug Window");
		GLOBAL(consoleWindowHandle) = GetStdHandle(STD_OUTPUT_HANDLE);

		// set the screen buffer to be big enough to let us scroll text
		GetConsoleScreenBufferInfo(GLOBAL(consoleWindowHandle),	&coninfo);
		coninfo.dwSize.X = width;
		coninfo.dwSize.Y = height;
		SetConsoleScreenBufferSize(GLOBAL(consoleWindowHandle), coninfo.dwSize);

		// redirect unbuffered STDOUT to the console
		lStdHandle = (long) GetStdHandle(STD_OUTPUT_HANDLE);
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
		fp = _fdopen( hConHandle, "w" );
		*stdout = *fp;
		setvbuf( stdout, NULL, _IONBF, 0 );

		// redirect unbuffered STDIN to the console
		lStdHandle = (long) GetStdHandle(STD_INPUT_HANDLE);
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
		fp = _fdopen( hConHandle, "r" );
		*stdin = *fp;
		setvbuf( stdin, NULL, _IONBF, 0 );

		// redirect unbuffered STDERR to the console
		lStdHandle = (long) GetStdHandle(STD_ERROR_HANDLE);
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
		fp = _fdopen( hConHandle, "w" );
		*stderr = *fp;
		setvbuf( stderr, NULL, _IONBF, 0 );

		// make //cout, w//cout, cin, wcin, wcerr, cerr, wclog and clog
		// point to console as well
		ios::sync_with_stdio();
	}
}

void qtMessageHandler(QtMsgType type, const char *msg)
{
     switch (type) {
     case QtDebugMsg:
		 consoleWrite(QString("Qt Debug: %1\n").arg(msg));
		 break;
     case QtWarningMsg:
		 consoleWrite(QString("Qt Warning: %1\n").arg(msg));
		 break;
     case QtCriticalMsg:
		 consoleWrite(QString("Qt Critical: %1\n").arg(msg));
		 break;
     case QtFatalMsg:
		 consoleWrite(QString("Qt Fatal: %1\n").arg(msg));
		 break;

	 default:
		 break;
     }
}

int consoleWrite(const QString& message)
{	
	DWORD count = 0;
	
	if (GLOBAL(settings).showConsoleWindow)
	{
		HANDLE console = GLOBAL(consoleWindowHandle);
		if (console)
			WriteConsole(console, (LPVOID) message.utf16(), message.size(), &count, NULL);
	}

	LOG(message);

	return int(count);
}

int consoleWrite(const QString& name, const Vec3& vec)
{
	return consoleWrite(QString("%1: (%2, %3, %4)\n").arg(name).arg(vec.x).arg(vec.y).arg(vec.z));
}

#ifdef DXRENDER
#else
void glutSolidCube(GLdouble size)
{
    float dsize = float(size * 0.5);

#   define V(obj,b,c) glVertex3f(obj dsize, b dsize, c dsize);
#   define N(obj,b,c) glNormal3f(obj, b, c);

    /* PWO: Again, I dared to convert the code to use macros... */
    glBegin(GL_QUADS);
        N(1.0f, 0.0f, 0.0f); V(+,-,+); V(+,-,-); V(+,+,-); V(+,+,+);
        N(0.0f, 1.0f, 0.0f); V(+,+,+); V(+,+,-); V(-,+,-); V(-,+,+);
        N(0.0f, 0.0f, 1.0f); V(+,+,+); V(-,+,+); V(-,-,+); V(+,-,+);
        N(-1.0f, 0.0f, 0.0f); V(-,-,+); V(-,+,+); V(-,+,-); V(-,-,-);
        N(0.0f,-1.0f, 0.0f); V(-,-,+); V(-,-,-); V(+,-,-); V(+,-,+);
        N(0.0f, 0.0f,-1.0f); V(-,-,-); V(-,+,-); V(+,+,-); V(+,-,-);
    glEnd();

#   undef V
#   undef N
}
#endif

void fghCircleTable(float **sint,float **cost,const int n)
{
    int i;

    /* Table size, the sign of n flips the circle direction */

    const int size = abs(n);

    /* Determine the angle between samples */

    const float angle = 2*PI/(float)((n == 0) ? 1 : n);

    /* Allocate memory for n samples, plus duplicate of first entry at the end */

    *sint = (float *) calloc(sizeof(float), size+1);
    *cost = (float *) calloc(sizeof(float), size+1);

    /* Bail out if memory allocation fails, fgError never returns */

    if (!(*sint) || !(*cost))
    {
        free(*sint);
        free(*cost);
        //fgError("Failed to allocate memory in fghCircleTable");
    }

    /* Compute cos and sin around the circle */

    (*sint)[0] = 0.0;
    (*cost)[0] = 1.0;

    for (i=1; i<size; i++)
    {
        (*sint)[i] = sin(angle*i);
        (*cost)[i] = cos(angle*i);
    }

    /* Last sample is duplicate of the first */

    (*sint)[size] = (*sint)[0];
    (*cost)[size] = (*cost)[0];
}

#ifdef DXRENDER
#else
void glutSolidSphere(GLdouble radius, GLint slices, GLint stacks)
{
    int i,j;

    /* Adjust z and radius as stacks are drawn. */

    float z0,z1;
    float r0,r1;

    /* Pre-computed circle */

    float *sint1,*cost1;
    float *sint2,*cost2;

    //FREEGLUT_EXIT_IF_NOT_INITIALISED ("glutSolidSphere");

    fghCircleTable(&sint1,&cost1,-slices);
    fghCircleTable(&sint2,&cost2,stacks*2);

    /* The top stack is covered with a triangle fan */

    z0 = 1.0;
    z1 = cost2[(stacks>0)?1:0];
    r0 = 0.0;
    r1 = sint2[(stacks>0)?1:0];

    glBegin(GL_TRIANGLE_FAN);

        glNormal3d(0,0,1);
        glVertex3d(0,0,radius);

        for (j=slices; j>=0; j--)
        {
            glNormal3d(cost1[j]*r1,        sint1[j]*r1,        z1      );
            glVertex3d(cost1[j]*r1*radius, sint1[j]*r1*radius, z1*radius);
        }

    glEnd();

    /* Cover each stack with a quad strip, except the top and bottom stacks */

    for(i=1; i<stacks-1; i++)
    {
        z0 = z1; z1 = cost2[i+1];
        r0 = r1; r1 = sint2[i+1];

        glBegin(GL_QUAD_STRIP);

            for(j=0; j<=slices; j++)
            {
                glNormal3d(cost1[j]*r1,        sint1[j]*r1,        z1      );
                glVertex3d(cost1[j]*r1*radius, sint1[j]*r1*radius, z1*radius);
                glNormal3d(cost1[j]*r0,        sint1[j]*r0,        z0      );
                glVertex3d(cost1[j]*r0*radius, sint1[j]*r0*radius, z0*radius);
            }

        glEnd();
    }

    /* The bottom stack is covered with a triangle fan */

    z0 = z1;
    r0 = r1;

    glBegin(GL_TRIANGLE_FAN);

        glNormal3d(0,0,-1);
        glVertex3d(0,0,-radius);

        for (j=0; j<=slices; j++)
        {
            glNormal3d(cost1[j]*r0,        sint1[j]*r0,        z0      );
            glVertex3d(cost1[j]*r0*radius, sint1[j]*r0*radius, z0*radius);
        }

    glEnd();

    /* Release sin and cos tables */

    free(sint1);
    free(cost1);
    free(sint2);
    free(cost2);
}
#endif

void DisableRotation(NxActorWrapper* a, bool justY)
{
	if(justY)
	{
		a->raiseBodyFlag(NX_BF_FROZEN_ROT_X);
		a->raiseBodyFlag(NX_BF_FROZEN_ROT_Z);
	}
	else 
		a->raiseBodyFlag(NX_BF_FROZEN_ROT);
}

NxActorWrapper* CreateCuboidOri(const Mat34& pose, float l, float w, float h, 
					  bool staticActor, 
					  const Vec3 & initial_velocity, 
					  float density)
{
	NxActorWrapper *aWrap = new NxActorWrapper(staticActor);
	
	aWrap->setLinearVelocity(initial_velocity);
	aWrap->setDims(Vec3(l,w,h));
	aWrap->setGlobalPose(pose);

	return aWrap;
}

NxActorWrapper* CreateCuboid(const Vec3 pos, float l, float w, float h, 
					  bool staticActor, 
					  const Vec3 initial_velocity, 
					  float density)
{
	Mat33 id;
	id.id();

	return CreateCuboidOri(Mat34(id, pos), l, w, h, staticActor, initial_velocity, density);
}


// DONT USE THIS (allocate memory for a new actor manually)
NxActorWrapper* CreateBumpIcon( Vec3 pos, Vec3 dims, int textureNum, bool staticActor /*= false*/, const Vec3 & initial_velocity /*= Vec3(0,0,0)*/, float density /*= globalSettings.ACTOR_DENSITY*/, bool fadeIn /*= false*/ )
{
	
	// nothing should call this function, its defunct!!
	assert(false);
	Actor* data = new Actor;

	// Density is not used
	// Static Actor is not used

	data->setGlobalPosition(pos);
	data->setDims(dims);
	data->setLinearVelocity(initial_velocity);

	if(GLOBAL(settings).AxisAlignedMode) data->setGlobalOrientation(GLOBAL(straightIconOri));

	if(fadeIn)
	{
		data->fadeIn();
	}

	return data;
}

//Returns first index of value in vector vec, if it exists (use default == operator)
//-1 if it doesn't exist

float AngleBetweenVectors(Vec3 one, Vec3 two)
{
	one.normalize();
	two.normalize();

	float cang = one.x * two.x + one.y * two.y; // (cos angle)
	float sang = one.x * two.y - one.y * two.x; // (sin angle)

	float ang = acos(cang);

	if (sang >= 0.0)
		return ang;
	else
		return -ang; 
}

float maxVec3Dim(const Vec3 & v)
{
	return max(v.x, max(v.y, v.z));
}

//standard Lerp funciton

float ease(float t)
{
	return (sin(t * PI - (PI / 2.0f)) + 1.0f) / 2.0f;
}

float softEase(float t)
{
	return (sin(pow(t, 0.6f) * PI - (PI / 2.0f)) + 1.0f) / 2.0f;
}

bool approxEqual(float a, float b, float tolerance)
{
	if(a >= b-(tolerance/2.0) && a <= b+(tolerance/2.0))
		return true;

	else return false;
}

bool approxEqual(Vec3 a, Vec3 b, float tolerance)
{
	if(approxEqual(a.x, b.x, tolerance) && approxEqual(a.y, b.y, tolerance) && 
			approxEqual(a.z, b.z, tolerance))
		return true;

	else return false;
}

//Line-Line Intersection.  Lines are defined like a ray.  If Line1 and Line2 don't intersect, t1 and t2 are the points of closest approach
//Here Line means infinite line
void LineLineIntersection(Vec3 o1, Vec3 dir1, Vec3 o2, Vec3 dir2, float & t1, float & t2)
{
	//normalize dir's
	Vec3 oldDir1=dir1;
	Vec3 oldDir2=dir2;
	dir1.normalize();  //innefficient.  i could expect that these dir's will be normalized before entering the function.  
	dir2.normalize();
	
	Mat33 m1;
	Vec3 crossDirs;
	crossDirs= dir1 ^ dir2;
	float crossDirsMagSq= crossDirs.magnitudeSquared();

	if(crossDirs.magnitudeSquared() == 0) 
	{
		consoleWrite("Rays are parallel! Haven't Updated t1,t2's\n");
		return;
	}

	m1.setColumn(0, o2-o1);
	m1.setColumn(1, dir2);
	m1.setColumn(2, crossDirs);
	t1= m1.determinant() / crossDirsMagSq;
	t1/=oldDir1.magnitude();

	m1.setColumn(1, dir1);
	t2= m1.determinant() / crossDirsMagSq;
	t2/=oldDir2.magnitude();
}

//Find closest point on a line from point 'pt'
//Algorithm:  Paul Bourke: http://www.lems.brown.edu/vision/courses/medical-imaging-1999/assignments/point-line.html
Vec3 ClosestPointOnLineToPoint(Vec3 o, Vec3 dir, Vec3 pt)
{
	Vec3 p1,p2,p3;

	p1=o;
	p2=o+dir;
	p3=pt;

	float u= (p3.x-p1.x)*(p2.x-p1.x) + (p3.y-p1.y)*(p2.y-p1.y) + (p3.z-p1.z)*(p2.z-p1.z);
	u=u/(p2-p1).magnitudeSquared();

	Vec3 p= p1+ u*(p2-p1);

	return p;
}


//Check to see if a Ray (p, dir) intersects a line-segment (between points a and b).  
//If so return TRUE and the intersectionPoint in intPoint
//If lines are parallel return FALSE 
//Note: Ray is NOT A LINE.  It starts at point P and continues in dir direction.
bool RayLineSegmentIntersection(Vec3 p, Vec3 dir, Vec3 a, Vec3 b, Vec3 & intPoint, float & T2)
{
    Vec3 dir2=b-a;
	//float maxT2=dir2.magnitude();  //maxT2 should be 1!
	Vec3 crossDirs;
	crossDirs.cross(dir, dir2);

	if(crossDirs.magnitudeSquared() == 0) //Line & Line Segment are parallel
	{
		consoleWrite("Line & Line Segment are parallel\n");
		intPoint=Vec3(-1,-1,-1);
		return false;
	}

	float t1, t2;
	LineLineIntersection(p, dir, a, dir2, t1, t2);
	T2=t2;

	//if lines intersect, determine intersection point.  Ensure intPoint lies within segment, not just the infinite line created by 'a' and dir2
	Vec3 p1=p + dir*t1;
	Vec3 p2=a + dir2*t2;
	//printVec(p1);
	//printVec(p2);
	if(t2>=0 && t2<=1.0 && t1>=0 && approxEqual(p1, p2, 0.05f))
	{
		intPoint=p1;
		return true;
	}
	else return false;
   
}

//Return true if line segment a-b intersects line segment c-d
//Code taken from RayLineSegmentIntersection
bool LineSegLineSegIntersection(Vec3 a, Vec3 b, Vec3 c, Vec3 d)
{
    Vec3 dir1=b-a;
	Vec3 dir2=d-c;
	Vec3 crossDirs;
	crossDirs.cross(dir1, dir2);

	if(crossDirs.magnitudeSquared() == 0) //Line & Line Segment are parallel
	{
		//consoleWrite("LineSegLineSegIntersection():  Both Line Segments are parallel.  ");
		return false;
	}

	float t1, t2;
	LineLineIntersection(a, dir1, c, dir2, t1, t2);

	//if lines intersect, determine intersection point.  Ensure intPoint lies within segment, not just the infinite line created by 'a' and dir2
	Vec3 p1=a + dir1*t1;
	Vec3 p2=c + dir2*t2;

	if(t2>=0.0f && t2<=1.0f && t1>=0.0f && t1<=1.0f && approxEqual(p1, p2, 0.05f))
	{
		//intPoint=p1;
		return true;
	}
	else return false;

}

//return the distance between the ray and the point
float RayPointDistSq(Vec3 p, Vec3 rayP, Vec3 rayDir)
{
	float t= (rayDir | (p-rayP))/(rayDir|rayDir);

	//forget about distances that are behind the ray
	if(t<0)
		return -1;

	return (p-(rayP+(t*rayDir))).magnitudeSquared();
}

// Returns a tuple containing the closest point the given ray
// intersects the bumptop desktop
tuple<int, NxReal, Vec3, NxPlane> RayIntersectDesktop( Ray ray, float padding /*= 0.0f*/ )
{
	// Get bumptop desktop
	Box desktopBox = GetDesktopBox();
	ray.dir.normalize();
	
	// Represent desktop as planes
	NxPlane planes[6];
	bool success = desktopBox.computePlanes(planes);	
	NxReal dist[6];
	Vec3 pointOnPlane[6];
	
	// Intersect each plane with given ray
	for (int i = 0;i<6;i++)
	{
		planes[i].d += padding;
		pointOnPlane[i].set(0.0f);
		dist[i] = NX_MAX_REAL;
		if (NxRayPlaneIntersect(ray, planes[i], dist[i], pointOnPlane[i]))
		{
			// ignore planes behind the ray
			if (dist[i] < 0.05f)
				dist[i] = NX_MAX_REAL;	// default value 
		}
	}

	// Determine which point is closet to the camera
	int minIndex = 0;						
	for (int i = 1;i<6;i++)
	{
		if (dist[i] < dist[minIndex])
			minIndex = i;
	}

	// Set minIndex to the index that matches GLOBAL(Walls) variable
	int wallIndex;
	if (minIndex == 5)
		wallIndex = 0;
	else if (minIndex == 0)
		wallIndex = 2;
	else if (minIndex == 4)
		wallIndex = 1;
	else if (minIndex == 1)
		wallIndex = 3;
	else
		wallIndex = -1;

	return make_tuple(wallIndex, dist[minIndex], pointOnPlane[minIndex], planes[minIndex]);
}

//Given v as a local-space coordinate, finds out its equivalent world-space coordinate
//Return a vector of points for a peeled poly described by mouse-entry point P, current mouse-point P_, and polygon points ps in CCW order
//-reflectedPart is the reflected portion of the polygon
//Assume's all points lie in XY plane (Z=0).  Because rotation about Y axis.
//Return False if it fails
void PeeledPoly(Vec3List ps, Vec3 P, Vec3 P_, Vec3List & peeledPs, Vec3List & uvs, Vec3List & reflectedPart)
{
	float low_t,high_t;
	Vec3 PtoP_=P_-P;
	Vec3 Pmid= ((0.5)*PtoP_) + P;

	Vec3 orthogAxis(ps[2]-ps[1] ^ ps[0]-ps[1]  );
	orthogAxis.normalize();
	//Quat B0rot= Quat(90, Vec3(0,1,0));  //To make more general, compute the orthogonal axis to rotate around by taking cross product of line-segments in ps.
	//Quat B1rot= Quat(-90, Vec3(0,1,0));
	//consoleWrite("this shuld be (0,1,0):  ");
	//printVec(orthogAxis);
	Quat B0rot= Quat(90, orthogAxis);  
	Quat B1rot= Quat(-90, orthogAxis);

	Vec3 B0Vec=PtoP_;
	Vec3 B1Vec=PtoP_;
	B1rot.rotate(B1Vec);
	B0rot.rotate(B0Vec);  //i think B0 should just be equal to -1*B1. 
	
/*	//********DEBUG
	//glColor3f(0.5f,0.5f,0);
	//drawLine(Pmid+Vec3(0,2,0), Pmid+B1Vec +Vec3(0,2,0), 2);
	glColor3f(0,0.5f,0.5f);
	drawLine(Pmid+Vec3(0,2,0), Pmid+B0Vec +Vec3(0,2,0), 2);

	glColor3f(1,0,1);
	drawLine(ps[0], ps[1], 2);

	if(printStuff)
	{	
		printVec(Pmid);
		printVec(Pmid+B0Vec);
		printVec(ps[0]);
		printVec(ps[1]);

		consoleWrite("\n\n");
	}
/*	glBegin(GL_LINE_LOOP);
	glColor3f(1,0,1);
	for(int i=0; i<ps.size(); i++)
		glVertex3fv((ps[i]+Vec3(0,1,0)).get());
	glEnd();
	*/
	//********END DEBUG

	//find out which linesegment of the polygon Pmid+t*B1Vec intersects with.
	int Low=-1;
	int High=-1;
	Vec3 Blow,Bhigh;
	
	//Find Low
	if(RayLineSegmentIntersection(Pmid, B0Vec, ps[0], ps[1], Blow, low_t))
		Low=0;
	else if(RayLineSegmentIntersection(Pmid, B0Vec, ps[1], ps[2], Blow, low_t))
		Low=1;
	else if(RayLineSegmentIntersection(Pmid, B0Vec, ps[2], ps[3], Blow, low_t))
		Low=2;
	else if(RayLineSegmentIntersection(Pmid, B0Vec, ps[3], ps[4], Blow, low_t))
		Low=3;

	//Find High
	if(RayLineSegmentIntersection(Pmid, B1Vec, ps[0], ps[1], Bhigh, high_t))
		High=1;
	else if(RayLineSegmentIntersection(Pmid, B1Vec, ps[1], ps[2], Bhigh, high_t))
		High=2;
	else if(RayLineSegmentIntersection(Pmid, B1Vec, ps[2], ps[3], Bhigh, high_t))
		High=3;
	else if(RayLineSegmentIntersection(Pmid, B1Vec, ps[3], ps[4], Bhigh, high_t))
		High=4;

/*
	glDisable(GL_DEPTH_TEST);
	glLineWidth(15.0);
	glBegin(GL_LINES);
	glColor3f(1,0,1);
	glVertex3fv(Blow.get());
	glColor3f(1,1,0);
	glVertex3fv(Bhigh.get());
	glEnd();
	//drawLine(Blow, Bhigh, 15);
	glEnable(GL_DEPTH_TEST);
	glLineWidth(1);

	if(Low==-1 && High==3)
	{
		consoleWrite("didn't intersect Ray: ");
		printVec(Pmid);
		printVec(B0Vec);

		consoleWrite("with segment: ");
		printVec(ps[
	}
*/
	//DEBUG
//	if(printStuff)
//		consoleWrite("DEBUG:  Low: %d, High: %d\n\n", Low, High);

    //Return appropriate points for specific high/low case (see scribbles on paper to determine cases)
	peeledPs.clear();
	reflectedPart.clear();
	uvs.clear();

	//4 Main corner Cases
	if(Low==0 && High==2)
	{
		peeledPs.push_back(ps[0]);
		peeledPs.push_back(Blow);
		peeledPs.push_back(Bhigh);
		peeledPs.push_back(ps[2]);
		peeledPs.push_back(ps[3]);

		//UVs
		uvs.push_back(Vec3(0,0,0));
		uvs.push_back(Vec3(low_t,0,0));
		uvs.push_back(Vec3(1,high_t,0));
		uvs.push_back(Vec3(1,1,0));
		uvs.push_back(Vec3(0,1,0));

		reflectedPart.push_back(Blow);
		reflectedPart.push_back(Bhigh);
		reflectedPart.push_back(P_);
	}
	else if(Low==1 && High==3)
	{
		peeledPs.push_back(ps[0]);
		peeledPs.push_back(ps[1]);
		peeledPs.push_back(Blow);
		peeledPs.push_back(Bhigh);
		peeledPs.push_back(ps[3]);

		//UVs
		uvs.push_back(Vec3(0,0,0));
		uvs.push_back(Vec3(1,0,0));
		uvs.push_back(Vec3(1,low_t,0));
		uvs.push_back(Vec3(1-high_t,1,0));
		uvs.push_back(Vec3(0,1,0));

		reflectedPart.push_back(P_);
		reflectedPart.push_back(Blow);
		reflectedPart.push_back(Bhigh);
	}
	else if(Low==2 && High==4)
	{
		peeledPs.push_back(ps[0]);
		peeledPs.push_back(ps[1]);
		peeledPs.push_back(ps[2]);
		peeledPs.push_back(Blow);
		peeledPs.push_back(Bhigh);

		//UVs
		uvs.push_back(Vec3(0,0,0));
		uvs.push_back(Vec3(1,0,0));
		uvs.push_back(Vec3(1,1,0));
		uvs.push_back(Vec3(1-low_t,1,0));
		uvs.push_back(Vec3(0,1-high_t,0));

		reflectedPart.push_back(Bhigh);
		reflectedPart.push_back(P_);
		reflectedPart.push_back(Blow);
	}
	else if(Low==3 && High==1)
	{
		peeledPs.push_back(Bhigh);
		peeledPs.push_back(ps[1]);
		peeledPs.push_back(ps[2]);
		peeledPs.push_back(ps[3]);
		peeledPs.push_back(Blow);

		//UVs
		uvs.push_back(Vec3(high_t,0,0));
		uvs.push_back(Vec3(1,0,0));
		uvs.push_back(Vec3(1,1,0));
		uvs.push_back(Vec3(0,1,0));
		uvs.push_back(Vec3(0,1-low_t,0));

		reflectedPart.push_back(Bhigh);
		reflectedPart.push_back(P_);
		reflectedPart.push_back(Blow);
	}
	//Next 4 Cases:  Peeling Parallel, with 2edges deleted
	else if(Low==3 && High==2)
	{
		peeledPs.push_back(Blow);
		peeledPs.push_back(Bhigh);
		peeledPs.push_back(ps[2]);
		peeledPs.push_back(ps[3]);

		//UVs
		uvs.push_back(Vec3(0,1-low_t,0));
		uvs.push_back(Vec3(1,high_t,0));
		uvs.push_back(Vec3(1,1,0));
		uvs.push_back(Vec3(0,1,0));

		reflectedPart.push_back(Blow);
		reflectedPart.push_back(Bhigh);
		reflectedPart.push_back(Bhigh+(Pmid-P));
		reflectedPart.push_back(Blow+(Pmid-P));
	}
	else if(Low==1 && High==4)
	{
		peeledPs.push_back(ps[0]);
		peeledPs.push_back(ps[1]);
		peeledPs.push_back(Blow);
		peeledPs.push_back(Bhigh);

		//UVs
		uvs.push_back(Vec3(0,0,0));
		uvs.push_back(Vec3(1,0,0));
		uvs.push_back(Vec3(1,low_t,0));
		uvs.push_back(Vec3(0,1-high_t,0));

		reflectedPart.push_back(Bhigh+(Pmid-P));
		reflectedPart.push_back(Blow+(Pmid-P));
		reflectedPart.push_back(Blow);
		reflectedPart.push_back(Bhigh);
	}
	else if(Low==0 && High==3)
	{
		peeledPs.push_back(ps[0]);
		peeledPs.push_back(Blow);
		peeledPs.push_back(Bhigh);
		peeledPs.push_back(ps[3]);

		//UVs
		uvs.push_back(Vec3(0,0,0));
		uvs.push_back(Vec3(low_t,0,0));
		uvs.push_back(Vec3(1-high_t,1,0));
		uvs.push_back(Vec3(0,1,0));

		reflectedPart.push_back(Blow+(Pmid-P));
		reflectedPart.push_back(Blow);
		reflectedPart.push_back(Bhigh);
		reflectedPart.push_back(Bhigh+(Pmid-P));
	}
	else if(Low==2 && High==1)
	{
		peeledPs.push_back(Bhigh);
		peeledPs.push_back(ps[1]);
		peeledPs.push_back(ps[2]);
		peeledPs.push_back(Blow);

		//UVs
		uvs.push_back(Vec3(high_t,0,0));
		uvs.push_back(Vec3(1,0,0));
		uvs.push_back(Vec3(1,1,0));
		uvs.push_back(Vec3(1-low_t,1,0));

		reflectedPart.push_back(Bhigh);
		reflectedPart.push_back(Bhigh+(Pmid-P));
		reflectedPart.push_back(Blow+(Pmid-P));
		reflectedPart.push_back(Blow);
	}
	//Last 4 Cases: Only a corner left (you're peeling 3 corners)
	else if(Low==1 && High==1)
	{
		peeledPs.push_back(Bhigh);
		peeledPs.push_back(ps[1]);
		peeledPs.push_back(Blow);

		//UVs
		uvs.push_back(Vec3(high_t,0,0));
		uvs.push_back(Vec3(1,0,0));
		uvs.push_back(Vec3(1,low_t,0));

		//this seems WRONG, but its Good!
		reflectedPart.push_back(Bhigh);
		reflectedPart.push_back(ps[0]+(Pmid-P));
		reflectedPart.push_back(P_);
		reflectedPart.push_back(ps[2]+(Pmid-P));
		reflectedPart.push_back(Blow);
	}
	else if(Low==2 && High==2)
	{
		peeledPs.push_back(Bhigh);
		peeledPs.push_back(ps[2]);
		peeledPs.push_back(Blow);

		//UVs
		uvs.push_back(Vec3(1,high_t,0));
		uvs.push_back(Vec3(1,1,0));
		uvs.push_back(Vec3(1-low_t, 1,0));

		//this seems WRONG
		reflectedPart.push_back(Blow);
		reflectedPart.push_back(Bhigh);
		reflectedPart.push_back(ps[1]+(Pmid-P));
		reflectedPart.push_back(P_);
		reflectedPart.push_back(ps[3]+(Pmid-P));
	}
	else if(Low==3 && High==3)
	{
		peeledPs.push_back(Blow);
		peeledPs.push_back(Bhigh);
		peeledPs.push_back(ps[3]);

		//UVs
		uvs.push_back(Vec3(0,1-low_t,0));
		uvs.push_back(Vec3(1-high_t,1,0));
		uvs.push_back(Vec3(0,1,0));

		//this seems WRONG
		reflectedPart.push_back(Blow);
		reflectedPart.push_back(Bhigh);
		reflectedPart.push_back(ps[2]+(Pmid-P));
		reflectedPart.push_back(P_);
		reflectedPart.push_back(ps[0]+(Pmid-P));
	}
	else if(Low==0 && High==4)
	{
		peeledPs.push_back(ps[0]);
		peeledPs.push_back(Blow);
		peeledPs.push_back(Bhigh);

		//UVs
		uvs.push_back(Vec3(0,0,0));
		uvs.push_back(Vec3(low_t,0,0));
		uvs.push_back(Vec3(0,1-high_t,0));

		//this seems WRONG
		reflectedPart.push_back(Blow);
		reflectedPart.push_back(Bhigh);
		reflectedPart.push_back(ps[3]+(Pmid-P));
		reflectedPart.push_back(P_);
		reflectedPart.push_back(ps[1]+(Pmid-P));
	}

	//Error Case:  Return regular 4 Ps
	//if(Low==-1 || High==-1)
	else {
		peeledPs.push_back(ps[0]);
		peeledPs.push_back(ps[1]);
		peeledPs.push_back(ps[2]);
		peeledPs.push_back(ps[3]);

		uvs.push_back(Vec3(0,0,0));
		uvs.push_back(Vec3(1,0,0));
		uvs.push_back(Vec3(1,1,0));
		uvs.push_back(Vec3(0,1,0));
		return;
	}

}

#ifdef DXRENDER
#else
void DrawSphere(Vec3 center, float radius, int slicesAndStacks)
{
	glPushMatrix();
	glTranslatef(center.x, center.y, center.z);
	glutSolidSphere(radius, slicesAndStacks, slicesAndStacks);
	glPopMatrix();
}

void DrawEllipse(Vec3 center, float height, float width)
{
	glPushMatrix();
	glTranslatef(center.x, center.y, center.z);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0f, 0.0f, 0.0f);
	float x, y, z;
	for (int t = 0; t <= 360; t += 3)
	{
		x = (height/2.0f) * cosf(t * (PI / 180));
		y = (width/2.0f) * sinf(t * (PI / 180));
		z = 0;
		glVertex3f(x, y, z);
	}
	glEnd();
	glPopMatrix();
}
#endif

TextOverlay* CreateGoProFingerMessage()
{
	OverlayLayout* overlay = new OverlayLayout();
	TextOverlay* text = new TextOverlay(QT_TRANSLATE_NOOP("BtUtilStrings", "Buy Pro!"));
	overlay->getStyle().setOffset(Vec3(0.5f, 0.5f, 0.0f));
	text->getStyle().setAlpha(1.0f);
	overlay->addItem(text);
	scnManager->registerOverlay(overlay);
	return text;
}

bool isPointInPolygon(const Vec3 &point, Vec3List &polygonPoints)
{
	uint polySides = polygonPoints.size();
	int i, j = polySides - 1;
	bool oddNodes = false;
	Vec3 actorPos;

	// Grab the objects Center
	actorPos = WorldToClient(point, true, true);

	// Check against every side of the growing lasso menu
	for (i = 0; i < polySides; i++)
	{
		Vec3 posI = polygonPoints[i];
		Vec3 posJ = polygonPoints[j];

		// Check if any of the points in the box intersect the object
		if (posI.y< actorPos.y && posJ.y >= actorPos.y || posJ.y < actorPos.y && posI.y >= actorPos.y) 
		{
			if (posI.x + (actorPos.y - posI.y) / (posJ.y - posI.y) * (posJ.x - posI.x) < actorPos.x)
			{
				// Toggle the size as it runs the lasso vector
				oddNodes = !oddNodes;
			}
		}

		j = i;
	}
	return oddNodes;
}

// Heron's Formula. Reference: http://mathworld.wolfram.com/HeronsFormula.html 
float calculateAreaOfTriangle(const Vec3 &firstPoint, const Vec3 &secondPoint, const Vec3 &thirdPoint)
{
	Vec3 firstSide = firstPoint - secondPoint;
	Vec3 secondSide = secondPoint - thirdPoint;
	Vec3 thirdSide = thirdPoint - firstPoint;
	float a = firstSide.magnitude();
	float b = secondSide.magnitude();
	float c = thirdSide.magnitude();

	float halfOfPerimeter = (a + b + c) / 2.0f;

	float areaSquared = halfOfPerimeter * (halfOfPerimeter - a) * (halfOfPerimeter - b) * (halfOfPerimeter - c);
	return sqrtf(areaSquared);
}

float calculateAreaOfPolygon(const Vec3List &polygonPoints)
{
	assert(polygonPoints.size() >= 3);
	
	//The point from which triangles will fan out to the other points in the polygon
	Vec3 fanPoint = polygonPoints[0];
	
	float totalArea = 0.0f;

	Vec3List::const_iterator pointsIt;
	for(pointsIt = (polygonPoints.begin() + 2); pointsIt != polygonPoints.end(); pointsIt++)
	{
		totalArea += calculateAreaOfTriangle(fanPoint, *(pointsIt - 1), *pointsIt);
	}
	return totalArea;
}

Vec3 calculatePolygonCentroid(const Vec3List &polygonPoints)
{
	uint polygonSize = polygonPoints.size();
	Vec3List::const_iterator pointsIt;
	Vec3 sumOfPoints = Vec3(0.0f);

	for(pointsIt = polygonPoints.begin(); pointsIt != polygonPoints.end(); pointsIt++)
	{
		sumOfPoints += *pointsIt;
	}

	return sumOfPoints / (float)polygonSize;
}

Vec3List createEllipse(Vec3 center, float width, float height)
{
	center.setz(0.0f);
	Vec3List vertices;
	for (int t = 0; t <= 360; t += 15)
	{
		Vec3 vertex;
		vertex.setx((height/2.0f) * cosf(t));
		vertex.sety((width/2.0f) * sinf(t));
		vertex.setz(0.0f);
		vertices.push_back(center + vertex);
	}
	return vertices;
}

void unpick()
{	
	sel->setPickedActor(NULL);
}

FileSystemActor *GetFileSystemActor(NxActorWrapper *a)
{
	if (a && a->getUserDataType() == UserDataAvailable)
	{
		Actor *actor = GetBumpActor(a);

		if (actor && actor->isActorType(FileSystem))
		{
			return (FileSystemActor *) actor;
		}else{
			// If you fail here, its because you passed in an actor thats NOT a FileSystemActor!!
			assert(false);
		}
	}

	return NULL;
}

Actor *GetBumpActor(NxActorWrapper *a)
{
	if (a && a->getUserDataType() == UserDataAvailable) return (Actor *) a;
	return NULL;
}

BumpObject *GetBumpObject(NxActorWrapper *a)
{
	if (a && a->getUserDataType() == UserDataAvailable) return (BumpObject*) a;
	return NULL;
}

QSet<BumpObject *> getAllObjectsUnderCursor(int x, int y)
{	
	Vec3 v, w, dir;
	Ray rayStab;
	NxRaycastHit hit;
	MultiRaycast rayCastReport;

	// Create the ray
	window2world(x, y, v, w);
	dir = w - v;
	dir.normalize();
	rayStab = Ray(v, dir);

	// Raycast and see which items we hit
	GLOBAL(gScene)->raycastAllShapes(rayStab, rayCastReport, NX_ALL_SHAPES);

	vector<BumpObject *> objs = rayCastReport.getLastRaycast();
	QSet<BumpObject *> objsSet;
	for (int i = 0; i < objs.size(); ++i)
	{
		objsSet.insert(objs[i]);
	}
	return objsSet;
}

//interaction
// return values: pickedActor, pickedBumpObject, stabPointActorSpace
tuple<NxActorWrapper*, BumpObject*, Vec3> pick(int x, int y)
{
	Vec3 line_start, line_end;
	Ray worldRay;
	NxRaycastHit hit;
	NxActor* a = NULL;
	NxActorWrapper *pickedActor = NULL;
	BumpObject *pickedBumpObject = NULL;
	Vec3 stabPointActorSpace;

	window2world(x, y, line_start, line_end);
	worldRay.orig	= line_start;
	worldRay.dir	= line_end - line_start;
	worldRay.dir.normalize();

	PickRaycast rayCast;
	GLOBAL(gScene)->raycastAllShapes(worldRay, rayCast, NX_ALL_SHAPES);
	pickedBumpObject = rayCast.getObject();
	hit = rayCast.getRaycastHit(worldRay);

	if (!pickedBumpObject) {
		
		Cluster *c = bubbleManager->pickCluster(x , y);
		if (c)
		{
			pickedBumpObject = (BumpObject*)c;
		}
		else
			return make_tuple((NxActorWrapper*)NULL, (BumpObject*)NULL,Vec3());
	}

	//else, just drag it:
	Mat34 mp = pickedBumpObject->getGlobalPoseReference();
	mp.multiplyByInverseRT(hit.impact, stabPointActorSpace);

	return make_tuple((NxActorWrapper *) pickedBumpObject, pickedBumpObject, stabPointActorSpace);
}

//interaction
void pickAndSet( int x, int y )
{
	// skip re-picking if the picked actor is already set
	if (sel->getPickedActor())
		return;

	tuple<NxActorWrapper*, BumpObject*, Vec3> t = pick(x, y);
	NxActorWrapper* pickedActor = t.get<0>();
	BumpObject* pickedBumpObject = t.get<1>();
	Vec3 stabPointActorSpace = t.get<2>();

	sel->setPickedActor(pickedBumpObject);

	if (pickedActor != NULL)
	{
		sel->setStabPointActorSpace(stabPointActorSpace);
	}
}

#ifdef DXRENDER
void window2world(int x, int y, Vec3 & v, Vec3 & w)
{
	dxr->window2world(x, y, v, w);
}
#else
void window2world(int x, int y, Vec3 & v, Vec3 & w)
{
	// we hit this function a lot, so to save on the calls, we will locally use the singleton
	Camera * camera = cam;
    GLint * viewport = camera->glViewport();
	GLdouble * projmatrix = camera->glProjectionMatrix();
	GLdouble * mvmatrix = camera->glModelViewMatrix();
	GLdouble wx(0), wy(0), wz(0);

	// convert from window coordinate space to ogl screen coordinate space
	// viewport [3] is the height of the window in pixels
	y = NxMath::max(0, viewport[3] - (GLint) y - 1);

	// we take the unprojection of the point to the far clipping plane
	// and the ray from the eye to that point becomes our ray
	v = cam->getEye();

	// Technically, we should use the depth buffer to determine the z, but that is
	// slow and blocking...
	GLdouble z = 1.0f;
	// glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_DOUBLE, &z);
	if (GL_TRUE == gluUnProject((GLdouble) x, (GLdouble) y, (GLdouble) z, mvmatrix, projmatrix, viewport, &wx, &wy, &wz))
		w.set(float(wx), float(wy), float(wz));
	else
		w.zero();
}
#endif

bool ToEulerAnglesXYZ(const Mat33 & MatrixEntry, float &rfXAngle, float &rfYAngle, float &rfZAngle)
{
    // rot =  cy*cz          -cy*sz           sy
    //        cz*sx*sy+cx*sz  cx*cz-sx*sy*sz -cy*sx
    //       -cx*cz*sy+sx*sz  cz*sx+cx*sy*sz  cx*cy

	float Entry[3][3];
	Vec3 temp;

	// They dont let you have access to the Matrix Itself
	MatrixEntry.getColumn(0, temp);
	Entry[0][0] = temp.x;
	Entry[0][1] = temp.y;
	Entry[0][2] = temp.z;
	MatrixEntry.getColumn(1, temp);
	Entry[1][0] = temp.x;
	Entry[1][1] = temp.y;
	Entry[1][2] = temp.z;
	MatrixEntry.getColumn(2, temp);
	Entry[2][0] = temp.x;
	Entry[2][1] = temp.y;
	Entry[2][2] = temp.z;	


/** this conversion uses conventions as described on page:
*   http://www.euclideanspace.com/maths/geometry/rotations/euler/index.htm
*   Coordinate System: right hand
*   Positive angle: right hand
*   Order of euler angles: heading first, then attitude, then bank
*   matrix row column ordering:
*   [m00 m01 m02]
*   [m10 m11 m12]
*   [m20 m21 m22]*/

    // Assuming the angles are in radians.
	if (Entry[1][0] > 0.998) { // singularity at north pole
		rfXAngle = atan2(Entry[0][2], Entry[2][2]);
		rfYAngle = PI / 2;
		rfZAngle = 0;
		return true;
	}
	if (Entry[1][0] < -0.998) { // singularity at south pole
		rfXAngle = atan2(Entry[0][2], Entry[2][2]);
		rfYAngle = -PI / 2;
		rfZAngle = 0;
		return true;
	}

	rfXAngle = atan2(-Entry[2][0], Entry[0][0]);
	rfYAngle = atan2(-Entry[1][2], Entry[1][1]);
	rfZAngle = asin(Entry[1][0]);
	return true;

	/*
    if (Entry[2] < 1.0)
    {
        if (Entry[2] > 1.0)
        {
            rfXAngle = atan2(-Entry[5],Entry[8]);
            rfYAngle = asin(Entry[2]);
            rfZAngle = atan2(-Entry[1],Entry[0]);
            return true;
        }
        else
        {
            // WARNING.  Not unique.  XA - ZA = -atan2(r10,r11)
            rfXAngle = -atan2(Entry[3],Entry[4]);
            rfYAngle = -(PI / 2);
            rfZAngle = 0.0;
            return false;
        }
    }
    else
    {
        // WARNING.  Not unique.  XAngle + ZAngle = atan2(r10,r11)
        rfXAngle = atan2(Entry[3],Entry[4]);
        rfYAngle = (PI / 2);
        rfZAngle = 0.0;
        return false;
    }*/
}

bool ToEulerAngleZ(const Mat33 & MatrixEntry, float &rfZAngle)
{
    // rot =  cy*cz          -cy*sz           sy
    //        cz*sx*sy+cx*sz  cx*cz-sx*sy*sz -cy*sx
    //       -cx*cz*sy+sx*sz  cz*sx+cx*sy*sz  cx*cy
	Vec3 temp;
	// They dont let you have access to the Matrix Itself
	MatrixEntry.getColumn(1, temp);
	const float Entry10 = temp.x;

/** this conversion uses conventions as described on page:
*   http://www.euclideanspace.com/maths/geometry/rotations/euler/index.htm
*   Coordinate System: right hand
*   Positive angle: right hand
*   Order of euler angles: heading first, then attitude, then bank
*   matrix row column ordering:
*   [m00 m01 m02]
*   [m10 m11 m12]
*   [m20 m21 m22]*/

    // Assuming the angles are in radians.
	if (Entry10 > 0.998) { // singularity at north pole
		rfZAngle = 0;
		return true;
	}
	if (Entry10 < -0.998) { // singularity at south pole
		rfZAngle = 0;
		return true;
	}

	rfZAngle = asin(Entry10);
	return true;
}

float DistanceBetweenPoints(Vec3 one, Vec3 two)
{
	return sqrt(pow(one.x - two.x, 2) + pow(one.z - two.z, 2));
}

//Interpolate a Mat34, by interpolating its individual entries.  (Not the "official" way to interoplate a matrix!!)
//This doesn't have much different an effect then the traditional method, but may be computationally cheaper since it avoids a slerp for the rotation. 
deque<Mat34> lerpMat34RangeRaw(const Mat34 & begin, const Mat34 & end, const int count, EaseFunctionType easeType)
{
	deque<Mat34> range;

	for (int i = 0; i < count; i++)
	{
		float t = float(i) / float(count - 1);

		// Apply Easing
		switch (easeType)
		{
		case Ease:
			t = ease(t);
			break;
		case SoftEase:
			t = softEase(t);
			break;
		default:
			break;
		}

		NxF32 matBegin[16];
		NxF32 matEnd[16];
		NxF32 matLerp[16];
		Mat34 lerped;

		begin.getColumnMajor44(matBegin);
		end.getColumnMajor44(matEnd);
		for(int j=0; j < 16; j++)
			matLerp[j]=lerp(matBegin[j], matEnd[j], t);

		lerped.setColumnMajor44(matLerp);

		range.push_back(lerped);
	}

	return range;

}

//Return vector of Desktop actors
//Not Static objects or widgets
vector<NxActorWrapper*> GetDesktopItems()
{
	
	vector<NxActorWrapper*> v;

	for (int i = 0; i < GLOBAL(activeNxActorList).size(); i++)
	{
		NxActorWrapper* actor = GLOBAL(activeNxActorList)[i];

		if (actor->getUserDataType() == UserDataAvailable)
		{
			BumpObject *obj = (BumpObject *) actor;

			if (obj->isBumpObjectType(BumpActor))
			{
				Actor *aData = GetBumpActor(actor);

				if (!(aData->isActorType(Invisible)) && 
					!(aData->isActorType(Temporary)))
				{
					v.push_back(obj);
				}
			}else if(obj->isBumpObjectType(BumpPile))
			{
				//v.push_back(obj->getActor());
			}
		}
	}

	return v;
}

//Is a desktop object.  Not a widget or wall (non-dynamic)
bool isDesktopItem(NxActorWrapper* actor)
{
	Actor* data=GetBumpActor(actor);
	bool rc = false;

	// Try to get the item and check to see if its a desktop item
	try 
	{
		if (actor->isDynamic() && (data!=NULL && !(data->isBumpObjectType(BumpWidget)) && !(data->isActorType(Invisible)) && !(data->isActorType(Temporary))))
		{
			rc = true;
		}

	}catch (char *e)
	{
		rc = false;
		consoleWrite(QString("CRITICAL ERROR [Line: %1] [File: %2] - %3\n").arg(__LINE__).arg(__FILE__).arg(e));
	}

	return rc;
}

//Fade and then DeleteActor().
Actor * FadeAndDeleteActor( Actor *obj )
{
	LOG_FUNCTION_REACHED();
		
	if (obj)
	{
		Actor * newObj = NULL;

		// Special case for thumbnails
		if (obj->isObjectType(ObjectType(BumpActor, FileSystem, Thumbnail)) || 
			obj->isObjectType(ObjectType(BumpActor, FileSystem, PhotoFrame)) ) 
		{
			newObj = new FileSystemActor();
			// we swap the thumbnail ids (so that it is removed _after_
			// the temporary actor)
			FileSystemActor *fsData = (FileSystemActor *) obj;
			if (fsData->isFileSystemType(PhotoFrame))
			{
				newObj->setTextureID("photoframe.empty");
			}
			else
			{
				((FileSystemActor *)newObj)->enableThumbnail(true, false);
				((FileSystemActor *)newObj)->setTextureID(fsData->getThumbnailID());
				((FileSystemActor *)newObj)->setThumbnailID(fsData->getThumbnailID());
			}
			fsData->setThumbnailID(QString());
		}
		else if (obj->isObjectType(ObjectType(BumpActor, Webpage)))
		{
			// delete the actual actor after animation finishes, since the pointer is still used
			newObj = obj; 
		}
		else
		{
			newObj = new Actor();
			// take on the object's texture id
			newObj->setTextureID(obj->getTextureID());
		}

		newObj->setText("fadeAndDeleteActor");
		newObj->hideText(true);
		newObj->setGlobalOrientation(obj->getGlobalOrientation());
		newObj->setGlobalPosition(obj->getGlobalPosition());
		newObj->setDims(obj->getDims());
		newObj->pushActorType(Temporary);
		newObj->setAlpha(obj->getAlpha());
		newObj->fadeOut();

		// Delete the actor, let the Temporary actor do the animation for us
		Pile * pile = NULL;
		if (obj->isParentType(BumpPile))
			pile = (Pile *) obj->getParent();
		
		if (!obj->isObjectType(ObjectType(BumpActor, Webpage)))
		{
			SAFE_DELETE(obj);
		}
		else if (pile)
		{
			// This is here because we handle WebActors a bit differently due to a dangling pointer issue.
			// We can't delete the object here, so the pile gets messed up later if we use conventional deletion
			// methods. So to circumvent that, just remove the WebActor from the pile it's in.
			pile->removeFromPile(obj);
		}

		if (pile)
			pile->updatePileState();

		// Use the animation manager to delete objects
		animManager->removeAnimation(newObj);
		animManager->addAnimation(AnimationEntry(newObj, (FinishedCallBack) DeleteActorAfterAnim, NULL, true));

		// disable collisions on the new object so we don't get the bounce when we pileize
		newObj->setCollisions(false);
		
		return newObj;
	}

	return NULL;
}

void *DeleteActorAfterAnim(AnimationEntry *animEntry)
{
	LOG_FUNCTION_REACHED();
	Actor *data = (Actor *) animEntry->getObject();
	SAFE_DELETE(data);
	return NULL;
}

void *SyncStickyNoteAfterResize(AnimationEntry *animEntry)
{
	FileSystemActor * fsActor = (FileSystemActor *) animEntry->getObject();
	if (fsActor->isFileSystemType(StickyNote))
	{
		StickyNoteActor * actor = (StickyNoteActor *) fsActor;
		actor->syncStickyNoteWithFileContents();
	}

	return NULL;
}

void DeletePile( Pile *p, bool deleteActors/*=false*/, bool fade/*=false*/, bool removeAnimation /*= true*/ )
{
	

	if (!p) return;

	//remove entry from piles vector of each item that was in this pile
	if (deleteActors)
	{
		// Remove the actors aswell, using fade or no fade
		for (int i = 0; i < p->getNumItems(); i++)
		{
			if (fade)
			{
				FadeAndDeleteActor((Actor *) (*p)[i]);
				(*p)[i]->setParent(NULL);
			}else{
				BumpObject *data = (*p)[i];
				SAFE_DELETE(data);
			}
		}
	}

	p->clear();

	SAFE_DELETE(p);
}

// This funciton moves selected files from thier current location into a Folder
NxActorWrapper * SoftPileToFolderIcon( Pile *pile, QString folderName )
{
	Pile *selectedPile = NULL;
	bool hasVirtualFolder = false;
	QString errorMessage(QT_TRANSLATE_NOOP("BtUtilStrings", "These items cannot be moved from the desktop into a folder:\n["));
	QStringList virtualFolders;

	for (int i = 0; i < pile->getNumItems(); i++)
	{
		Actor * a = (Actor *) (*pile)[i];

		if (!(a->isActorType(FileSystem)))
		{
			GLOBAL(gPause) = false;
			sel->setPickedActor(NULL);
			return NULL;
		}
	}

	// Search the pile for VIRTUAL folders
	for (int i = 0; i < pile->getNumItems(); i++)
	{
		if (!(*pile)[i]->isPilable(HardPile))
		{
			virtualFolders.append(((FileSystemActor *) (*pile)[i])->getFullPath());
			hasVirtualFolder = true;
		}
	}

	// If this pile has items that are VRTUAL, ask the user to remove them
	if (hasVirtualFolder)
	{
		errorMessage.append(virtualFolders.join(", "));
		errorMessage.append("]\n");
		errorMessage.append(QT_TRANSLATE_NOOP("BtUtilStrings", "\n\nCollapse anyway and leave these items unchanged?"));

		dlgManager->clearState();
		dlgManager->setPrompt(errorMessage);
		if (dlgManager->promptDialog(DialogYesNo))
		{
			for (int i = 0; i < pile->getNumItems(); i++)
			{
				FileSystemActor *fsData = (FileSystemActor *) (*pile)[i];

				// Check to see if this item is really a VRTUAL folder
				if (fsData->isFileSystemType(Virtual))
				{
					// Remove this item from the pile
					FreezeActor((*pile)[i], false);
					(*pile)[i]->wakeUp();
					pile->removeFromPile((*pile)[i]);
					i--;
				}
			}
		}else{
			// User aborted
			return NULL;
		}
	}

	QString setFolderName;
	if (pile->getText().isEmpty())
	{
		// if the pile has no name, then prompt for one
		QString newFolderName("Piled Stuff");
		dlgManager->clearState();
		dlgManager->setPrompt(BtUtilStr->getString("FolderizePrompt"));
		dlgManager->setText(newFolderName);

		// Prompt if no folder name has been passed into the function
		if(folderName.isNull())
		{
			if (dlgManager->promptDialog(DialogInput2))
				setFolderName = dlgManager->getText();
		} else {
			setFolderName = folderName;
		}
	}
	else
	{
		// use the pile's name otherwise
		setFolderName = pile->getText();
	}
	
	// If no folder name was passed in and the user canceled the dialog box then do not execute this block
	if(!setFolderName.isNull())
	{
		FileSystemPile *fsPile = new FileSystemPile();
		QString newPath = native(fsManager->getUniqueNewFolderPathInWorkingDirectory(setFolderName));
		if (fsPile->convert(pile, newPath))
		{
			fsManager->addObject(fsPile);
			fsPile->folderize();
		}else{
			DeletePile(fsPile, false, false, false);
		}
	}
	return NULL;
}

void * PinItemAfterAnim( AnimationEntry *entry )
{
	BumpObject *obj = (BumpObject *) entry->getObject();
	obj->onPin();

	return NULL;
}

void * EnableGravityAfterAnim(AnimationEntry * entry)
{
	BumpObject *obj = (BumpObject *) entry->getObject();
	obj->setGravity(true);

	return NULL;
}

boost::function<void(BumpObject *obj)> _onPinItemHandler;

void setOnPinItemHandler( boost::function<void(BumpObject *obj)> onPinItemHandler )
{
	_onPinItemHandler = onPinItemHandler;
}


void PinItemToWall(NxActorWrapper* u, bool breakable)
{
	bool reorientedToWall = true;

	if (u == NULL) return;

	// Don't allow temporary items to be pinned
	BumpObject *obj = (BumpObject *) u;
	Actor * actor = GetBumpActor(u);
	if (actor && actor->isActorType(Temporary))
		return;

	//when an object is pinned, it is no longer part of a cluster
	//remove from cluster
	if (obj->isParentType(BumpCluster))
	{
		((Cluster*)(obj->getParent()))->remove(obj);
		obj->setParent(NULL);		
	}

	if (!_onPinItemHandler.empty())
		_onPinItemHandler(obj);

	Vec3 pos = GLOBAL(PinPoint);
	Vec3 dims = obj->getDims();

	// straighten the pinned actor by using the default icon ori
	Quat newQori;
	GLOBAL(straightIconOri).toQuat(newQori);

	NxActorWrapper * pinWall = GLOBAL(PinWall);

	// In general, we don't allow piles to change orientation when pinned.
	// The hidden special case is a gridded hard pile named "Bump Shared Wall".
	bool isPile = obj->getObjectType() == ObjectType(BumpPile);
	if (isPile)
	{
		reorientedToWall = false; // By default piles are not reoriented

		if (obj->getObjectType() == ObjectType(BumpPile, HardPile, Grid))
		{
			reorientedToWall = true; // Piles with a "special name" ARE reoriented

			// When the pile is vertical, allow it to collide like a regular object
			repoManager->removeFromPileSpace(obj);
			newQori = pinWall->getGlobalOrientationQuat();
		}
	}
	else
	{
		// ensure that the orientation is correct (facing inwards)
		Mat33 ori = GLOBAL(straightIconOri);
		Vec3 orientedNormal = ori * Vec3(0, 0, -1);
		Vec3 toCenter = -obj->getGlobalPosition();
		if (orientedNormal.dot(toCenter) < 0)
		{
			// the actor is facing backwards, so flip it to the wall ori
			newQori = pinWall->getGlobalOrientationQuat();
			reorientedToWall = false;
		}		
	}

	if (reorientedToWall)
	{
		// Adjust the position of the object to be up against the wall
		Vec3 wallDims = pinWall->getDims();
		for(int i = 0; i < GLOBAL(Walls).size(); i++)
		{
			if (pinWall == GLOBAL(Walls)[i])
			{
				if (i < 2)		// top/bottom wall
					pos.z = pinWall->getGlobalPosition().z + (i == 0 ? -(dims.z + wallDims.z) : (dims.z + wallDims.z));
				else
					pos.x = pinWall->getGlobalPosition().x + (i == 2 ? (dims.z + wallDims.z) : -(dims.z + wallDims.z));
				break;
			}
		}
	}

	bool prevEnforcePinning = obj->getEnforcePinning();
	obj->setEnforcePinning(true);	
	obj->setGlobalOrientationQuat(newQori);
	
	// bound the pinned item to the dimensions of the wall	
	Box objBox = obj->getBox();
	if (adjustBoxToInsideWalls(objBox))
		pos = objBox.center;
	obj->setGlobalPosition(pos);

	obj->onPin();

	// XXX: Hack for sharing prototype
	if (isPile && reorientedToWall) 
		((Pile *)obj)->grid(pos);

	obj->setEnforcePinning(prevEnforcePinning);
}

void *ReshapeTextAfterAnim(AnimationEntry *animEntry)
{
	if (!isSlideshowModeActive())
	{
		textManager->forceUpdate();
		rndrManager->invalidateRenderer();
	}
	return NULL;
}

void *PostIntroLoad(AnimationEntry *animEntry)
{
	// Start loading textures
	texMgr->setSuspendLoading(false);
	
	// Get all the post it notes call
	// syncStickyNoteWithFileContents on each post it note
	vector<FileSystemActor *> stickyNotes = scnManager->getFileSystemActors(StickyNote);
	for (int i = 0; i < stickyNotes.size(); i++)
	{
		StickyNoteActor * actor = (StickyNoteActor *) stickyNotes[i];
		actor->syncStickyNoteWithFileContents();
	}

	// Get all the photo frame actors
	// Call initSource for each photo frame actor
	vector<FileSystemActor *> photoFrameActors = scnManager->getFileSystemActors(PhotoFrame, false);
	for (int i = 0; i < photoFrameActors.size(); i++)
	{
		PhotoFrameActor * pfActor = (PhotoFrameActor *) photoFrameActors[i];
		pfActor->initSource();	
	}
	
	// Run Automated JSON Tests
	if (scnManager->runAutomatedJSONTestsOnStartup)
	{
		// Wait a few seconds for textures to load, etc.
		Sleep(5000);
		StartAutomatedJSONTesting();
	}

	// Run Automated demo
	if (scnManager->runBumpTopTestsOnStartup)
	{
		Sleep(5000);
		startAutomatedTests();
	}

	// Mark the scene as loaded
	GLOBAL(isSceneLoaded) = true;
	return NULL;
}

void *DeletePileAfterAnim(AnimationEntry *animEntry)
{
	Pile *fsPile = (Pile *) animEntry->getObject();

	DeletePile(fsPile, true, false, false);

	return NULL;
}

void *UpdatePileDimsAfterAnim(AnimationEntry *animEntry)
{
	Pile * pile = dynamic_cast<Pile *>(animEntry->getObject());
	pile->updatePhantomActorDims();
	return NULL;
}

void *UpdatePileStateAfterLeafing(AnimationEntry *animEntry)
{
	Pile * pile = dynamic_cast<Pile *>(animEntry->getObject());
	assert(pile->getLastItem());
	if (pile->getLastItem())
	{
		Vec3 lastItemPos = pile->getLastItem()->getGlobalPosition();
		Vec3 pos = pile->getGlobalPosition();
		Vec3 stackPos(lastItemPos.x, pos.y, lastItemPos.z);
		pile->stack(stackPos, false);
		animManager->finishAnimation(pile);
	}
	return NULL;
}

void EnableRotation(NxActorWrapper* a, bool justY)
{
	if(justY)
	{
		a->clearBodyFlag(NX_BF_FROZEN_ROT_X);
		a->clearBodyFlag(NX_BF_FROZEN_ROT_Z);
	}
	else 
		a->clearBodyFlag(NX_BF_FROZEN_ROT);
}

void DisableRotation(vector<NxActorWrapper*> v)
{
	for(int i=0; i<v.size(); i++)
		v[i]->raiseBodyFlag(NX_BF_FROZEN_ROT);
}

void ExitBumptop()
{
	LOG(QString_NT("Exiting BumpTop"));

	if (Finder->isActive()) Finder->shutdown();
	
	// uninitialize all widgets (since they may still be referencing existing objects)
	widgetManager->uninitializeActiveWidgets();

	// If BumpTop is already marked for exit return, because we don't want to redo the exit code.  It interrupts the first batch and causes bad stuff.  
	if(GLOBAL(exitBumpTopFlag))
		return;

	// NOTE: reset actors being watched by the camera
	cam->restorePreviousVisibleNameables(true);
	cam->loadCameraFromPreset(GLOBAL(settings.cameraPreset));

	// save the scene before we set the exti flag
	SaveSceneToFile();

	GLOBAL(exitBumpTopFlag) = true;	
	sel->clear();

	// hide all items
	const vector<OverlayComponent *>& items = scnManager->nameables()->items();
	for (int i = 0; i < items.size(); ++i)
	{
		items[i]->setAlphaAnim(items[i]->getStyle().getAlpha(), 0.0f, 5);
	}

	// Folderize any items that are currently pileized
	while (GLOBAL(getPiles()).size())
	{
		Pile *t = GLOBAL(getPiles())[0];

		// Folderize the pile that is pileized
		if (t->getPileType() == HardPile)
		{
			FileSystemPile *fsPile = (FileSystemPile *) t;
			fsPile->folderize(false);
		}

		DeletePile(t, true, false, true);

		GLOBAL(gPause)=false;
		sel->setPickedActor(NULL);

	}

	// Loop through all items and move them to the center "flush down the toilet" animation
	vector<NxActorWrapper *> desktopActors = GetDesktopItems();
	for (uint i = 0; i < desktopActors.size(); i++)
	{
		Actor* data=(Actor*)desktopActors[i];

		if (data)
		{
			if (GetBumpActor(desktopActors[i]))
			{
				deque<Mat34> animPath;

				//Quadratic jump down toilet animation
				deque<Vec3> curve=lerpQuardaticCurve(desktopActors[i]->getGlobalPosition(), desktopActors[i]->getGlobalPosition()*0.15f + Vec3(0.0f,15.0f * float(i),0),Vec3(0,float(getDimensions(desktopActors[i]).y)*-1.5f,0), int(float(i) * 2.0f+1.0f));
				for(int j=0; j<curve.size(); j++)
				{
					animPath.push_back(Mat34(desktopActors[i]->getGlobalOrientation(), curve[j]));
				}

				data->setPoseAnim(animPath);
				data->fadeOut();

				animManager->addAnimation(AnimationEntry(GetBumpObject(desktopActors[i]), (FinishedCallBack) DeleteActorAfterAnim));
			}
		}
	}

	//Camera should spin+zoom out
	//Vec3 newUp(cam->getUp());
	//Quat(200, cam->getDir()).rotate(newUp);
	//cam->animateTo(Vec3(0,cam->getEye().y*1.5, 0), Vec3(0,-1,0), newUp, 50);

	// hide the text
	GLOBAL(settings).RenderText = false;

	//	
	evtManager->forceExitAfterTimeout(1500);
}

// Calculate the camera eye for the given bounds
Vec3 calculateEyeForBounds(const Bounds& newBounds)
{
	Vec3 camView;

	// cap a max box bounds for this, if the buffer percentage is used
	Bounds bounds = newBounds;
	if (scnManager->isInInfiniteDesktopMode)
	{
		// center the desktop box on the bounds specified
		// take the minimum of the desktop bounds
		/*
		Box desktopBox = GetDesktopBox();
		desktopBox.center = center;

		// get the union bounds
		extents.max(desktopBox.extents);
		*/

		Vec3 center, extents;
		bounds.getCenter(center);
		bounds.getExtents(extents);
		extents *= GLOBAL(settings).topDownViewBufferSize;
		bounds.setCenterExtents(center, extents);
	}

	// Calculate camera position into camView
	camView.x = bounds.getMin().x + ((bounds.getMax().x - bounds.getMin().x) / 2);
	camView.z = bounds.getMin().z + ((bounds.getMax().z - bounds.getMin().z) / 2);

	float aspect = float(winOS->GetWindowWidth()) / float(winOS->GetWindowHeight());
	float hFoV = bounds.getMax().z - bounds.getMin().z;
	float wFoV = bounds.getMax().x - bounds.getMin().x;
	float FoV = 60.0 / 2; // 60 / 2 Deg on Y-axis
	float tanFoV = tan(FoV * (PI / 180));
	float multiplier;

	// Fix the multiplier based on the aspect ratio, if needed (aspect ratio only if width is larger)
	if (wFoV / hFoV > aspect)
	{
		// Width is longer then the screen, use it
		multiplier = wFoV / aspect;
	}else{
		// height is longer then the screen
		multiplier = hFoV;
	}

	// distance form the group
	camView.y = bounds.getMax().y + ((multiplier / 2) / tanFoV);

	return camView;
}
//Zooms camera right above given bounds.  Using a Top-down view
Vec3 zoomToBounds(const Bounds& newBounds, bool animate, int timeStep)
{
	Vec3 camView = calculateEyeForBounds(newBounds);

	// Slerp the camera into the direction of this object
	if (animate)
	{
		cam->animateTo(camView, Vec3(0,-1,0), Vec3(0,0,1), timeStep);
	}else{
		cam->setOrientation(camView, Vec3(0,0,1), Vec3(0,-1,0));
	}

	return camView;
}

//Zooms camera right above given bounds.  Using a Top-down view
Vec3 zoomToAngledBounds(const Bounds& bounds, bool animate)
{
	Vec3 up;
	//up.cross(Vec3(0,-0.9f,0.4f), Vec3(0, 1, 0));
	//up.cross(up, Vec3(0,-0.9f,0.4f));

	// Move the camera into position so it encapsulates the entire work area
	Vec3 oldEye = cam->getEye(), oldUp = cam->getUp(), oldDir = cam->getDir();
	Vec3 newEye, newDir, extents;
	bounds.getExtents(extents);
	float distZ = extents.z * 0.70f;
	float distX = extents.x * 0.70f;

	// Move the camera to look at that location
	newEye = zoomToBounds(bounds, false);

	// Restore old location
	cam->setOrientation(oldEye, oldUp, oldDir);
	//up.cross(-newEye, Vec3(1, 0, 0));
	//up.normalize();

	// Animate to new location from old location
	Vec3 center;
	bounds.getCenter(center);
	newEye += Vec3(0, 10, -distZ);
	//newEye += Vec3(distX, 10, 0); // iphone spin
	Vec3 direction = center - newEye;

	//Vec3 up;
	up.cross(direction, Vec3(0, 1, 0));
	up.cross(up, direction);

	if (animate)
	{
		if (scnManager->zoomToAngleBoundsTempOverride > 0)
			cam->animateTo(newEye, direction, up, scnManager->zoomToAngleBoundsTempOverride);
		else
			cam->animateTo(newEye, direction, up);
	}
	else
	{
		cam->setOrientation(newEye, up, direction);
	}

	// dismiss the free-form camera message
	dismiss("Camera::setIsCameraFreeForm");

	return newEye;
}

deque<Vec3> lerpQuardaticCurve(Vec3 startPoint, Vec3 topPoint, Vec3 endPoint, int numSteps, EaseFunctionType easeType)
{
	deque<Vec3> curve;
	deque<Vec3> lineOne;
	deque<Vec3> lineTwo;
	deque<float> range;

	// Get the path for each of the lines from the start, to the apex, and down to the end
	lineOne = lerpRange(startPoint, topPoint, numSteps, NoEase);
	lineTwo = lerpRange(topPoint, endPoint, numSteps, NoEase);
	range = lerpRange(0.0f, 1.0f, numSteps, easeType);

	// Loop from 1 to numSteps -1 because we already have the start/end points
	for (int i = 0; i < numSteps; i++)
	{
		Vec3 origin = (lineOne[i] - startPoint);
		Vec3 end = (lineTwo[i] - startPoint) - origin;

		// This does the lerping using easing (if specified)
		curve.push_back(origin + (end * range[i]) + startPoint);
	}

	return curve;
}

deque<Vec3> lerpQuardaticFittingCurve(Vec3 startPoint, Vec3 topPoint, Vec3 endPoint, int numSteps, EaseFunctionType easeType)
{
	deque<Vec3> curve;
	deque<float> range = lerpRange(0.0f, 1.0f, numSteps, easeType);

	float t = 0.4f;
	Vec3 p0 = startPoint;
	Vec3 p1 = topPoint;
	Vec3 p2 = endPoint;

	Vec3 actualP1 = (p1 - (p0 * pow(1.0f-t, 2.0f)) - (pow(t, 2.0f) * p2)) / (2 * t * (1-t));
	for (int i = 1; i < numSteps; ++i)
	{
		t = range[i];
		Vec3 p = (pow(1.0f-t, 2.0f) * p0) + (2 * t * (1-t) * actualP1) + (pow(t, 2.0f) * p2);
		curve.push_back(p);
	}
	curve.push_back(endPoint);

	return curve;
}

void setDimsFromFileSize(NxActorWrapper *actor)
{
	
	FileSystemActor *data = GetFileSystemActor(actor);
	uint fileSize = 0;
	float scale;
	Vec3 iSize;

	// Disable it for now
	return;

	if (data && 
		!(fsManager->getFileAttributes(data->getFullPath()) & Directory) &&
		fsManager->getFileAttributes(data->getFullPath()) != 0
		)
	{
		fileSize = fsManager->getFileSize(data->getFullPath());
		iSize = data->getDims();

		if (fileSize > GLOBAL(settings).maxFileSizeForScale)
			scale = GLOBAL(settings).maxFileSizeThickness;
		else if (fileSize < GLOBAL(settings).minFileSizeForScale)
			scale = GLOBAL(settings).minFileSizeThickness;
		else
			scale = ((fileSize / GLOBAL(settings).maxFileSizeForScale) * (GLOBAL(settings).maxFileSizeThickness - GLOBAL(settings).minFileSizeThickness)) + GLOBAL(settings).minFileSizeThickness;

		data->setDims(Vec3(iSize.x, iSize.y, scale));

		if (data->isParentType(BumpPile))
		{
			Pile *pile = (Pile *) data->getParent();
			pile->updatePhantomActorDims();
		}

		return;
	}

	// No Actor Data!
	return;
}

#ifdef DXRENDER
void world2window(const Vec3& point, int & x, int & y)
{
	dxr->world2window(point, x, y);
}
#else
void world2window(const Vec3& point, int & x, int & y)
{
	GLdouble wx, wy, wz;  //  returned window x, y, z coords
	GLint * viewport = cam->glViewport();
	GLdouble * projmatrix = cam->glProjectionMatrix();
	GLdouble * mvmatrix = cam->glModelViewMatrix();
	// note viewport[3] is height of window in pixels
	 //printf ("Coordinates at cursor are (%4d, %4d)\n", x, realy);
	gluProject(point.x, point.y, point.z, mvmatrix, projmatrix, viewport, &wx, &wy, &wz);


	x=int(wx);
	y=int(wy);
}
#endif

Vec3 world2window(const Vec3& point)
{
	int x = 0, y = 0;
	world2window(point, x, y);
	return Vec3(x, y, 0);
}



bool isMatEqual(Mat34 a, Mat34 b)
{
	NxF32 d[16], e[16];

	a.getColumnMajor44(d);
	b.getColumnMajor44(e);

	for (int i = 0; i < 16; i++)
	{
		if (abs(d[i] - e[i]) > 0.01)
		{
			return false;
		}
	}

	return true;
}
// Writes a chunk of data to a stream.
// stream and buffer are used interchangeably here
//
// bufPtr = the stream you are writing to
// dataSz = how long the data chunk is
// streamSize = the remaining bytes in the stream
// dataPtr = the place where the data chunk starts
//
bool SERIALIZE_WRITE(unsigned char **bufPtr, uint dataSz, uint &streamSize, void *dataPtr)
{ 
	uint sz = dataSz; 
	
	// Byte Layout:
	// [ SIZE OF VALUE IN BYTES ] [ VALUE BYTES ]
	// ie. [ sizeof(Vec3) ] [ &someVectorVariable ]

	if (streamSize >= (dataSz + sizeof(int)))
	{
		memcpy((void *) *bufPtr, (void *) &sz, sizeof(uint)); 
		*bufPtr += sizeof(uint); 
		streamSize -= sizeof(int);

		if (dataSz > 0)
		{
			memcpy((void *) *bufPtr, (void *) dataPtr, sz); 
			*bufPtr += sz; 
			streamSize -= sz;
		}

		return true;
	}

	return false;
}

bool SERIALIZE_WRITE_QSTRING(unsigned char **bufPtr, uint &streamSize, QString str)
{
	QByteArray tmp = str.toUtf8();
	return SERIALIZE_WRITE(bufPtr, tmp.size(), streamSize, (void *) tmp.constData());
}

bool SERIALIZE_WRITE_VEC3(unsigned char **bufPtr, uint &streamSize, Vec3 vec)
{
	return SERIALIZE_WRITE(bufPtr, sizeof(NxReal), streamSize, &vec.x) &&
		   SERIALIZE_WRITE(bufPtr, sizeof(NxReal), streamSize, &vec.y) &&
		   SERIALIZE_WRITE(bufPtr, sizeof(NxReal), streamSize, &vec.z);
}

bool SERIALIZE_READ(unsigned char **bufPtr, void *dataPtr, uint varSize, uint &bufSz)
{ 
	uint sz; 

	// Incoming data format:	
	// [ sizeof(someData) ] [ someData ]

	if (bufSz >= sizeof(uint))
	{
		// Read out the size of the entry
		memcpy((void *) &sz, (void *) *bufPtr, sizeof(uint)); 

		if (bufSz >= sz && sz <= varSize)
		{
			*bufPtr += sizeof(uint);
			bufSz -= sizeof(uint);

			if (sz > 0)
			{
				memset(dataPtr, NULL, sz); 
				memcpy((void *) dataPtr, (void *) *bufPtr, sz); 
				*bufPtr += sz; 
				bufSz -= sz;
			}

			return true;
		}
	}

	return false;
}

bool SERIALIZE_READ_STRING(unsigned char **bufPtr, QString &str, uint &bufSz)
{ 
	unsigned char dat[1024] = {0}; 
	str.clear();
	
	if (SERIALIZE_READ(bufPtr, dat, 1024, bufSz))
	{
		// we are reading a previously written ascii string
		if (dat[0] != '0')
			str = QString((char *) dat); 
		return true;
	}

	return false;
}

bool SERIALIZE_READ_QSTRING(unsigned char **bufPtr, QString &str, uint &bufSz)
{ 
	unsigned char dat[1024] = {0}; 
	str.clear();
	
	if (SERIALIZE_READ(bufPtr, dat, 1024, bufSz))
	{
		// we are reading a utf8 string
		if (dat[0] != '0')
			str = QString::fromUtf8((char *) dat); 
		return true;
	}

	return false;
}

bool SERIALIZE_READ_VEC3(unsigned char **bufPtr, Vec3& vec, uint &bufSz)
{
	NxReal x, y, z;
	if (SERIALIZE_READ(bufPtr, &x, sizeof(NxReal), bufSz) && SERIALIZE_READ(bufPtr, &y, sizeof(NxReal), bufSz) && SERIALIZE_READ(bufPtr, &z, sizeof(NxReal), bufSz))
	{
		vec = Vec3(x, y, z);
		return true;
	}
	return false;
}

void SaveSceneToFile()
{
	QMutexLocker m(&GLOBAL(saveSceneMutex));
	if (GLOBAL(skipSavingSceneFile))
		return; 

	QFileInfo filePath = scnManager->getScenePbBumpFile();
	QFileInfo backupFilePath = scnManager->getBackupScenePbBumpFile();
	QString filePathStr = native(filePath);
	QString backupFilePathStr = native(backupFilePath);
	bool isLocalSceneFile = filePathStr.startsWith(native(scnManager->getWorkingDirectory()), Qt::CaseInsensitive);

	// unhide the scene files so we can modify it
	if ((fsManager->getFileAttributes(filePathStr) & Hidden) > 0)
		fsManager->setFileAttributes(filePathStr, Normal);
	if ((fsManager->getFileAttributes(backupFilePathStr) & Hidden) > 0)
		fsManager->setFileAttributes(backupFilePathStr, Normal);

	// don't save the backup when in shell extension mode
	if (!scnManager->isShellExtension)
	{
		// replace the previous backup
		if (exists(backupFilePath))
			QFile::remove(backupFilePathStr);

		// make the current scene file the backup
		if (exists(filePath))
			QFile::rename(filePathStr, backupFilePathStr);
	}
	
	// ensure that the scene file creation date time matches the modified time
	if (scnManager->getSceneBumpFile().created() < QDateTime::currentDateTime())
	{
		HANDLE hFile = CreateFile((LPCTSTR) filePathStr.utf16(), 
			GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile)
		{
			FILETIME now;
			GetSystemTimeAsFileTime(&now);
			SetFileTime(hFile, &now, NULL, NULL);
			CloseHandle(hFile);
		}
	}

	// save the scene 
	PbPersistenceManager::getInstance()->saveScene(filePathStr);
	
	// mark files hidden again
	// Note: only local scene files (scene files not in the user data folder) will
	//		 be hidden	
	if (isLocalSceneFile)
	{
		if (exists(filePathStr))
			fsManager->setFileAttributes(filePathStr, Hidden);
		if (!scnManager->isShellExtension && exists(backupFilePath))
			fsManager->setFileAttributes(backupFilePathStr, Hidden);
	}
	m.unlock();
}

bool LoadSceneFromFile()
{	
	// try and load the old scene files
	QFileInfo file = scnManager->getSceneBumpBackupFile();
	QFileInfo backup = scnManager->getSceneBumpFile();

	bool oldSceneFileLoaded = 
		LegacyPersistenceManager::getInstance()->loadScene(native(file)) ||
		LegacyPersistenceManager::getInstance()->loadScene(native(backup));

	// if an old scene file was loaded, then we need to save the new format, 
	// then delete the old files
	if (oldSceneFileLoaded)
	{
		// save the current scene file
		SaveSceneToFile();

		// rename the old scene files
		if (exists(native(file)))
		{
			QFileInfo legacyFilePath = make_file(parent(file), "scene.legacy.bump");
			QFile::rename(native(file), native(legacyFilePath));
		}
		if (exists(native(backup))) 
		{
			QFileInfo legacyBackupPath = make_file(parent(backup), "scene.legacy.bump.bak");
			QFile::rename(native(backup), native(legacyBackupPath));
		}

		return true;
	}
	else
	{
		// otherwise, just load the new scene file
		QFileInfo pbFile = scnManager->getScenePbBumpFile();
		QFileInfo pbBackup = scnManager->getBackupScenePbBumpFile();
		return PbPersistenceManager::getInstance()->loadScene(native(pbFile)) ||
			PbPersistenceManager::getInstance()->loadScene(native(pbBackup));
	}
}

void PruneJoints(NxActorWrapper *actor)
{
	
	//Delete any pin joints pickedActor has
	if (GetBumpActor(actor) && GetBumpActor(actor)->isPinned())
	{
		BumpObject * object = (BumpObject *) actor;
		NxJoint * joint = object->getPinJoint();
		if (joint)
		{
			GLOBAL(gScene)->releaseJoint(*joint);
			object->setPinJoint(NULL);
		}
	}

}

void doNovodexTick(float timeElapsed)
{
	__try
	{
		scnManager->gScene->startRun(timeElapsed * 2.0f);	//Note: a real application would compute and pass the elapsed time here.
		scnManager->gScene->flushStream();
		scnManager->gScene->finishRun();
	} __except(EXCEPTION_EXECUTE_HANDLER)
	{
		// consoleWrite(QT_TR_NOOP("Novodex Crash! Attempting to Re-initialize.\n"));
		handleNovodexCrash();
	}
}

void handleNovodexCrash()
{
	#ifdef BTDEBUG
		assert(false);
	#endif
	
	static int novodexCrashCount = 0;
	novodexCrashCount++;
	LOG(QString_NT("NovodexCrash %1").arg(novodexCrashCount));
	if (novodexCrashCount > 10)
	{
		// show the internal error dialog
		dlgManager->clearState();
		dlgManager->setPrompt(BtUtilStr->getString("InternalError"));
		dlgManager->promptDialog(DialogCaptionOnly);
	}

	// default vars
	Vec3 defaultDims(GLOBAL(settings).xDist, GLOBAL(settings).zDist, GLOBAL(settings).yDist);
	Vec3 defaultPosition(0.0f);
	Mat33 defaultOri(NX_IDENTITY_MATRIX);

	// force-validate every actor
	vector<BumpObject *> bumpObjects = scnManager->getBumpObjects();
	for (int i = 0; i < bumpObjects.size(); ++i)
	{
		BumpObject * actor = bumpObjects[i];

		// zero all other attributes
		actor->setAngularMomentum(Vec3(0.0f));
		actor->setAngularVelocity(Vec3(0.0f));
		actor->setForce(Vec3(0.0f));
		actor->setLinearMomentum(Vec3(0.0f));
		actor->setLinearVelocity(Vec3(0.0f));

		// check the dimensions
		unsigned int numShapes = actor->getNbShapes();
		NxShape ** shapes = actor->getShapes();
		if (numShapes > 0)
		{
			NxBoxShape * box = shapes[0]->isBox();
			if (box && !box->getDimensions().isFinite())
				box->setDimensions(defaultDims);
		}
		else
			consoleWrite(QString("Invalid shape for: %1\n").arg(actor->getFullText()));

		// check the actor pose
		const Mat34& poseRef = actor->getGlobalPoseReference();
		if (!poseRef.M.isFinite())
			actor->setGlobalOrientation(defaultOri);
		if (!poseRef.t.isFinite())
			actor->setGlobalPosition(defaultPosition);
		else
		{
			Vec3 newPos = poseRef.t;
			Box actorBox = actor->getBox();
			adjustBoxToInsideWalls(actorBox);
			actorBox.center.y = NxMath::max(actorBox.center.y, actorBox.extents.y); // Position above floor
			actor->setGlobalPosition(actorBox.center);
		}
		
	}
}

void InitNovodex()
{	
	NovodexOutputStream * nxStream = NULL;
#if BTDEBUG
	nxStream = new NovodexOutputStream();
#endif

	GLOBAL(gPhysicsSDK) = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, 0, nxStream);
	GLOBAL(gPhysicsSDK)->setParameter(NX_MIN_SEPARATION_FOR_PENALTY, -0.5f);

	// steel on steel
 	NxMaterial defaultMaterial;
	defaultMaterial.staticFriction = 0.55f;
	defaultMaterial.dynamicFriction = 0.675f;
	GLOBAL(gPhysicsSDK)->setMaterialAtIndex(0, &defaultMaterial);

	//Initialize Shape Visualizer
#ifndef DXRENDER
	ShapeVis::init();
#endif

	// Create a scene
	float gravityScale = 10.0f;
	NxSceneDesc sceneDesc;
	sceneDesc.gravity = Vec3(0, gravityScale * -9.81f, 0);
	GLOBAL(gScene) = GLOBAL(gPhysicsSDK)->createScene(sceneDesc);

	// icons no longer collide with each other!
	GLOBAL(gPhysicsSDK)->setGroupCollisionFlag(DYNAMIC_GROUP_NUMBER, DYNAMIC_GROUP_NUMBER, false);
	GLOBAL(gPhysicsSDK)->setGroupCollisionFlag(DYNAMIC_GROUP_NUMBER, PILE_SPACE_FLOOR_NUMBER, false);
	GLOBAL(gPhysicsSDK)->setGroupCollisionFlag(DYNAMIC_GROUP_NUMBER, PILE_SPACE_GROUP_NUMBER, false);
	GLOBAL(gPhysicsSDK)->setGroupCollisionFlag(DYNAMIC_GROUP_NUMBER, PILE_SPACE_OTHER_NUMBER, false);
	GLOBAL(gPhysicsSDK)->setGroupCollisionFlag(PILE_SPACE_GROUP_NUMBER, PILE_SPACE_OTHER_NUMBER, false);

	// Create ground plane
	NxPlaneShapeDesc PlaneDesc;
	NxActorDesc ActorDesc;
	ActorDesc.shapes.pushBack(&PlaneDesc);
	NxActor *temp = GLOBAL(gScene)->createActor(ActorDesc);
	temp->getShapes()[0]->setGroup(FLOOR_GROUP_NUMBER);

	// create the dynamic floor
	repoManager->createDynamicFloor();	
}

void CreateWalls()
{
	if (!themeManager->getValueAsBool("textures.wall.allowStretch",false))
		GLOBAL(WallHeight) = 55.0f;

	const float wallHeight = GLOBAL(WallHeight);
	const float wallPos = wallHeight - kWallPositionBuffer;

	GLOBAL(Walls).push_back( CreateCuboid( Vec3( 0, wallPos, 120 ), 150, wallHeight, 10, true ) ); //0 top wall
	GLOBAL(Walls).push_back( CreateCuboid( Vec3( 0, wallPos, -120 ), 150, wallHeight, 10, true ) ); //1 bottom wall
	GLOBAL(Walls).push_back( CreateCuboid( Vec3( -150, wallPos, 0 ), 10, wallHeight, 10, true ) ); //2 right wall
	GLOBAL(Walls).push_back( CreateCuboid( Vec3( 150, wallPos, 0 ), 10, wallHeight, 10, true ) ); //3 left wall

	for (int i = 0; i < 4; i++)
	{
		GLOBAL(Walls)[i]->getShapes()[0]->setGroup(WALL_GROUP_NUMBER);
		GLOBAL(WallsPos).push_back(GLOBAL(Walls)[i]->getGlobalPosition());

		// Create walls that are attached to the camera
		if (GLOBAL(settings).camWallsEnabled)
		{			
			GLOBAL(CamWalls).push_back(CreateCuboid(Vec3(0.0f), 1, 1, 1, true));
			GLOBAL(CamWalls)[i]->getShapes()[0]->setGroup(WALL_GROUP_NUMBER);
		}
	}
}

Vec3 getDimensions(NxActorWrapper *a)
{
	// REFACTOR: this is for backwards compatibility with the old code.
	NxShape** shapes;
	NxBoxShape* box;

	if (a->getNbShapes() > 0)
	{
		shapes = a->getShapes();
		box = shapes[0]->isBox();

		if (box)
		{
			// Return proper dimensions
			return box->getDimensions();
		}
		else 
		{
			// Actor does not have a Bounding Box?
			return Vec3(0.0f);
		}
	}

	return Vec3(0.0f);
}

void ResizeWallsToWorkArea(int maxX, int maxY)
{
	vector<NxActorWrapper *>& walls = GLOBAL(Walls);
	vector<Vec3>& wallsPos = GLOBAL(WallsPos);

	Vec3 dims, pos;
	const float wallHeight = GLOBAL(WallHeight);
	const float wallPos = wallHeight - kWallPositionBuffer;
	Quat qRight(-90, Vec3(0,1,0));
	Quat qLeft(90, Vec3(0,1,0));
	Quat qBottom(180, Vec3(0,1,0));

	// Top Wall
	dims = walls[0]->getDims();
	dims.x = NxMath::abs((maxX / 2) * GLOBAL(factor));
	walls[0]->setDims(Vec3(dims.x, wallHeight, dims.z));
	pos = walls[0]->getGlobalPosition();
	pos.z = -(maxY / 2 * GLOBAL(factor)) + dims.z;
	walls[0]->setGlobalPosition(pos);

	// Bottom Wall
	walls[1]->setGlobalOrientationQuat(qBottom);
	dims = walls[1]->getDims();
	dims.x = NxMath::abs((maxX / 2) * GLOBAL(factor));
	walls[1]->setDims(Vec3(dims.x, wallHeight, dims.z));
	pos = walls[1]->getGlobalPosition();
	pos.z = (maxY / 2 * GLOBAL(factor)) - dims.z;
	walls[1]->setGlobalPosition(pos);

	// Right Wall
	walls[2]->setGlobalOrientationQuat(qRight);
	dims = walls[2]->getDims();
	dims.x = NxMath::abs((maxY / 2) * GLOBAL(factor));
	walls[2]->setDims(Vec3(dims.x, wallHeight, dims.z));
	pos = walls[2]->getGlobalPosition();
	pos.x = (maxX / 2 * GLOBAL(factor)) - dims.z;
	walls[2]->setGlobalPosition(pos);

	// Left Wall
	walls[3]->setGlobalOrientationQuat(qLeft);
	dims = walls[3]->getDims();
	dims.x = NxMath::abs((maxY / 2) * GLOBAL(factor));
	walls[3]->setDims(Vec3(dims.x, wallHeight, dims.z));
	pos = walls[3]->getGlobalPosition();
	pos.x = -(maxX / 2 * GLOBAL(factor)) + dims.z;
	walls[3]->setGlobalPosition(pos);

	// Save the position of the walls when they are on the floor
	wallsPos.clear();
	for (int i = 0; i < walls.size(); i++)
		wallsPos.push_back(walls[i]->getGlobalPosition());

#ifdef DXRENDER
	Vec3 mainWorkspaceDims(abs(GLOBAL(WallsPos)[3].x - GLOBAL(WallsPos)[2].x) - GLOBAL(Walls)[2]->getDims().z,
		wallHeight * 2,
		abs(GLOBAL(WallsPos)[0].z - GLOBAL(WallsPos)[1].z) - GLOBAL(Walls)[1]->getDims().z);

	dxr->initializeDesktop(mainWorkspaceDims);
#endif

	vector<BumpObject *> objs = scnManager->getBumpObjects();
	for (int i = 0; i < objs.size(); ++i)
	{
		if (objs[i]->isPinned())
		{
			// bound this new point by the wall dims
			Vec3 newPt = adjustPtOnWall(objs[i], objs[i]->getPinWall());
			objs[i]->breakPin();
			objs[i]->setGlobalPosition(newPt);
			objs[i]->onPin();
		}
		else
		{
			// just adjust it to be in the bounds of the desktop box
			Box objBox = objs[i]->getBox();
			if (adjustBoxToInsideWalls(objBox))
				objs[i]->setGlobalPosition(objBox.center);
		}
	}	
}

void ClearBumpTop()
{
	// remove all pending transfers from the file transfer manager
	ftManager->removeAllTransfers();
	// remove all the actors and piles from the scene
	scnManager->clearBumpTop();
	texMgr->deleteNonPersistentTextures();
	
}

void PushBelowGround(NxActorWrapper *actor)
{
	if (actor)
	{
		actor->setGlobalPosition(actor->getGlobalPosition() + Vec3(0, -100, 0));

		if (((BumpObject *) actor)->isBumpObjectType(BumpActor))
		{
			GetBumpActor(actor)->pushActorType(Invisible);
		}
	}
}

void Key_ToggleInvisibleActors()
{	
	vector<NxActorWrapper *> actors = GetDesktopItems();
	Actor *actor;

	for (int i = 0; i < actors.size(); i++)
	{
		actor = GetBumpActor(actors[i]);

		if (actor->isActorType(Invisible))
		{
			actor->popActorType(Invisible);
		}
	}

	printStr(BtUtilStr->getString("ToggleInvisibleActors"));
}

void Key_TriggerUndo()
{
	/*
	if (!undoManager->undo())
	{
		MessageClearPolicy clearPolicy;
			clearPolicy.setTimeout(2);
		scnManager->messages()->addMessage(new Message("Key_TriggerUndoRedo", QT_TR_NOOP("Nothing more to Undo."), Message::Ok, clearPolicy));
	}
	*/
	GLOBAL(mouseUpTriggered) = false;
}

void Key_TriggerRedo()
{
	/*
	if (!undoManager->redo())
	{
		MessageClearPolicy clearPolicy;
			clearPolicy.setTimeout(2);
		scnManager->messages()->addMessage(new Message("Key_TriggerUndoRedo", QT_TR_NOOP("Nothing more to Redo."), Message::Ok, clearPolicy));
	}
	*/

	GLOBAL(mouseUpTriggered) = false;
}

void Key_TogglePrintMode()
{
	
	GLOBAL(settings).PrintMode=!GLOBAL(settings).PrintMode;
#ifdef DXRENDER
	// TODO DXR
#else
	if(GLOBAL(settings).PrintMode)
		glClearColor(white3, 1.0);
	else
		glClearColor(black3, 1.0); 
#endif

}

//Show the console window if its hidden
void Key_ShowConsole()
{
	if(!GLOBAL(settings).showConsoleWindow)
	{
		GLOBAL(settings).showConsoleWindow = true;
		startConsoleWin();	
		printStr(QT_TRANSLATE_NOOP("BtUtilStrings", "Debug Console Revealed"));
	}
}

void Key_ShowSettingsDialog() {
	QString paramsStr;
	paramsStr.append(QString(" -bumptopHWND %1").arg((unsigned int) winOS->GetWindowsHandle()));
	if (!GLOBAL(settings).inviteCode.isEmpty())
		paramsStr.append(QString(" -inviteCode %1").arg(GLOBAL(settings).inviteCode));
	if (winOS->IsKeyDown(KeyShift))
		paramsStr.append(" -admin");
	paramsStr.append(QString(" -language %1").arg(winOS->GetLocaleLanguage()));

	Key_ShowSettingsDialogWindow(paramsStr);
}

void Key_ShowSettingsDialogWindow(QString paramsStr)
{
	HWND settingsWindow = FindWindowEx(NULL, NULL, NULL, L"BumpTop Settings");
	if (settingsWindow)
	{
		// bring the existing settings window to the top if there is one
		BringWindowToTop(settingsWindow);
	}
	else
	{
		// launch the new settings window
		QDir settingsPath = winOS->GetExecutableDirectory();
		QFileInfo settingsApp = make_file(settingsPath, "BumpTop Settings.exe");

		LOG("Key_ShowSettingsDialog::settingsPath");
		LOG(native(settingsPath));
		LOG("Key_ShowSettingsDialog::settingsApp");
		LOG(native(settingsApp));

		QString execDirStr = native(winOS->GetExecutableDirectory());

		LOG("Key_ShowSettingsDialog::params");
		LOG(paramsStr);

		QString settingsAppStr = native(settingsApp);
		printUnique("Key_ShowSettingsDialog", BtUtilStr->getString("LaunchSettings"));

		SHELLEXECUTEINFO sei = {0};
			sei.cbSize = sizeof(SHELLEXECUTEINFO);
			sei.hwnd = winOS->GetWindowsHandle();
			sei.lpVerb = NULL; // giving a null value for the verb forces it to use the default verb
			sei.lpFile = (LPCTSTR) settingsAppStr.utf16();
			sei.lpDirectory = (LPCTSTR) execDirStr.utf16();
			sei.lpParameters = (LPCTSTR)paramsStr.utf16();
			sei.nShow = SW_SHOWNORMAL;
		ShellExecuteEx(&sei);

		statsManager->getStats().bt.settings.instantiations++;
	}
}

void Key_ToggleSharingMode()
{
	if (!GLOBAL(enableSharingMode))
		return;

	if (GLOBAL(isInSharingMode))
	{
#ifdef TABLE
		Key_SetCameraAsTopDown();
#else
		cam->zoomToAngledView();
#endif

		// hide all desktop labels
		for (int i = 0; i < GLOBAL(sharedDesktops).size(); ++i)
		{
			SharedDesktop * desktop = GLOBAL(sharedDesktops)[i];
			desktop->hideText();
		}

		for (int i = 0; i < GLOBAL(Walls).size(); ++i)
		{
			GLOBAL(Walls)[i]->setGlobalPosition(GLOBAL(Walls)[i]->getGlobalPosition() - Vec3(0, 1500, 0));
		}
	}
	else
	{
		Vec3 newEye = cam->getEye() + Vec3(0, 6.0f * GLOBAL(WallsPos)[1].y, GLOBAL(WallsPos)[1].z / 2.0f);
		Vec3 newDir = cam->getOrigin() + Vec3(0, 0, 150.0f) - newEye;
		cam->animateTo(newEye, newDir);

		// show all desktop labels
		for (int i = 0; i < GLOBAL(sharedDesktops).size(); ++i)
		{
			SharedDesktop * desktop = GLOBAL(sharedDesktops)[i];
			desktop->showText();
		}

		for (int i = 0; i < GLOBAL(Walls).size(); ++i)
		{
			GLOBAL(Walls)[i]->setGlobalPosition(GLOBAL(Walls)[i]->getGlobalPosition() + Vec3(0, 1500, 0));
		}
	}
	GLOBAL(isInSharingMode) = !GLOBAL(isInSharingMode);
}

void Key_UnsetSharingMode()
{
	if (GLOBAL(isInSharingMode))
		Key_ToggleSharingMode();
}

void Key_CreateWebActor()
{	
#ifdef ENABLE_WEBKIT
	LOG_LINE_REACHED();
	WebActor * actor = NULL;
	CustomizeWizard* addGadgetWizard = new CustomizeWizard(CustomizeWizard::GADGET);
	QString text = addGadgetWizard->exec();
	if (!text.isEmpty())
	{
		actor = new WebActor();
		QString data = text.trimmed();
		QString u = QUrl(data, QUrl::StrictMode).toString();
		//illegal characters in url, but appear in html; 
		//QUrl thinks Google Gadget code is URL because it contains an URL in <script> attribute
		if (data.contains('<') || data.contains('>')) 
			actor->loadHTML(data);
		else
		{
			QUrl url(data);
			if (!url.isValid())
				url.setUrl(QString("http://") + data);
			actor->load(url.toString());
		}
	}
	if(!WebActor::canMakeMore()) {
		printUnique("CreateWebActor", BtUtilStr->getString("WebWidgetLimit"));
		FadeAndDeleteActor(actor);
	}
#endif
}

void Key_ReloadWebActor()
{
	if (sel->getSize() == 1)
	{
		WebActor *actor = dynamic_cast<WebActor*>(sel->getBumpObjects()[0]);
		if (actor)
			actor->reload();
	}
}

void Key_ToggleUserAgent() 
{
	switch (GLOBAL(settings).userAgent)
	{
	case DefaultUserAgent:
		printUnique("ConsoleLoggingQWebPage::userAgentForUrl", "Default BumpTop User Agent");
		GLOBAL(settings).userAgent = iPhoneUserAgent;
		break;
	case iPhoneUserAgent:
		printUnique("ConsoleLoggingQWebPage::userAgentForUrl", "iPhone User Agent");
		GLOBAL(settings).userAgent = IE8UserAgent;
		break;
	case IE8UserAgent:
		printUnique("ConsoleLoggingQWebPage::userAgentForUrl", "IE8 User Agent");
		GLOBAL(settings).userAgent = DefaultUserAgent;
		break;
	default: break;
	}
}

//A function to test random functional by keystroke.  Delete the body and put whatever you want here. 
void Key_Test()
{
#ifdef DXRENDER
	if (sel->getSize() == 2)
	{
		
		WebActor * webActor = NULL;
		FileSystemActor * fileSystemActor = NULL;
		if (sel->getBumpObjects()[0]->isObjectType(ObjectType(BumpActor, Webpage)))
		{
			webActor = (WebActor *)sel->getBumpObjects()[0];
			if (sel->getBumpObjects()[1]->isObjectType(ObjectType(BumpActor, FileSystem)))
				fileSystemActor = (FileSystemActor *)sel->getBumpObjects()[1];
		}
		else if (sel->getBumpObjects()[1]->isObjectType(ObjectType(BumpActor, Webpage)))
		{
			webActor = (WebActor *)sel->getBumpObjects()[1];
			if (sel->getBumpObjects()[0]->isObjectType(ObjectType(BumpActor, FileSystem)))
				fileSystemActor = (FileSystemActor *)sel->getBumpObjects()[0];
		}
		if (webActor && fileSystemActor)
		{
			LPCWSTR file = (LPCWSTR)fileSystemActor->getTargetPath().utf16();
			if (QFileInfo(fileSystemActor->getTargetPath()).suffix() == QT_NT("x"))
			{
				SAFE_DELETE(webActor->_mesh);
				webActor->_mesh = new Mesh(file);
			}
			else
			{
				SAFE_DELETE(webActor->_videoRender);
				webActor->_videoRender = new VideoRender(file);
			}

			if (webActor->_videoRender && webActor->_mesh)
			{
				IDirect3DTexture9 * texture = NULL;
				webActor->_videoRender->GetTexture(&texture);
				for (unsigned int i = 0; i < webActor->_mesh->GetMeshMaterialCount(); i++)
					webActor->_mesh->SetTexture(i, texture);
			}
		}
	}
#endif
}

void Key_ToggleProfiling()
{
	if (!profiler->isProfiling())
	{
		profiler->start();
		printStr("Profiling started");
	} else {
		profiler->stop();
		printStr(QString("Profiling stopped, results written to %1").arg(native(winOS->GetDataDirectory())));
	}
}

void Key_ForceDump()
{
	int* a = NULL;
	a[0] = 0;
}

void Key_SetLaunchOverride()
{
	vector<BumpObject *> selection = sel->getBumpObjects();
	if (selection.size() == 1)
	{
		// prompt for the path to launch
		dlgManager->clearState();
		dlgManager->setPrompt(BtUtilStr->getString("LaunchTargetPrompt"));
		if (dlgManager->promptDialog(DialogInput))
		{
			BumpObject * obj = selection.front();
			obj->setLaunchOverride(dlgManager->getText());

			printUnique("Key_SetLaunchOverride", BtUtilStr->getString("LaunchTargetSet"));
		}
	}
	else
		printUnique("Key_SetLaunchOverride", BtUtilStr->getString("LaunchTargetObj"));
}

void Key_CreateCustomActor()
{
	// prompt for the name of the actor
	dlgManager->clearState();
	dlgManager->setPrompt(BtUtilStr->getString("CustomActorNamePrompt"));
	if (dlgManager->promptDialog(DialogInput))
	{
		QString name = dlgManager->getText();

		// prompt for the texture path
		// NOTE: for a dummy actor, the texture id is the texture path
		dlgManager->clearState();
		dlgManager->setPrompt(BtUtilStr->getString("CustomActorTexturePrompt"));
		if (dlgManager->promptDialog(DialogInput))
		{
			QString texturePath = dlgManager->getText();

			CustomActorInfo * info = new CustomActorInfo;
				info->name = name;
				info->textureId = winOS->GetUniqueID();
			DummyActorImpl * dummyImpl = new DummyActorImpl(info);
			CustomActor * actor = new CustomActor(info);
			texMgr->loadTexture(GLTextureObject(Load|Compress, info->textureId, texturePath, HiResImage, HighPriority));
		}
	}
}

void Key_CreateStickyNote()
{
	if (GLOBAL(settings).freeOrProLevel == AL_FREE && StickyNoteMenuActionCustomizer::hasExceededMaxNumStickyNotes())
	{
		printUnique("CreateStickyNote", BtUtilStr->getString("StickyNoteLimit"));
		return;
	}
	
	reauthorize(false);

	// show the dialog to edit it
	StickyNoteActor * stickyNote = CreateStickyNote();
	if (stickyNote)
	{
		// orient the post it
		const int distDelta = 15;
		stickyNote->setGlobalPosition(Vec3(float(rand() % distDelta), 35, float(rand() % distDelta)));
		stickyNote->setGlobalOrientation(Quat(35, Vec3(1,0,0)));
		stickyNote->launchEditDialog(true);
			
		// select the post it
		sel->clear();
		sel->add(stickyNote);

		statsManager->getStats().bt.interaction.actors.fs_types.postitnotesCreated++;
	}		
}

void Key_ToggleSharingWidget()
{
	LOG("Toggling sharing widget");
	vector<BumpObject *> webActors = scnManager->getBumpObjects(ObjectType(BumpActor, Webpage));
	for (int i = 0; i < webActors.size(); ++i) 
	{
		WebActor * actor = (WebActor *) webActors[i];
		if (actor->isSharingWidget()) 
		{
			FadeAndDeleteActor(actor);
			return;
		}
	}

	Key_CreateSharingWidget();
}

bool Key_IsSharingWidgetEnabled() {
	LOG("Querying sharing widget");
	vector<BumpObject *> webActors = scnManager->getBumpObjects(ObjectType(BumpActor, Webpage));
	for (int i = 0; i < webActors.size(); ++i) 
	{
		WebActor * actor = (WebActor *) webActors[i];
		if (actor->isSharingWidget()) 
		{
			return true;
		}
	}

	return false;
}

void Key_ToggleFacebookWidget()
{
	LOG("Toggling facebook widget");
	vector<BumpObject *> webActors = scnManager->getBumpObjects(ObjectType(BumpActor, Webpage));
	for (int i = 0; i < webActors.size(); ++i) 
	{
		WebActor * actor = (WebActor *) webActors[i];
		if (actor->isFacebookWidgetUrl()) 
		{
			FadeAndDeleteActor(actor);
			return;
		}
	}

	Key_CreateFacebookWidget();
}

bool Key_IsFacebookWidgetEnabled() {
	LOG("Querying facebook widget");
	vector<BumpObject *> webActors = scnManager->getBumpObjects(ObjectType(BumpActor, Webpage));
	for (int i = 0; i < webActors.size(); ++i) 
	{
		WebActor * actor = (WebActor *) webActors[i];
		if (actor->isFacebookWidgetUrl()) 
		{
			return true;
		}
	}

	return false;
}

void Key_ToggleCustomEmailActor()
{
	LOG("Toggling email widget");
	CustomActor * actor = scnManager->getCustomActor<EmailActorImpl>();
	if (actor)
	{
		// delete the email actor 
		FadeAndDeleteActor(actor);
		dismiss("EmailActorImpl_description");
	}
	else
	{
		// create the email actor 
		CreateCustomEmailActor();

		// notify the user
		CustomActor * actor = scnManager->getCustomActor<EmailActorImpl>();
		printTimedUnique("EmailActorImpl_description", 5, actor->getCustomActorInfo()->description);
	}	
}

void Key_ToggleCustomStickyNotePadActor()
{
	LOG("Toggling sticky note widget");
	CustomActor * actor = scnManager->getCustomActor<StickyNotePadActorImpl>();
	if (actor)
	{
		// delete the email actor 
		FadeAndDeleteActor(actor);

		// STICKYNOTE_TODO: destroy the tooltip if we remove it
	}
	else
	{
		// create the email actor 
		CreateCustomStickyNotePadActor();
	}	
}

bool Key_IsEmailWidgetEnabled() {
	LOG("Querying email widget");
	CustomActor * actor = scnManager->getCustomActor<EmailActorImpl>();
	if (actor)
		return true;
	return false;
}

bool Key_IsStickyNotePadWidgetEnabled() {
	LOG("Querying sticky note pad widget");
	CustomActor * actor = scnManager->getCustomActor<StickyNotePadActorImpl>();
	if (actor)
		return true;
	return false;
}

void Key_ToggleCustomPrinterActor()
{
	LOG("Toggling printer widget");
	CustomActor * actor = scnManager->getCustomActor<PrinterActorImpl>();
	if (actor)
	{
		// delete the printer actor 
		FadeAndDeleteActor(actor);
		dismiss("PrinterActorImpl_description");
	}
	else
	{
		// create the printer actor 
		CreateCustomPrinterActor();

		// notify the user
		CustomActor * actor = scnManager->getCustomActor<PrinterActorImpl>();
		printTimedUnique("PrinterActorImpl_description", 5, actor->getCustomActorInfo()->description);
	}	
}

bool Key_IsPrinterWidgetEnabled() {
	LOG("Querying printer widget");
	CustomActor * actor = scnManager->getCustomActor<PrinterActorImpl>();
	if (actor)
		return true;
	return false;
}

void Key_ToggleCustomFacebookActor()
{
	LOG("Toggling legacy facebook actor");
	assert(false);
	//deprecated
	CustomActor * actor = scnManager->getCustomActor<FacebookActorImpl>();
	if (actor)
	{
		// delete the printer actor 
		FadeAndDeleteActor(actor);
		dismiss("FacebookActorImpl_description");
	}
	else
	{
		// create the printer actor 
		CreateCustomFacebookActor();

		// notify the user
		CustomActor * actor = scnManager->getCustomActor<FacebookActorImpl>();
		printTimedUnique("FacebookActorImpl_description", 5, actor->getCustomActorInfo()->description);
	}	
}

void Key_ToggleCustomTwitterActor()
{
	LOG("Toggling twitter widget");
	CustomActor * actor = scnManager->getCustomActor<TwitterActorImpl>();
	if (actor)
	{
		// delete the printer actor 
		FadeAndDeleteActor(actor);
		dismiss("TwitterActorImpl_description");
	}
	else
	{
		// create the printer actor 
		CreateCustomTwitterActor();

		// notify the user
		CustomActor * actor = scnManager->getCustomActor<TwitterActorImpl>();
		printTimedUnique("TwitterActorImpl_description", 5, actor->getCustomActorInfo()->description);
	}	
}

bool Key_IsTwitterWidgetEnabled()
{
	LOG("Querying twitter widget");
	CustomActor * actor = scnManager->getCustomActor<TwitterActorImpl>();
	if (actor)
		return true;
	return false;
}

void Key_ToggleCustomFlickrActor()
{
	CustomActor * actor = scnManager->getCustomActor<FlickrActorImpl>();
	if (actor)
	{
		// delete the printer actor 
		FadeAndDeleteActor(actor);
		dismiss("FlickrActorImpl_description");
	}
	else
	{
		// create the printer actor 
		CreateCustomFlickrActor();

		// notify the user
		CustomActor * actor = scnManager->getCustomActor<FlickrActorImpl>();
		printTimedUnique("FlickrActorImpl_description", 5, actor->getCustomActorInfo()->description);
	}	
}

void Key_ToggleFPS()
{	
	GLOBAL(settings).drawFramesPerSecond=!GLOBAL(settings).drawFramesPerSecond;
}

void Key_RefreshFileActors()
{
	vector<BumpObject *> actors = sel->getBumpObjects();
	for (int i = 0; i < actors.size(); ++i)
	{
		if (actors[i]->getObjectType() == ObjectType(BumpActor, FileSystem))
			((FileSystemActor *)actors[i])->refreshThumbnail();
	}

	if (!actors.empty())
		printUnique("Key_RefreshFileActors", BtUtilStr->getString("RefreshDesktop"));
}

void Key_StartAutomatedDemo()
{
	Replayable * replay = scnManager->getReplayable();
	AutomatedDemo * demo = dynamic_cast<AutomatedDemo *>(replay);
	if (demo)
	{
		if (replay->getPlayState() == Replayable::Stopped)
		{
			// start the demo
			replay->play();
		}
		else
		{
			evtManager->interruptIdleTimer();
		}
	}
	else
	{
		// create a new automated demo
		demo = new AutomatedDemo;
		scnManager->setReplayable(demo);
		demo->play();
	}
}

void StartAutomatedJSONTesting()
{
	// Start recording the test results in the log
	testLogger->setIsLogging(true);
	TEST_LOG(QString("\n----------------------------------------------------\n"));
	TEST_LOG(QString("NEW TEST SESSION (" + QDateTime(QDateTime::currentDateTime()).toString()) + ")");
	
	// If a custom test file exists, then only run that file, otherwise run all .json files in the folder	
	QDir JSONTestScriptFolder = scnManager->JSONTestScriptPath;
	QStringList filter(QString("*.json"));
	QStringList JSONTestScriptFiles("customTests.json");
	if(!JSONTestScriptFolder.exists(JSONTestScriptFiles.first()))
		JSONTestScriptFiles = JSONTestScriptFolder.entryList(filter);
	
	Json::Value root;
	Json::Reader reader;

	// Set the tests as the new playable
	// This object is eventually deleted by the scene manager after playing
	AutomatedJSONTestRunner * testRunner = new AutomatedJSONTestRunner;
	scnManager->setReplayable(testRunner);

	// Iterate through test files
	for(int i = 0; i < JSONTestScriptFiles.size(); i++)
	{
		QString absScriptPath = JSONTestScriptFolder.absoluteFilePath(JSONTestScriptFiles[i]);
		QString testingDescStr = read_file_utf8(native(absScriptPath));

		// Read from the JSON file
		if (reader.parse(testingDescStr.toUtf8().constData(), root))
		{	
			//Iterate through tests within test file and add them to the runner
			for(int testIndex = 0; testIndex < root["TestSuite"].size(); testIndex++)
			{	
				QString JSONTestName = root["TestSuite"][testIndex].asCString();
				testRunner->addTestSuite(new AutomatedJSONTestSuite(JSONTestName, absScriptPath));
			}			
		}
	}

	testRunner->play();
}

void Key_StartAutomatedTradeshowDemo()
{
	Replayable * replay = scnManager->getReplayable();
	AutomatedTradeshowDemo * demo = dynamic_cast<AutomatedTradeshowDemo *>(replay);
	if (demo)
	{
		if (replay->getPlayState() == Replayable::Stopped)
		{
			// start the demo
			replay->play();
		}
		else
		{
			evtManager->interruptIdleTimer();
		}
	}
	else
	{
		// create a new automated demo
		demo = new AutomatedTradeshowDemo;
		scnManager->setReplayable(demo);
		demo->play();
	}
}


void Key_StartAutomatedTests()
{
	// Run the unit tests. Results are printed to the console window.
	static bool gtest_initialized = false;
	if (!gtest_initialized)
	{
		// Convert the Windows command line into the standard arg/argv
		int argc;
		LPWSTR *argv = CommandLineToArgvW((LPCWSTR)GetCommandLineA(), &argc);
		testing::InitGoogleTest(&argc, argv);
		gtest_initialized = true;
	}
	cout << "\nRunning unit tests:\n";
	RUN_ALL_TESTS();

	// Automated tests are disabled for now, as they don't really work.
	// Not commented out because we want to keep them compiling. At some point,
	// they should be revisited and fixed up.
	if (false)
	{
		// set the tests as the new playable
		AutomatedTestRunner * testRunner = new AutomatedTestRunner;
		scnManager->setReplayable(testRunner);
		set<AutomatedTestSuite *> testSuites = Singleton<AutomatedTestSuiteRegistry>::getInstance()->getTestSuites();
		set<AutomatedTestSuite *>::iterator iter = testSuites.begin();
		while (iter != testSuites.end())
		{
			testRunner->addTestSuite(*iter);
			iter++;
		}

		// notify the user
		cout << endl
			<< "Starting Automated Tests" << endl
			<< "------------------------" << endl;

		testRunner->play();
	}
}

// Given a root and a keyPath will return the value for the keyPath
// This method will throw an invalid argument exception if the keyPath doesn't exist
Json::Value getValueFromRoot (QString keyPath, Json::Value root)
{
	QStringList tokens = keyPath.split(".");
	Json::Value node = root;
	for (int i = 0; i < tokens.size(); ++i)
	{
		// ensure valid key path
		if (!node.isMember(stdString(tokens[i])))
		{
#ifdef BTDEBUG
			QString err = "No such key path exists:\n" + keyPath;
			::MessageBox(winOS->GetWindowsHandle(), (LPCTSTR) err.utf16(), L"BumpTop Util Error", MB_OK | MB_ICONERROR);
#endif
			return Json::Value();
		}

		node = node[stdString(tokens[i])];
	}
	return node;
}

void loadDemoSceneFromDirectory(QDir sourceDir, bool showInfoControl)
{
	Actor *actor;

	// Gather necessary data to place actors
	vector<NxActorWrapper*> walls = GLOBAL(Walls);
	Bounds frontWall = walls[0]->getBoundingBox();
	Bounds backWall = walls[1]->getBoundingBox();
	Bounds leftWall = walls[3]->getBoundingBox();
	Vec3 frontCenter, frontExtents, backCenter, backExtents, corner, leftCenter, leftExtents;
	frontWall.getCenter(frontCenter);
	frontWall.getExtents(frontExtents);
	backWall.getCenter(backCenter);
	backWall.getExtents(backExtents);
	leftWall.getCenter(leftCenter);
	leftWall.getExtents(leftExtents);
	Quat oriForFrontWall (walls[0]->getGlobalOrientationQuat());
	Quat oriForLeftWall (walls[3]->getGlobalOrientationQuat());

	if (showInfoControl)
	{
		CornerInfoControl *cornerInfoControl = cam->getCornerInfoControl();
		cornerInfoControl->init();
		cornerInfoControl->setMessageText(QT_TRANSLATE_NOOP("BtUtilStrings", "BumpTop Demo Scene\nTo refresh the scene hit F7\nTo start the automatic demo hit CTRL + F7"));
	}

	// Prepare new Scene
	// Clear out all files from the tradeshow directory
	// NOTE: Not sure if there's any particular reason to use a directory in
	// the user's %APPDATA% -- maybe use a temp dir instead?
	QString workingDirectory = native(winOS->GetTradeshowDirectory());
	StrList dirContents = fsManager->getDirectoryContents(workingDirectory);
	for_each(QString fileName, dirContents)
	{
		fsManager->deleteFileByName(fileName, true, true); // delete silently and skip recycle bin
	}

	// copy files over that we need to the working directory
	QDir currentWorkingDirectory = scnManager->getWorkingDirectory();
	StrList sourceFiles = fsManager->getDirectoryContents(native(sourceDir));
	for (int i = 0; i < sourceFiles.size(); ++i)
	{
		QString fileName = filename(sourceFiles[i]);
		if (!currentWorkingDirectory.exists(fileName) && 
			fileName != "tradeshow.json" &&
			fileName != "Thumbs.db")
		{
			fsManager->copyFileByName(sourceFiles[i], workingDirectory, fileName, false, true);
		}
	}

	GLOBAL(skipSavingSceneFile) = true; // this will prevent saving of the scene file

	// Switch to the working directory, clear the scene, and load the new one
	// NOTE: We may want to keep track of the current working dir so we can switch back

	scnManager->setWorkingDirectory(workingDirectory);
	ClearBumpTop();
	CreateBumpObjectsFromDirectory(native(workingDirectory));

	vector<FileSystemActor *> files = scnManager->getFileSystemActors();
	
	// Open Json file
	Json::Value rootJsonValue;
	Json::Reader reader;
	QString fileName = "/tradeshow.json";
	QString pathToFile = sourceDir.absolutePath() + fileName;
	QString jsonSceneFile = read_file_utf8(pathToFile);

	// Json Value objects to store different values
	Json::Value position, relativeTo, pinWall, grow;
	std::string fileProperty, memberName;
	unsigned int positionIndex = 0; // Used to index into position array
	int fileIndex = 0;	// The index for a specific file in the files vector
	NxReal newX, newZ;	// Used when computing the global position
	bool fileFound;		// The file was found in the files vector
	Vec3 actorDims;		// Used to store the dimensions of the actor

	// Read Json file
	if (reader.parse(jsonSceneFile.toUtf8().constData(), rootJsonValue))
	{
		// Get list of members
		vector<string> members = rootJsonValue.getMemberNames();
		for (int i = 0; i < rootJsonValue.size() ; i++)
		{
			memberName = members[i];
			fileFound = false;
			// Find the index of the file with name memberName inside files vector
			for (int j = 0; j < files.size(); j++)
			{
				if (files[j]->getFileName() == QString(memberName.c_str()))
				{
					actor = files[j];
					fileIndex = j;
					fileFound = true;
					break;
				}					
			}
			
			// If the file wasn't found skip trying to position it
			if (!fileFound)
			{
				if (memberName == "Twitter")
					actor = scnManager->getCustomActor<TwitterActorImpl>();
				else
					continue;
			}

			// Store the file actor dimensions
			actorDims = actor->getDims();
			
			// fileProperty is used to store the path for the property we wish to load
			fileProperty = memberName;
			
			// Get position
			fileProperty.append(".position");
			position = getValueFromRoot(QString(fileProperty.c_str()), rootJsonValue);

			// Get relative to
			fileProperty = memberName;
			fileProperty.append(".relativeTo");
			relativeTo = getValueFromRoot(QString(fileProperty.c_str()), rootJsonValue);

			// get pinWall
			fileProperty = memberName;
			fileProperty.append(".pinWall");
			pinWall = getValueFromRoot(QString(fileProperty.c_str()), rootJsonValue);

			// get grow
			fileProperty = memberName;
			fileProperty.append(".grow");
			grow = getValueFromRoot(QString(fileProperty.c_str()), rootJsonValue);		
			
			// Determine what to use as a baseline
			if (relativeTo.isString() && relativeTo.asString() == "frontWall")
			{
				newX = frontExtents.x;
				newZ = frontCenter.z;
			}
			else if (relativeTo.isString() && relativeTo.asString() == "backWall")
			{
				newX = backExtents.x;
				newZ = backCenter.z;
			}
			else if (relativeTo.isString() && relativeTo.asString() == "leftWall")
			{
				newX = leftCenter.x;
				newZ = leftExtents.z;
			}

			// Grow object
			if (grow.isNumeric() && grow.asInt() > 0)
			{
				actor->grow(1, grow.asInt());
			}

			// Set position
			if (position.isArray())
			{
				actor->setGlobalPosition(Vec3(newX * position[positionIndex].asDouble(), frontCenter.y * position[positionIndex + 1].asDouble(), newZ * position[positionIndex + 2].asDouble()));
			}

			// Pin object on wall
			if (pinWall.isString() && pinWall.asString() == "front")
			{
				actor->setGlobalPosition(actor->getGlobalPosition() - Vec3(0, 0, actorDims.z));
				actor->setGlobalOrientation(oriForFrontWall);
				actor->onPin();
			}
			else if (pinWall.isString() && pinWall.asString() == "left")
			{
				actor->setGlobalPosition(actor->getGlobalPosition() - Vec3(actorDims.x, 0, 0));
				actor->setGlobalOrientation(oriForLeftWall);
				actor->onPin();
			}
		}		
	}
}

void Key_LoadTradeshowScene()
{
	QDir tradeshowSourceDirectory = parent(winOS->GetTexturesDirectory()) / "Tradeshow";
	loadDemoSceneFromDirectory(tradeshowSourceDirectory, true);
}

void Key_ReloadScene()
{
	GLOBAL(isInTrainingMode) = false;
	GLOBAL(skipSavingSceneFile) = false;

	ClearBumpTop();

	assert(winOS->GetLibraryManager());
	scnManager->setCurrentLibrary(winOS->GetLibraryManager()->getDesktopLibrary());
	
	if (!scnManager->startupDemoSceneDir.isNull())
	{
		printUnique("RELOAD_SCENE_MSG", QT_TRANSLATE_NOOP("BtUtilStrings", "Reloading scene..."));
		loadDemoSceneFromDirectory(QDir(scnManager->startupDemoSceneDir));
	}
	else
	{
		if (!LoadSceneFromFile())
			CreateBumpObjectsFromWorkingDirectory();
	}
	cam->getCornerInfoControl()->disable();
	
}
boost::function<void()> _onInvokeCommandHandler;

void setOnInvokeCommandHandler( boost::function<void()> onInvokeCommandHandler )
{
	_onInvokeCommandHandler = onInvokeCommandHandler;
}


void pDeleteSelection(bool skipRecycleBin)
{
	vector<BumpObject *> freeItemsToDelete = sel->getBumpObjects();
	vector<FileSystemActor *> delFiles, failedObj;
	vector<Pile *> pilesToDelete = sel->getFullPiles();

	for (uint i = 0; i < pilesToDelete.size(); i++)
	{
		Pile * p = pilesToDelete[i];
		if (p->getPileType() == HardPile)
		{
			// for hard piles that are expanded from a link, delete the link itself
			// and not the pile of items
			FileSystemPile * fsPile = (FileSystemPile *) p;
			FileSystemActor * owner = fsPile->getOwner();
			if (owner->isFileSystemType(Link))
			{
				fsPile->folderize(false);
				freeItemsToDelete.push_back(owner);
			}
		}
		else if (p->getPileType() == SoftPile)
		{
			// for small piles, we can just delete the actors themselves and clear the pile
			const vector<BumpObject *>& pileItems = p->getPileItems();
			for (uint j = 0; j < pileItems.size(); j++)
				freeItemsToDelete.push_back(pileItems[j]);
			p->clear(-1);
		}
		
		// if we are removing the pile, then we need to remove it from the list to delete
		for (unsigned int j = 0; j < freeItemsToDelete.size(); j++)
		{
			if (freeItemsToDelete[j] == p)
			{
				freeItemsToDelete.erase(freeItemsToDelete.begin() + j);
				break;
			}
		}
	}

	for (uint i = 0; i < freeItemsToDelete.size(); i++)
	{
		ObjectType type = freeItemsToDelete[i]->getObjectType();

		if (type == ObjectType(BumpActor, FileSystem, PhotoFrame))
		{
			// ensure that the photo frame sources flush their cache
			PhotoFrameActor * pfActor = (PhotoFrameActor *) freeItemsToDelete[i];
			pfActor->flushCache();

			// delete the actor only 
			FadeAndDeleteActor((Actor *) freeItemsToDelete[i]);
		}
		else if (type == ObjectType(BumpActor, FileSystem) &&
			!(type == ObjectType(BumpActor, FileSystem, LogicalVolume)))
		{
			delFiles.push_back((FileSystemActor *) freeItemsToDelete[i]);
		}
		else
		{
			FadeAndDeleteActor((Actor *) freeItemsToDelete[i]);
		}
	}

	// Delete all the files in the selection
	if (!GLOBAL(isInTrainingMode)) {
		fsManager->deleteFiles(delFiles, failedObj, true, skipRecycleBin);
	} else {
		foreach(FileSystemActor* obj, delFiles)
			fsManager->onFileRemoved((LPCWSTR)obj->getFileName().utf16());
	}

	sel->clear();
}

void Key_DeleteSelection()
{
	pDeleteSelection(false);
}	

void Key_DeleteSelectionSkipRecycleBin()
{
	pDeleteSelection(true);
}

void Key_ToggleAxisAlignMode()
{
// 	globalSettings.AxisAlignedMode = !globalSettings.AxisAlignedMode;
// 
// 	if (globalSettings.AxisAlignedMode)
// 	{
// 		DisableRotation(activeNxActorList);
// 	}else{
// 		EnableRotation(activeNxActorList);
// 	}
}

void Key_ZoomToSavedCamera()
{
	if (!cam->isCameraFreeForm() && (GLOBAL(settings).cameraPreset.startsWith("def") || GLOBAL(settings).cameraPreset.isEmpty()) || GLOBAL(isInInfiniteDesktopMode))
		cam->zoomToAngledView();
}

void Key_ZoomToAll()
{
	cam->loadCameraFromPreset(GLOBAL(settings).cameraPreset);
}

void Key_SetCameraAsTopDown()
{
	// Move the camera into position so it encapsulates the entire work area
	Bounds bounds;

	// Find out the region where this camera will lohWok at
	for (int i = 0; i < 4; i++)
	{
		Vec3 wallsPos = GLOBAL(WallsPos)[i];
		wallsPos.y = 0;
		bounds.include(wallsPos);
	}

	// Move the camera to look at that location
	NxVec3 boundsView = calculateEyeForBounds(bounds);
	// Move the camera to slightly above bounds
	boundsView.add(boundsView,Vec3(0.0f,25.0f,0.0f));

	cam->setOrigin(boundsView);
	cam->animateTo(boundsView, Vec3(0,-1,0), Vec3(0,0,1));
}

void Key_MoveToDesktopOne()
{
	Vec3 up;
	up.cross(Vec3(0,-0.9f,0.4f), Vec3(0, 1, 0));
	up.cross(up, Vec3(0,-0.9f,0.4f));
	cam->animateTo(Vec3(500.7f, 187.0f, -105.0f), Vec3(0,-0.9f,0.4f), Vec3(0,1,0));
}

void Key_MoveToDesktopTwo()
{
	Vec3 up;
	up.cross(Vec3(0,-0.9f,0.4f), Vec3(0, 1, 0));
	up.cross(up, Vec3(0,-0.9f,0.4f));
	cam->animateTo(Vec3(-500.7f, 187.0f, -105.0f), Vec3(0,-0.9f,0.4f), Vec3(0,1,0));
}

void Key_MoveToDesktopThree()
{
	Vec3 up;
	up.cross(Vec3(0,-0.9f,0.4f), Vec3(0, 1, 0));
	up.cross(up, Vec3(0,-0.9f,0.4f));
	cam->animateTo(Vec3(7.7f, 187.0f, 405.0f), Vec3(0,-0.9f,0.4f), Vec3(0,1,0));
}

void Key_NameSoftPile()
{	
	vector<Pile *> selList = sel->getFullPiles();
	vector<BumpObject *> objList;

	if (selList.empty())
		return;

	Pile * p = selList.front();
	if (!p) 
		return;

	// Pop out the items that cannot be added
	for (uint j = 0; j < p->getNumItems(); j++)
	{
		FileSystemActor *fsActor = (FileSystemActor *) (*p)[j];

		// Check to see if we can put this item in a pile
		if (fsActor && !fsActor->isPilable(HardPile))
		{
			MessageClearPolicy clearPolicy;
				clearPolicy.setTimeout(4);
			scnManager->messages()->addMessage(new Message("Key_NameSoftPile", BtUtilStr->getString("ErrorFolderize"), Message::Warning, clearPolicy));
			return;
		}else{
			objList.push_back(fsActor);
		}
	}

	// Nothing can be added, return
	if (objList.size() == 0)
		return;

	dlgManager->clearState();
	dlgManager->setCaption(BtUtilStr->getString("NamePileCaption"));
	dlgManager->setPrompt(BtUtilStr->getString("PileNamePrompt"));
	dlgManager->setText("Piled Stuff");
	if (dlgManager->promptDialog(DialogInput2))
	{
		FileSystemPile *fsPile = new FileSystemPile();
		QString newPath = native(fsManager->getUniqueNewFolderPathInWorkingDirectory(dlgManager->getText()));
		if (fsPile->convert(p, newPath))
		{
			fsManager->addObject(fsPile);
		}else{
			fsPile->clear();
			DeletePile(fsPile, false, false, false);
		}
	}
}

void Key_SetCameraAsAngled()
{
	cam->popAllWatchActors();
	cam->zoomToAngledView();
}

// Called by Ctrl+Shift+W
// This method zooms the camera in
void Key_MoveCameraForward()
{
	// Determine what point in the bumptop desktop the camera is facing
	Ray cameraRay = Ray(cam->getEye(), cam->getDir());
	tuple<int, NxReal, Vec3, NxPlane> t = RayIntersectDesktop(cameraRay);
	Vec3 pointToMoveTo = t.get<2>();
	cam->scrollToPoint(pointToMoveTo);
}


// Called by Ctrl+Shift+S
// This method zooms the camera out
void Key_MoveCameraBackwards()
{
	Ray cameraRay = Ray(cam->getEye(), -1 * cam->getDir());
	tuple<int, NxReal, Vec3, NxPlane> t = RayIntersectDesktop(cameraRay);
	Vec3 pointToMoveTo = t.get<2>();

	// Reflect pointToMove along dir
	cam->scrollToPoint(pointToMoveTo);
}


// Called by Ctrl+Shift+D
// This method moves the camera right relative to
// the direction of the camera.
void Key_MoveCameraRight()
{
	// Construct right direction
	Vec3 rightDir = cam->getDir().cross(cam->getUp());
	rightDir.normalize();

	// Determine what point to scroll to
	Ray rightRay(cam->getEye(), rightDir);
	tuple<int, NxReal, Vec3, NxPlane> t = RayIntersectDesktop(rightRay);
	Vec3 pointToMoveTo = t.get<2>();

	cam->scrollToPoint(pointToMoveTo);
}

// Called by Ctrl+Shift+A
// This method moves the camera left relative to
// the direction of the camera.
void Key_MoveCameraLeft()
{
	// Construct left direction
	Vec3 leftDir = cam->getUp().cross(cam->getDir());
	leftDir.normalize();

	// Determine what point to scroll to
	Ray leftRay(cam->getEye(), leftDir);
	tuple<int, NxReal, Vec3, NxPlane> t = RayIntersectDesktop(leftRay);
	Vec3 pointToMoveTo = t.get<2>();

	cam->scrollToPoint(pointToMoveTo);
}

void Key_ShootCuboid()
{
	// Figure out how to shoot something out
	/*
	Create NxBullet actor and create a bullet here
	then in the onRender callback use draw sphere
	Actor* bullet= new Actor();
	bullet->setGlobalPosition(cam->getEye());
	bullet->setDims(Vec3(1,1,1));
	bullet->setLinearVelocity(cam->getDir()*300);
	bullet->setAngularMomentum(Vec3(40,10,100));
	*/
}
void Key_ToggleDrawText()
{	
	GLOBAL(settings).RenderText= !GLOBAL(settings).RenderText;
}

void Key_ToggleCPUOptimization()
{	
	// Toggle global Optimizations
	GLOBAL(settings).useCPUOptimizations = !GLOBAL(settings).useCPUOptimizations;

	QString msg = GLOBAL(settings).useCPUOptimizations ? QT_TRANSLATE_NOOP("BtUtilStrings", "Optimizations On") : QT_TRANSLATE_NOOP("BtUtilStrings", "Optimizations Off");
	scnManager->messages()->addMessage(new Message("Optimizations", msg, Message::Ok));	
}

void Key_ToggleWindowMode()
{
	winOS->ToggleWindowMode();
}

void Key_PreviousSlide()
{
#ifdef ENABLE_SLIDESHOW_CLASS
	slideShow->prevSlide();
#endif
}

void Key_NextSlide()
{
#ifdef ENABLE_SLIDESHOW_CLASS
	slideShow->nextSlide();
#endif
}

void Key_RepollDesktop()
{
	
	Key_ClearBumpTop();

	if (GetDesktopItems().size() == 0)
	{
		
		dlgManager->clearState();
		dlgManager->setPrompt(BtUtilStr->getString("SelectDirectory"));
		
		if (dlgManager->promptDialog(DialogInputBrowse))
		{
			CreateBumpObjectsFromDirectory(dlgManager->getText());
			Key_SetCameraAsAngled();
		}
	}
}

void Key_ToggleSlideShow()
{
	// ensure that we aren't already in slideshow mode
	if (isSlideshowModeActive())
	{
		Key_DisableSlideShow();
	}
	else if (!cam->isAnimating()) // We should only be entering the slideshow when the camera is not animating
		Key_EnableSlideShow();
}

void Key_DisableSlideShow()
{
	// ensure that we are already in slideshow mode
	if (isSlideshowModeActive())
	{
		BumpObject *lastWatched = cam->getHighlightedWatchedActor();
		if(lastWatched && lastWatched->getObjectType() == ObjectType(BumpActor, FileSystem, PhotoFrame)) //If the object being watched is a photoframe
			cam->highlightWatchedActor(NULL,NULL);

		cam->setSlideshow(false);
		cam->popWatchActors();
		GLOBAL(touchGestureBrowseMode) = false; //This should be set to false
		GLOBAL(useSingleClickExpiration) = false;
		sel->setPickedActor(NULL);

		if (!GLOBAL(isInInfiniteDesktopMode)) 
#ifdef TABLE
			Key_SetCameraAsTopDown();
#else
			cam->loadCameraFromPreset(GLOBAL(settings.cameraPreset));
#endif
		cam->getRestoreOriginalPhotoControl()->hide();
	}
}



void Key_EnableSlideShow()
{
	Key_UnsetSharingMode();

	// enable touch browsing as necessary
	if (!GLOBAL(settings).enableTouchGestureBrowse) return;

	//start watching the selection, and highlight the first actor	
	vector<BumpObject *> objs = sel->getBumpObjects();
	if (!objs.empty())
	{
		// reduce the selection set to only filesystem image actors
		vector<BumpObject *>::iterator iter = objs.begin();
		while (iter != objs.end())
		{
			BumpObject * obj = (*iter);
			Pile * p = NULL;
			if (obj->getParent())
				p = dynamic_cast<Pile *>(obj->getParent());

			bool isValidImage = (obj->getObjectType() == ObjectType(BumpActor, FileSystem, Image));
			if (isValidImage)
			{
				// ensure that we are not in a stacked pile
				if (p)
				{
					const vector<BumpObject *>& pileItems = p->getPileItems();
					if (p->getPileState() == Stack)
						iter = objs.erase(iter);
					else if (p->getPileState() == Leaf && p->getLeafIndex())
					{
						if (p->getActiveLeafItem() == obj)
							iter++;
						else
							iter = objs.erase(iter);
					}
					else
						iter++;
				}
				else
					iter++;
			}
			else
				iter = objs.erase(iter);
		}

		// if there is only one image actor, use all images in the scene as the slideshow
		// set. If there are no image actors don't do anything
		BumpObject * initialHighlightActor = 0;
		if (objs.size() == 1)
		{
			// use the selection actor if there is one
			if (objs.size() > 0)
				initialHighlightActor = objs[0];
			vector<FileSystemActor *> imageActors = scnManager->getFileSystemActors(Thumbnail, true);
			objs.assign(imageActors.begin(), imageActors.end());			
		}
		else if (objs.empty())
			return;
		
		cam->setSlideshow(true);

		// Since we are in slide show mode enable touch gesture browsing
		GLOBAL(touchGestureBrowseMode) = true; //This should be set to true
		GLOBAL(useSingleClickExpiration) = false;

		//TODO:  Sort images by left/right reading order. 

		// zoom into the selection (highlighting the first obj if none was selected)
		if (!initialHighlightActor)
			initialHighlightActor = objs[0];
		cam->pushWatchActors(objs, true);
		cam->highlightWatchedActor(initialHighlightActor);

		// record this slideshow inititation
		statsManager->startTimer(statsManager->getStats().bt.window.slideshowTime);
	}
}

void Key_ClearBumpTop()
{
	dlgManager->clearState();
	dlgManager->setPrompt(BtUtilStr->getString("ClearBumpTopPrompt"));

	if (GetDesktopItems().size() == 0 || dlgManager->promptDialog(DialogYesNo))
	{
		QFileInfo p = scnManager->getSceneBumpFile();
		if (exists(p))
		{
			// Write out the contents of the file
			FILE *fp = _wfopen((LPCWSTR) native(scnManager->getSceneBumpFile()).utf16(), L"r");
			FILE *out = _wfopen((LPCWSTR) native(scnManager->getSceneBumpBackupFile()).utf16(), L"w");
			if (fp && out)
			{
				fseek (fp, 0, SEEK_END);
				long lSize = ftell (fp);
				rewind (fp);

				char *c = new char[lSize];
				fread(c, 1, lSize, fp);
				fwrite(c, 1, lSize, out);
				SAFE_DELETE_ARRAY(c);

				fclose(fp);
				fclose(out);
			}
		}

		SaveSceneToFile();

		//Delete ALL Stuff (works!)
		while(GLOBAL(getPiles()).size()>0)
		{
			for(int i=0; i < GLOBAL(getPiles()).size(); i++)
			{
				Pile *p = (Pile *) GLOBAL(getPiles())[i];

				if (!p->getParent())
				{
					if (p->getPileType() == HardPile)
					{
						FileSystemPile *fsPile = (FileSystemPile *) p;

						fsPile->folderize(false);
					}

					DeletePile(GLOBAL(getPiles())[i], true, false, true);
				}
			}
		}

		ClearBumpTop();
	}
}

void Key_SetCameraOnSelection()
{
	Vec3 camView(0.0f);
	Bounds bounds;
	Box box;
	vector <NxActorWrapper *> ActorList;

	// Get the list of things on the desktop
	ActorList = sel->getLegacyActorList();

	if (ActorList.size() > 0)
	{
	
		// place the camera above the center point
		for (int i = 0; i < ActorList.size(); i++)
		{
			NxActorWrapper *data = ActorList[i];
			Actor *d = GetBumpActor(data);
				
			bounds.combine(data->getBoundingBox());
		}

		zoomToBounds(bounds);
	}
}

void Key_ToggleOtherBumpTops()
{
	
	GLOBAL(settings).DrawOtherBumpTops = !GLOBAL(settings).DrawOtherBumpTops;

	if (GLOBAL(settings).DrawOtherBumpTops)
	{
		cam->animateTo(Vec3(7.7f, 187.0f * 5, -105.0f * 3), Vec3(0,-0.9f,0.4f), Vec3(0,0,1));
	}else{
#ifdef TABLE			
		Key_SetCameraAsTopDown();
#else
		cam->loadCameraFromPreset(GLOBAL(settings.cameraPreset));
#endif
	}

	for (int i = 0; i < GLOBAL(Walls).size(); i++)
	{
		if (GLOBAL(settings).DrawOtherBumpTops)
		{
			// Make the GLOBAL(Walls) un-bumpable
			GLOBAL(Walls)[i]->raiseActorFlag(NX_AF_DISABLE_COLLISION);

			// move the walls 500 units down the Y axis
			Vec3 Pos = GLOBAL(Walls)[i]->getGlobalPosition();
			Pos.y = 5000;
			GLOBAL(Walls)[i]->setGlobalPosition(Pos);
			GLOBAL(DrawWalls) = false;

		}else{
			GLOBAL(Walls)[i]->clearActorFlag(NX_AF_DISABLE_COLLISION);

			// move the walls 500 units down the Y axis
			Vec3 Pos = GLOBAL(Walls)[i]->getGlobalPosition();
			Pos.y = 180;
			GLOBAL(Walls)[i]->setGlobalPosition(Pos);
			GLOBAL(DrawWalls) = true;
		}
	}
}

void Key_RenameSelection()
{
	// Only rename if a selection is 1 item
	if (sel->getSize() == 1)
	{
		BumpObject *obj = sel->getBumpObjects()[0];		
		
		Renamer->rename(obj, true);
	}
	else
	{
		printUnique("Key_RenameSelection", BtUtilStr->getString("ErrorRenameSelection"));
	}
}

void Key_LogoutServiceSelection()
{
	// Only rename if a selection is 1 item
	if (sel->getSize() == 1)
	{
		BumpObject *obj = sel->getBumpObjects()[0];

		if (obj->getObjectType() == ObjectType(BumpActor, Custom))
		{
			CustomActor * actor = (CustomActor *) obj;
			if (actor == scnManager->getCustomActor<FacebookActorImpl>())
			{
				GLOBAL(settings).fbc_uid.clear();
				GLOBAL(settings).fbc_session.clear();
				GLOBAL(settings).fbc_secret.clear();
				winOS->SaveSettingsFile();
				printUnique("Key_LogoutServiceSelection", BtUtilStr->getString("ErrorReloginSelectionFacebook"));
			}
			else if (actor == scnManager->getCustomActor<TwitterActorImpl>())
			{
				GLOBAL(settings).tw_login.clear();
				GLOBAL(settings).tw_password.clear();
				GLOBAL(settings).tw_oauth_key.clear();
				GLOBAL(settings).tw_oauth_secret.clear();
				winOS->SaveSettingsFile();
				printUnique("Key_LogoutServiceSelection", BtUtilStr->getString("ErrorReloginSelectionTwitter"));
			}
			else
			{
				printUnique("Key_LogoutServiceSelection", BtUtilStr->getString("ErrorReloginSelectionUnsupported"));
			}
		}
		else
		{
			printUnique("Key_LogoutServiceSelection", BtUtilStr->getString("ErrorReloginSelection"));
		}
	}
	else
	{
		printUnique("Key_LogoutServiceSelection", BtUtilStr->getString("ErrorReloginSelection"));
	}
}

void Key_GridLayoutSelection()
{
	// grab the selection, and grid the pile
	vector<BumpObject *> selected = sel->getBumpObjects();
	orderSpatially2D(selected);

	if (!selected.empty())
	{
		// grid each of the items
		Vec3 dims(0.0f), center(0.0f);
		
		// determine the center of the selection set
		Bounds selectionBounds;
			selectionBounds.setEmpty();
		for (uint i = 0; i < selected.size(); ++i)
			selectionBounds.combine(selected[i]->getBoundingBox());
		selectionBounds.getCenter(center);

		// Calculate the size of the grid spacing relative to the largest item in the grid
		float gridSpacingX = 10.0f; // Horizontal spacing of the grid layout
		float gridSpacingZ = 8.0f; // Vertical spacing of the grid layout
		int gridSizeX = int(ceil(sqrt(float(selected.size()))));
		int gridSizeZ = selected.size() / gridSizeX + ((selected.size() % gridSizeX > 0) ? 1 : 0);

		// Find the largest dimensions in the selection
		for (uint i = 0; i < selected.size(); i++)
		{
			Vec3 indvDims = selected[i]->getDims();

			if (dims.x < indvDims.x) dims.x = indvDims.x;
			if (dims.z < indvDims.y) dims.z = indvDims.y;
		}

		Box expectedGridBox(center, 
			Vec3(
			(dims.x * gridSizeX) + (gridSpacingX * NxMath::max(0, (gridSizeX - 1))), 
				0.0f,
				(dims.z * gridSizeZ) + (gridSpacingZ * NxMath::max(0, (gridSizeZ - 1))) 
				),
			Mat33(NX_IDENTITY_MATRIX));

		Box desktopBox = GetDesktopBox();
		Box normalizedBox(desktopBox.center, expectedGridBox.extents, expectedGridBox.rot);
		if (normalizedBox.isInside(desktopBox))
		{
			if (adjustBoxToInsideWalls(expectedGridBox))
			{
				center = expectedGridBox.center;
			}
		}


		// Index Structure:
		// +---+  +---+  +---+
		// | 0 |  | 1 |  | 2 |
		// +---+  +---+  +---+
		//                    
		// +---+  +---+  +---+
		// | 3 |  | 4 |  | 5 |
		// +---+  +---+  +---+
		//                    
		// +---+  +---+  +---+
		// | 6 |  | 7 |  | 8 |
		// +---+  +---+  +---+
		// Find out where to start the grid and calculate outwards form the center
		float halfGridDimX = (((gridSizeX) * (dims.x * 2)) + ((gridSizeX - 1) * gridSpacingX)) / 2 - dims.x;
		float halfGridDimZ = (((gridSizeZ) * (dims.z * 2)) + ((gridSizeZ - 1) * gridSpacingZ)) / 2 - dims.z;
		center += Vec3(halfGridDimX, 0, halfGridDimZ);

		// Move the Actors into their rightful places on the grid
		Vec3 pos(0.0f);
		uint locX = 0, locY = 0;
		for (int i = 0; i < selected.size(); i++)
		{
			// Jump to next Row
			if (locX >= gridSizeX)
			{
				locX = 0;
				locY++;
			}

			// New position - Spacing + half Dims of each item over
			pos = Vec3(locX * ((dims.x * 2) + gridSpacingX), dims.y, locY * ((dims.z * 2) + gridSpacingZ));
			locX++;

			// Calculate the relative Positions
			ZeroAllActorMotion(selected[i]);
			selected[i]->setPoseAnim(selected[i]->getGlobalPose(), Mat34(GLOBAL(straightIconOri), center-pos), 25);
		}

		// note the stat
		statsManager->getStats().bt.interaction.layout.laidOutInGrid++;
	}
}

void Key_SearchSubString()
{
	dlgManager->clearState();
	dlgManager->setPrompt(BtUtilStr->getString("SearchFilenames"));

	// prompt the user for a sub string to search for 
	if (dlgManager->promptDialog(DialogInput))
	{
		statsManager->getStats().bt.interaction.search.searchSubstring++;
		Finder->searchString = dlgManager->getText();
		Finder->search();
		
	}
}

////
////

void Key_LaunchSelection()
{
	vector<BumpObject *> actorListing = sel->getBumpObjects();

	// ask whether we want to launch 'em all, or, no thanks/oops my mistake
	// have a threshold of maybe 2 items
	if (actorListing.size() > 2)
	{
		QString msg = QString(BtUtilStr->getString("LaunchPrompt")).arg(actorListing.size());
		dlgManager->clearState();
		dlgManager->setPrompt(msg);
		if (!dlgManager->promptDialog(DialogYesNo))
			return;
	}

	for (uint i = 0; i < actorListing.size(); i++)
	{
		BumpObject *obj = actorListing[i];

		if (obj->isBumpObjectType(BumpActor))
		{
			Actor *aData = (Actor *) obj;

			if (aData->isActorType(FileSystem) || 
				aData->isActorType(Custom))
			{
				((FileSystemActor *) aData)->onLaunch();
			}
		}else if (obj->isBumpObjectType(BumpPile))
		{
			Pile *pile = (Pile *) obj;
			Vec3 pos = pile->getGlobalPosition();
			pos.y = repoManager->getPlaneFloorLevel() + PILE_SLATE_THICKNESS + PILE_SLATE_SPACE_ADJUSTMENT;
			pile->grid(pos);
		}
	}
}

void Key_ToggleInfiniteDesktopMode()
{
	if (!GLOBAL(settings).DrawOtherBumpTops)
	{
		Vec3 mainWorkspaceDims = Vec3(abs(GLOBAL(WallsPos)[2].x - GLOBAL(WallsPos)[3].x) - (getDimensions(GLOBAL(Walls)[3]).z + getDimensions(GLOBAL(Walls)[2]).z), 
			abs(GLOBAL(WallsPos)[0].z - GLOBAL(WallsPos)[1].z) - (getDimensions(GLOBAL(Walls)[0]).z + getDimensions(GLOBAL(Walls)[1]).z), 0);		
		mainWorkspaceDims /= -GLOBAL(factor);

		if (!GLOBAL(isInInfiniteDesktopMode))
		{
			MessageClearPolicy clearPolicy;
				clearPolicy.setTimeout(1);
			scnManager->messages()->addMessage(new Message("Key_ToggleInfiniteDesktopMode", BtUtilStr->getString("InfiniteBumpTop"), Message::Ok, clearPolicy));

			// move the walls to the max bounds of the background desktop image
			Vec3List tempWallsPos = GLOBAL(WallsPos);

			// we get these dimensions from the render callback/draw background code
			mainWorkspaceDims *= 7;
			ResizeWallsToWorkArea((int)mainWorkspaceDims.x, (int)mainWorkspaceDims.y);

			// restore the temp wall positions
			GLOBAL(WallsPos) = tempWallsPos;
			GLOBAL(isInInfiniteDesktopMode) = true;
			
			// set the camera to watch the whole selection
			cam->pushWatchActors(scnManager->getBumpObjects());
			GLOBAL(DrawWalls) = false;
		}
		else
		{
			MessageClearPolicy clearPolicy;
				clearPolicy.setTimeout(1);
			scnManager->messages()->addMessage(new Message("Key_ToggleInfiniteDesktopMode", BtUtilStr->getString("NormalBumpTop"), Message::Ok, clearPolicy));

			ResizeWallsToWorkArea((int)mainWorkspaceDims.x, (int)mainWorkspaceDims.y);

			// restore the walls
			for (uint i = 0; i < 4; ++i)
				GLOBAL(Walls)[i]->setGlobalPosition(GLOBAL(WallsPos)[i]);

			cam->popAllWatchActors();

			// reset the camera to normal
			GLOBAL(isInInfiniteDesktopMode) = false;
			GLOBAL(DrawWalls) = true;

#ifdef TABLE			
			Key_SetCameraAsTopDown();
#else
			cam->loadCameraFromPreset(GLOBAL(settings.cameraPreset));
#endif
		}
	}
}
void Key_CreateNewDirectory()
{
	QString text = "";
	QString prompt = BtUtilStr->getString("NameFolder");

	while(true) {
		dlgManager->clearState();
		dlgManager->setCaption(prompt);
		dlgManager->setText(text);
		dlgManager->setTextSelection(0,0);
		dlgManager->setPrompt(prompt);

		if (dlgManager->promptDialog(DialogInput))
		{
			// ensure a valid name was given
			QString newName = dlgManager->getText();
			newName = fsManager->getUniqueNewFolderPathInWorkingDirectory(newName).dirName();

			if (isValidFilename(newName))
			{
				// Create a new directory
				QFileInfo newFile = make_file(GLOBAL(getWorkingDirectory()).absolutePath(), newName);
				QString newFilePath = native(newFile);
				CreateDirectory((LPCTSTR)(newFilePath.utf16()),NULL);
				break;
			}
			else
			{
				// notify the user of the error, and reprompt
				text = newName;
				prompt = QString(BtUtilStr->getString("NameFolderAgain"));
			}
		} else {
			return;
		}

	}

}

bool isValidFilename(QString newName) {
// Check if the input string is a valid windows filename
	QString invalidChars = "\\:/*?\"<>|";			// taken from windows explorer

	if(newName.size() == 0) {
		return false;
	}

	for (int i = 0; i < invalidChars.size(); ++i)
	{
		if (newName.contains(invalidChars[i]))
		{
			return false;
		}
	}
	return true;
}

void Key_ToggleSelectionText()
{
	vector<BumpObject *> selection = sel->getFreeItems();
	for (int i = 0; i < selection.size(); ++i) 
	{
		Actor * data = GetBumpActor(selection[i]);
		if (data->isTextHidden()) 
			data->showText();
		else
			data->hideText();
	}
}

void Key_TogglePausePhysics()
{	
	GLOBAL(gPause)=!GLOBAL(gPause);
}

void Key_ToggleCollisions()
{	
	vector<NxActorWrapper*> actors=GetDesktopItems();
	for(int i=0; i<actors.size(); i++)
		for(int j=0; j<i; j++) //iterate through every pair. once.  
		{
			if(NX_IGNORE_PAIR == GLOBAL(gScene)->getActorPairFlags(*(actors[i]->getActor()), *(actors[j]->getActor()))) //find current collision setting and flip it
				GLOBAL(gScene)->setActorPairFlags(*(actors[i]->getActor()), *(actors[j]->getActor()), 0);
			else
				GLOBAL(gScene)->setActorPairFlags(*(actors[i]->getActor()), *(actors[j]->getActor()), NX_IGNORE_PAIR);
		}
}

void Key_ToggleFullScreen()
{
	winOS->ToggleFullScreen();
}

//Load all supported images as Bump Objects.  Return num images loaded. 
int loadImageDirAsObjects(QString imgDirStr, int maxImgWidth, int maxImgHeight, int maxBumpObjDim, int bumpObjDepth, bool recursive, bool scaled)
{
	
	int imgsLoaded=0, Max = 0, Cntr = 0;
	NxActorWrapper *obj;
	vector<NxActorWrapper *> ActorList;
	float xOffset = 0.0, Size = 0.0;
	QDir imgDir(imgDirStr);

	StopwatchInSeconds imgTimer;
	StopwatchInSeconds allImgTimer;


	//use bilinear filter, for better shrinking of images
	//iluImageParameter(ILU_FILTER, ILU_BILINEAR);  //okay and fast
	//iluImageParameter(ILU_FILTER, ILU_SCALE_TRIANGLE); //better
	iluImageParameter(ILU_FILTER, ILU_SCALE_BELL); //best

	if (fsManager->getFileAttributes(imgDirStr))
	{
		cerr << "Directory does not exist: " << stdString(imgDirStr) << endl;
		return -1;
	}
	else
	{	//Count how many items in the directory
		//directory_iterator end_itr;
		StrList dirContents = fsManager->getDirectoryContents(imgDirStr);

		////for(directory_iterator itr(imgDir); itr != end_itr ; ++itr)
		//for (int i = 0; i < dirContents.size(); i++)
		//{
		//	QString fileName = dirContents[i];
		//	//if(is_directory(*itr) && recursive)
		//	//	loadImageDirAsObjects(itr->native_directory_string(), true); //recurse as necessary 
		//	//else 
		//	if(!(fsManager->getFileAttributes(dileName) & Directory))
		//	{
		//		//imgTimer.restart();
		//		//Check if Filetype is supported by DevIL
		//		QString ext=extension(*itr);
		//		if(ext.length()>1)
		//		{
		//			//Check extensions and load loadable images in directory
		//			to_lower(ext);
		//			if(supportedExtensions.find(ext + ".", 0) != string::npos)
		//			{
		//				// Count how many pictures we have
		//				Max++;
		//			}
		//		}
		//	}
		//}

		//for(directory_iterator itr(imgDir); itr != end_itr ; ++itr)
		for (int i = 0; i < dirContents.size(); i++)
		{
			QString fileName = dirContents[i];
			//if(is_directory(*itr) && recursive)
			//	loadImageDirAsObjects(itr->native_directory_string(), true); //recurse as necessary 
			//else 
			if(!(fsManager->getFileAttributes(fileName) & Directory))
			{
				imgTimer.restart();
				//Check if Filetype is supported by DevIL
				QString ext = fsManager->getFileExtension(fileName);

				if (ext.length()>1)
				{
					//Check extensions and load loadable images in directory
					if(GLOBAL(supportedExtensions).contains(ext + "."))
					{
						//ILinfo imgInfo;

						//////cout << "Extension: " << ext << " is supported\n";
						//ilLoadImage((ILstring)itr->native_file_string().c_str());
						////iluGetImageInfo(&imgInfo);
						//int imgWidth=150; //ilGetInteger(IL_IMAGE_WIDTH);
						//int imgHeight=150; //ilGetInteger(IL_IMAGE_HEIGHT);
						//iluScale(NxMath::min((int)imgWidth, (int)maxImgWidth), NxMath::min((int)imgHeight, (int)maxImgHeight), 1);

						//float aspectRatio = float(imgWidth) / float(imgHeight);
						//float x = 35 * (aspectRatio > 1 ? 1: aspectRatio);
						//float y = 35 * (aspectRatio < 1 ? 1: 1.0f/aspectRatio);
						//float sizeX = 512, sizeY = 512;
						//int texNum = createThumbnailFromFile(itr->native_file_string(), sizeX, sizeY);

						Vec3 Pos;

						//if (scaled)
						//{
						//	sizeX = sizeX * (-Cntr + 20) / 20;
						//	sizeY = sizeY * (-Cntr + 20) / 20;
						//	Pos = Vec3(0, cam->getEye().y, (Cntr * 20) - 90);
						//}else{
						//}
						Pos = Vec3(0, cam->getEye().y, 0);

						Vec3 dims(GLOBAL(settings).xDist, GLOBAL(settings).zDist, GLOBAL(settings).yDist);
						Vec3 Vel = Vec3(0.0f);

						//Vel.normalize(); Vel*=200;
						obj = CreateBumpIcon(Pos, dims , NULL, false, Vel);
						obj->setGlobalOrientation(GLOBAL(straightIconOri));
						GetFileSystemActor(obj)->setFilePath(fileName);
						//tex->setNewTexture(itr->native_file_string(), itr->native_file_string());
						GetFileSystemActor(obj)->enableThumbnail();

						// Add to the Pile
						if (!scaled)
						{
							ActorList.push_back(obj);
						}

						imgsLoaded++;
						////cout << itr->leaf() << " loaded in " << imgTimer.elapsed() << " seconds.\n";

					}
				}
			}
		}		
	}

	//Layout loaded images in a grid
	if (!scaled)
	{
		int gridSize = int(ceil(sqrt(float(ActorList.size())))); //Grid is gridSize x gridSize

		float GridSpacing = 0.8f;
		Vec3 dims = getDimensions(ActorList.front());
		float MaxVal = NxMath::max(dims.x, dims.y);
		float Xsize = MaxVal * 2 + MaxVal * GridSpacing;
		//float zDims = (GetActor(ActorList.front())->objNum == -1 ?  dims.y : dims.z); //use z for OBJs, and y for Icons, cause of inconsistent coordinate systems
		float Zsize = MaxVal * 2 + MaxVal * GridSpacing;
		Vec3 cent = Vec3(0,0,0);

		for (int i = 0; i < ActorList.size(); i++)
		{
			cent += ActorList[i]->getGlobalPosition();
		}

		cent /= float(ActorList.size());

		//Origin point, where to start laying out the grid
		Vec3 o = cent + Vec3(Xsize * float(gridSize + ((gridSize) * GridSpacing))/5.0f, 0, Zsize * float(gridSize + ((gridSize) * GridSpacing))/5.0f);
		o.y=100;

		for(int i = 0; i < int(ActorList.size()); i++)
		{
			Vec3 pos = o;
			pos += Vec3(-Xsize * (i % gridSize), 50, -Zsize * int(i / gridSize));
			//endPoses[ActorList[ActorList.size() - 1 - i]] = Mat34(Mat33(Quat(90, Vec3(1,0,0))), pos);

			obj = ActorList[ActorList.size() - 1 - i];
			obj->setGlobalPose(Mat34(Mat33(GLOBAL(straightIconOri)), pos));
			//obj->setGlobalOrientation(straightIconOri);
		}
	}


	//cout << "All Images loaded in " << allImgTimer.elapsed() << " seconds.\n\n";
	return imgsLoaded;
}

NxActorWrapper* LoadSingleFile(QString Inpt)
{
	
	FileSystemActor *obj;
	Vec3 dims, vel;
	vel = Vec3(0.0f);

	// Create the actor with proportionate size and the right texture
	obj =  FileSystemActorFactory::createFileSystemActor(Inpt);
	obj->setGlobalPosition(Vec3(0, cam->getEye().y, 0));
	obj->setGlobalOrientation(GLOBAL(straightIconOri));
	obj->setDims(Vec3(GLOBAL(settings).maxImageSize, GLOBAL(settings).maxImageSize, GLOBAL(settings).yDist));
	obj->setFilePath(Inpt);
	obj->enableThumbnail();

	return obj;
}

void PushActorAboveGround(NxActorWrapper* a, float extraPush, bool alwaysPush)
{
	//Push actor so its above ground plane by gets extents of its shape[0]
	NxShape** shapes=a->getShapes();
	NxBoxShape* box=shapes[0]->isBox();
	Box obb;
	box->getWorldOBB(obb);
	Vec3 mins=minAABBvalues(obb);

	if(mins.y < 0 || alwaysPush)
	{
		//printVec(a->getGlobalPosition());
		a->setGlobalPosition(a->getGlobalPosition() + Vec3(0,abs(mins.y)+extraPush,0));
	}
}





Vec3 pointOnFloor(int x, int y)
{
	NxPlane floor = NxPlane(Vec3(0,0,0), Vec3(0,1,0));
	Ray ray;
	Vec3 pt;
	NxF32 dist;

	// Project to the floor
	Vec3 start, end, dir;
	window2world(x, y, start, end);
	dir = end - start;
	dir.normalize();
	ray = Ray(start, dir);

	// Get the point on the floor
	NxRayPlaneIntersect(ray, floor, dist, pt);

	return pt;
}


void switchToOrtho()
{
#ifdef DXRENDER
	_ASSERT(0);
#else
	// H = Screen Height (In Pixels)
	// W = Screen Width (In Pixels)
	//
	// (0, H)               (W, H)
	//   +--------------------+
	//   |                    |
	//   |                    |
	//   |       SCREEN       |
	//   |                    |
	//   |                    |
	//   +--------------------+
	// (0, 0)               (W, 0)

	// Switch to Ortho Mode
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, winOS->GetWindowWidth(), 0, winOS->GetWindowHeight());
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
#endif
}

void switchToPerspective()
{
#ifdef DXRENDER
	_ASSERT(0);
#else
	// Switch back to Perspective mode
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, float(winOS->GetWindowWidth()) / float(winOS->GetWindowHeight()), GLOBAL(nearClippingPlane), GLOBAL(farClippingPlane));
	gluLookAt(cam->getEye().x, cam->getEye().y, cam->getEye().z, cam->getEye().x + cam->getDir().x, cam->getEye().y + cam->getDir().y, cam->getEye().z + cam->getDir().z, cam->getUp().x, cam->getUp().y, cam->getUp().z);
	glViewport(0, 0, winOS->GetWindowWidth(), winOS->GetWindowHeight());
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
#endif
}

void Key_Shrink()
{
	vector<BumpObject *> selection = sel->getBumpObjects();
	for (uint i = 0; i < selection.size(); i++)
	{
		selection[i]->shrink();
	}
	textManager->invalidate();
}

void Key_Grow()
{
	vector<BumpObject *> selection = sel->getBumpObjects();
	for (uint i = 0; i < selection.size(); i++)
	{
		selection[i]->grow();
	}
	textManager->invalidate();
}

void Key_ResetSize()
{
	vector<Pile *> pilesToUpdate;
	vector<BumpObject *> selection = sel->getBumpObjects();
	for (int i = 0; i < selection.size(); ++i)
	{
		selection[i]->finishAnimation();
		// queue up each of the pile items for resizing
		if (selection[i]->isBumpObjectType(BumpPile))
		{
			Pile * p = (Pile *) selection[i];
			vector<BumpObject *> items = p->getPileItems();
			selection.insert(selection.end(), items.begin(), items.end());
			animManager->addAnimation(AnimationEntry(p, (FinishedCallBack) UpdatePileDimsAfterAnim));
		}
		else
		{
			// update the size according to the aspect ratio
			Vec3 dims = selection[i]->getDims();
			const Vec3 & defaultDims = selection[i]->getDefaultDims();
			float aspect = dims.x / dims.y;
			float newWidth = (aspect > 1.0f) ? defaultDims.x: defaultDims.x * aspect;
			float newHeight = (aspect > 1.0f) ? defaultDims.y / aspect : defaultDims.y;
			selection[i]->setSizeAnim(dims, Vec3(newWidth, newHeight, defaultDims.z), 25);
		}
	}
	textManager->invalidate();
}

boost::function<void(Pile*)> _onMakePileHandler;


void setMakePileHandler( boost::function<void(Pile*)> onMakePileHandler )
{
	_onMakePileHandler = onMakePileHandler;
}

void Key_MakePile()
{
	Bounds bounds;
	vector<NxActorWrapper *> selection = sel->getLegacyActorList();
	vector<BumpObject *> objList = sel->getBumpObjects();
	Pile *vertStackPile;
	Vec3 cent;

	vertStackPile = new Pile();

	// Save this as a significant Action
	winOS->SetConditionalFlag(SignificantChange, true);

	Bounds actorBounds;
	for (int i = 0; i < objList.size(); i++)
	{
		NxActorWrapper *actor = objList[i];

		if (objList[i]->isPilable(SoftPile))
		{
			actorBounds = actor->getBoundingBox();
			bounds.include(actorBounds.getMin());
			bounds.include(actorBounds.getMax());
		}
	}

	bounds.getCenter(cent);

	// Animate a VertStack
	for (uint i = 0; i < objList.size(); i++)
	{
		if (objList[i]->isPilable(SoftPile))
		{
			vertStackPile->addToPile(objList[i]);
		}
	}

	vertStackPile->stack(cent);
	vertStackPile->sortBySize();

	if (!_onMakePileHandler.empty())
		_onMakePileHandler(vertStackPile);

	sel->clear();
	sel->add((BumpObject *) vertStackPile);
}

void Key_CreatePileFromCluster()
{
	vector<BumpObject *> objList = sel->getBumpObjects();

	// Save this as a significant Action
	winOS->SetConditionalFlag(SignificantChange, true);

	sel->clear();

	for (int i = 0; i < objList.size(); i++)
	{
		assert(objList[i]->isBumpObjectType(BumpCluster));
		Cluster *c = (Cluster*)objList[i];	
	
		QVector<BumpObject*> items = c->getItems();
		for (int i = 0; i < items.size(); i++)
		{
			sel->add(items[i]);
		}
		
	}

	Key_MakePile();
	
}


void Key_GridView()
{
	vector<Pile *> selection = sel->getFullPiles();

	for (uint i = 0; i < selection.size(); i++)
	{
		Pile *pile = (Pile *) selection[i];
		Vec3 pos = pile->getGlobalPosition();
		pos.y = repoManager->getPlaneFloorLevel() + PILE_SLATE_THICKNESS + PILE_SLATE_SPACE_ADJUSTMENT;

		pile->grid(pos);
		sel->clear();
	}
}

void Key_LeafForwardSelection()
{
	// if it's a pile, then invoke the leafer
	vector<BumpObject *> selection = sel->getBumpObjects();
	if (selection.size() == 1)
	{
		Pile * p = dynamic_cast<Pile *>(selection.front());
		if (!p) 
			p = dynamic_cast<Pile *>(selection.front()->getParent());
		if (p)
		{
			if (p->getPileState() == Stack)
			{
				p->leafDown();
				printTimedUnique("Key_LeafForwardSelection", 4, BtUtilStr->getString("ScrollWheelLeafPile"));
			}
			// this is not a typo, on the first transition, we leaf twice so that we can see the leaf
			// animation (not done for the first item on the stacked pile)
			if (p->getPileState() == Leaf)
			{
				p->leafDown();
			}
		}
	}
}

void Key_FanOut()
{	
	FinishModeBasedOnSelection();

	// explicitly show the pile items in case they were hidden before
	vector<Pile *> selection = sel->getFullPiles();
	if (!selection.empty())
	{
		Pile * pile = selection[0];
		pile->beginFanout();
		GLOBAL(mode) = FanoutOnStrokeMode;
	
		textManager->invalidate();
	}
}

void Key_SortByType()
{
	vector<Pile *> selection = sel->getFullPiles();

	// UNFINISHED:
	for (uint i = 0; i < selection.size(); i++)
	{
		Pile *pile = selection[i];
		pile->sortByType();
		pile->updatePileItems(true);
		if (pile->getPileState() == Grid)
		{
			pile->scrollGridRow(1);
			while (pile->scrollGridRow(-1)) {}
		}
	}
}

boost::function<void()> _onPileByTypeHandler;

void setPileByTypeHandler( boost::function<void()> onPileByTypeHandler )
{
	_onPileByTypeHandler = onPileByTypeHandler;
}

void gridLayoutPiles(QList<pair<QString, vector<FileSystemActor *>>> & container, const Bounds & selectionBounds)
{
	// selectionBounds is the area where the piles are to be laid, it's in world space coordinates	
	Vec3 selectionDims; selectionBounds.getDimensions(selectionDims);

	// calculate the dimensions/positions of the piles-to-be
	Vec3 padding(15.0f, 0.0f, 15.0f);
	Vec3 rowDims = padding; // dims of the current row including padding; start with padding
	vector<list<QString> > itemsInRows(1, list<QString>());
	vector<list<Vec3> > itemsInRowsPos(1, list<Vec3>()); // position of center of piles, relative to 0,0
	float zOffset = padding.z;
	unsigned int rowCount = 1;
	QList<pair<QString, vector<FileSystemActor *>>>::iterator iter = container.begin();
	float boxX = 0, boxZ = 0; // bounding box of the rows
	while (iter != container.end())
	{
		// for each pile, get the max dimensions of the actor's sizes
		Vec3 maxDims(0,0,0); // max dim of items in a pile; z is actually y in world
		for (int i = 0; i < iter->second.size(); ++i)
			maxDims.max((iter->second)[i]->getDims());

		if ((rowDims.x + 2.0f * maxDims.x + padding.x) > selectionDims.x)
		{
			// make a new row
			itemsInRows.push_back(list<QString>());
			itemsInRowsPos.push_back(list<Vec3>());
			zOffset += rowDims.z;
			rowDims = padding; // start new row with padding
		}
		itemsInRowsPos.back().push_back(Vec3(rowDims.x + maxDims.x, 0.0f, zOffset + maxDims.y));
		itemsInRows.back().push_back(iter->first);
		rowDims.x += (2.0f * maxDims.x) + padding.x;
		rowDims.z = NxMath::max(rowDims.z, (2.0f * maxDims.y) + padding.z);
		boxX = NxMath::max(boxX, rowDims.x);
		boxZ = NxMath::max(boxZ, zOffset + rowDims.z);
		iter++;
	}

	// actually position the piles in place
	sel->clear();
	Pile * tmpPile = NULL;
	vector<list<QString> >::iterator rowsItemsIter = itemsInRows.begin();
	vector<list<Vec3> >::iterator rowsItemsPosIter = itemsInRowsPos.begin();

	Vec3 offset((selectionDims.x - boxX) / 2, 0, 0);
	while ((rowsItemsIter != itemsInRows.end()) && (rowsItemsPosIter != itemsInRowsPos.end()))
	{		
		// fill out all piles in that row
		list<QString>::iterator itemsIter = (*rowsItemsIter).begin();
		list<Vec3>::iterator itemsPosIter = (*rowsItemsPosIter).begin();
		while ((itemsIter != (*rowsItemsIter).end()) && (itemsPosIter != (*rowsItemsPosIter).end()))
		{			
			const vector<FileSystemActor *> * items = NULL;
			for (uint i = 0; i < container.size(); i++)
			{
				if (container[i].first == *itemsIter)
					items = &container[i].second;
			}
			_ASSERT(items);
			Vec3 pos(selectionBounds.getMax() - *itemsPosIter - offset); // start from top left offset, go in decreasing x and z
			pos.y = 0;
			if (items->size() == 1)
			{
				// need special case for piles with only one item, since the pile will be automatically broken when stacked
				(items->back())->setGlobalPosition(pos);
				sel->add(items->back());
			}
			else
			{
				tmpPile = new Pile();
				for (int i = 0; i < items->size(); ++i)
					tmpPile->addToPile((*items)[i]);

				tmpPile->stack(pos);
				tmpPile->sortBySize();
				tmpPile->setGlobalPosition(pos);
				tmpPile->setText(BtUtilStr->tryGetString(*itemsIter));

				sel->add(tmpPile);
			}

			itemsIter++;
			itemsPosIter++;
		}

		rowsItemsIter++;
		rowsItemsPosIter++;
	}
}

QString sortIntoPilesKeyByType(const FileSystemActor * fsActor)
{
	if (fsActor->isFileSystemType(Link))
		return QString(QT_NT("Shortcuts"));
	else if (fsActor->isFileSystemType(Virtual))
		return QString(QT_NT("System Icons"));
	else if (fsActor->isFileSystemType(Folder))
		return QString(QT_NT("Folders"));
	else if (fsActor->isFileSystemType(Image) || fsActor->isFileSystemType(Thumbnail))
		return QString(QT_NT("Images"));
	else if (fsActor->isFileSystemType(Executable))
		return QString(QT_NT("Executables"));
	else if (fsActor->isFileSystemType(StickyNote))
		return QString(QT_NT("Sticky Notes"));
	else // just try and sort out each of the remaining items by their extension
		return QString("%1s").arg(winOS->GetFileTypeDescription(fsActor->getTargetPath()));
}

QString sortIntoPilesKeyByTime(const FileSystemActor * fsActor)
{
	int days = (QFileInfo(fsActor->getTargetPath()).lastModified().daysTo(QDateTime::currentDateTime()));
	// These are internal strings used for sorting the piles
	if (days < 1)
		return QString(QT_NT("00Today"));
	else if (days < 2)
		return QString(QT_NT("01Yesterday"));
	else if (days < 7)
		return QString(QT_NT("02Last Week"));
	else if (days < 31)
		return QString(QT_NT("03This Month"));
	else if (days < 365)
		return QString(QT_NT("04This Year"));
	else
		return QString(QT_NT("05A long time ago")); // in a galaxy far away.
}

void Key_SortIntoPiles(bool groupSingleItemsIntoMisc, bool setPileByTypeHandler, QString ( * sortIntoPilesKey) (const FileSystemActor * fsActor))
{
	Bounds selectionBounds;
	selectionBounds.setCenterExtents(GetDesktopBox().center, GetDesktopBox().extents);
	
	// get all the items in the selection (skip piles? or break soft piles only?)
	bool explodeSoftPiles = false;
	vector<BumpObject *> selected;
	if (sel->getSize() > 0)
	{
		selected = sel->getBumpObjects();
		explodeSoftPiles = true;
	}
	else
	{
		dlgManager->clearState();
		dlgManager->setPrompt(BtUtilStr->getString("AutoPilePrompt"));
		if (!dlgManager->promptDialog(DialogYesNo))
			return;

		if (setPileByTypeHandler)
			if (!_onPileByTypeHandler.empty())
				_onPileByTypeHandler(); // Used for tutorial to progress.

		selected = vector<BumpObject *>(scnManager->getBumpObjects());		
	}
		
	// sort each of the items into their respective container
	typedef QHash<QString, vector<FileSystemActor *> > Container;
	Container container;
	int numValidItems = 0;
	for (int i = 0; i < selected.size(); ++i)
	{
		BumpObject * obj = selected[i];
		ObjectType objType = obj->getObjectType();
		if (objType == ObjectType(BumpActor, FileSystem))
		{
			FileSystemActor * fsActor = (FileSystemActor *) obj;
			if (fsActor->isFileSystemType(PhotoFrame))
				continue;	// skip photo frames
			else if (winOS->GetIconTypeFromFileName(fsActor->getFullPath()) == RecycleBin)
				continue; // skip Recycle Bin
			else if (fsActor->getParent() && fsActor->getParent()->isObjectType(ObjectType(BumpPile, HardPile)))
				continue;	// skip items in a hard pile (folder)
			else if (fsActor->isPinned())
				continue;	// skip pinned items
			
			container[sortIntoPilesKey(fsActor)].push_back(fsActor);

			++numValidItems;
		}
		else if (objType == ObjectType(BumpPile, SoftPile))
		{
			// break the soft pile, items in a soft pile will be sorted when encountered
			Pile * pile = dynamic_cast<Pile *>(obj);

			if (explodeSoftPiles) 
			{
				// in certain cases, only the pile will be selected (not all the items in it)
				// so we need to explode the soft pile and add their items
				vector<BumpObject *> pileItems = pile->getPileItems();
				for (int j = 0; j < pileItems.size(); ++j) {
					selected.push_back(pileItems[j]);
				}
			}

			pile->breakPile();
			animManager->finishAnimation(pile);
		}
	}
	
	// ensure that there's a valid number of items
	if (numValidItems == 0)
	{		
		MessageClearPolicy clearPolicy;
		clearPolicy.setTimeout(5);
		scnManager->messages()->addMessage(new Message("Key_SortIntoPiles", BtUtilStr->getString("ErrorAutoPileNoItems"), Message::Ok, clearPolicy));
		return;
	}
	else if (numValidItems == 1)
	{
		// select the item and notify the user
		sel->clear();
		sel->add(selected.front());
		MessageClearPolicy clearPolicy;
		clearPolicy.setTimeout(5);
		scnManager->messages()->addMessage(new Message("Key_SortIntoPiles", BtUtilStr->getString("ErrorAutoPileSingleItem"), Message::Ok, clearPolicy));
		return;
	}


	// ensure valid number of items
	if (container.empty())
		return;

	if (groupSingleItemsIntoMisc)
	{
		// put all single-item categories into the __Misc__ category
		Container::iterator iter = container.begin();
		vector<FileSystemActor *> misc;
		while (iter != container.end())
		{
			if (iter.value().size() == 1)
			{
				misc.push_back(iter.value().front());
				container.erase(iter++);
			}
			else
				iter++;
		}
		if (!misc.empty())
			container["Misc Item"] = misc;
	}

	QList<QString> keys = container.uniqueKeys();
	qSort(keys.begin(), keys.end());
	QList<pair<QString, vector<FileSystemActor *>>> piles;
	for (uint i = 0; i < keys.size(); i++)
		piles.push_back(make_pair(keys[i], container[keys[i]]));

	gridLayoutPiles(piles, selectionBounds);

	// notify the user
	QString userNotification = BtUtilStr->getString("SortIntoPileMessage").arg(selected.size()).arg(container.size());
	printTimedUnique("Key_SortIntoPiles", 5, userNotification);	
}

void Key_SortIntoPilesByType()
{
	statsManager->getStats().bt.interaction.piles.pileByType++; // record in stats
	Key_SortIntoPiles(true, true, &sortIntoPilesKeyByType);
}

void Key_SortIntoPilesByTime()
{ 
	Key_SortIntoPiles(false, false, &sortIntoPilesKeyByTime); 
}

void Key_SortBySize()
{
	vector<Pile *> selection = sel->getFullPiles();

	// UNFINISHED:
	for (uint i = 0; i < selection.size(); i++)
	{
		Pile *pile = selection[i];
		pile->sortBySize();
		pile->updatePileItems(true);
		if (pile->getPileState() == Grid)
		{
			pile->scrollGridRow(1);
			while (pile->scrollGridRow(-1)) {}
		}
	}
}

void Key_SortByName()
{
	vector<Pile *> selection = sel->getFullPiles();

	// UNFINISHED:
	for (uint i = 0; i < selection.size(); i++)
	{
		Pile *pile = selection[i];
		pile->sortByName();
		pile->updatePileItems(true);
		if (pile->getPileState() == Grid)
		{
			pile->scrollGridRow(1);
			while (pile->scrollGridRow(-1)) {}
		}
	}
}

void Key_Folderize()
{
	reauthorize(false);

	vector<Pile *> selection = sel->getFullPiles();

	for (uint i = 0; i < selection.size(); i++)
	{
		Pile *pile = (Pile *) selection[i];

		if (selection[i]->getPileType() == HardPile)
		{
			FileSystemPile *fsPile = (FileSystemPile *) pile;
			fsPile->folderize();
		}
		else if (selection[i]->getPileType() == SoftPile)
		{
			SoftPileToFolderIcon(pile);
		}
	}

	selection.clear();
}

void Key_BreakPile()
{
	LOG_LINE_REACHED();

	// try and finish the mode before breaking the pile
	FinishModeBasedOnSelection();

	vector<Pile *> selection = sel->getFullPiles();

	for (uint i = 0; i < selection.size(); i++)
	{
		Pile *pile = selection[i];

		// Restore the Messy Poses
		FileSystemPile * fsPile = dynamic_cast<FileSystemPile *>(pile);
		if (!fsPile || !fsPile->getOwner()->isFileSystemType(Removable))
			pile->breakPile();
	}
}

void Key_ClosePile()
{
	vector<BumpObject *> selection = sel->getBumpObjects();
	if (selection.size() == 1)
	{
		BumpObject * obj = selection.front();
		if (obj->getParent())
			obj = obj->getParent();
		if (obj->getObjectType() == ObjectType(BumpPile))
		{
			Pile * p = (Pile *) obj;
			if (p->getPileState() != Stack)
			{
				p->close();
			}
		}
	}
	else
	{
		if (!markingMenu->isEnabled())
			sel->clear();
	}
}

void Key_RemoveFromPile()
{
	vector<BumpObject *> objList = sel->getPileMembers();
	vector<NxActorWrapper *> deleteList;
	Actor *data = NULL;
	Pile *pile;
	NxActorWrapper *actor;
	Vec3 v, w;
	set<Pile *> closePiles;

	// For each object, remove it from the pile and move it to the desktop
	for (uint i = 0; i < objList.size(); i++)
	{
		data = (Actor *) objList[i];
		actor = data;
		pile = data->getPileParent();

		// If the item is in a slideshow, end the slideshow
		if(objList[i] == cam->getHighlightedWatchedActor()) {
			if(isSlideshowModeActive()) {
				Key_ToggleSlideShow();
			}
		}

		// Item is Inside a Pile
		if (pile)
		{
			// SoftPile - Pull it out from the pile
			pile->removeFromPile(GetBumpObject(actor), NoUpdate);
			closePiles.insert(pile);
			data->killAnimation();
		}

		ZeroAllActorMotion(actor, false);
		actor->setLinearVelocity(Vec3(0, -1, 0));

		// Clean up on the Piles
		if (pile && pile->getNumItems() == 0)
		{
			if (pile->getPileType() == HardPile)
			{
				FileSystemPile *fsPile = (FileSystemPile *) pile;
				fsPile->folderize();
			}
		}
		else
		{
			// Pick the closest Item in the selection
			vector<NxActorWrapper *> legacyActors = sel->getLegacyActorList();
			if (!legacyActors.empty())
			{
				Vec3 itemCentroid = WorldToClient(legacyActors.front()->getGlobalPosition(), false, true);
				pickAndSet( int(itemCentroid.x), int(itemCentroid.y) );
				closePiles.insert(pile);
				lassoMenu->reset();
			}
		}
	}

	set<Pile *>::const_iterator iter = closePiles.begin();
	while (iter != closePiles.end())
	{
		(*iter)->close();
		iter++;
	}


	sel->clear();
}

// Converts a pile in any state into a Vertical Stack
void Key_StackPile()
{
	vector<BumpObject *> selList = sel->getBumpObjects();

	for (uint i = 0; i < selList.size(); i++)
	{
		Pile *pile = (Pile *) selList[i];

		// Pileize the Folder
		pile->stack(pile->getGlobalPosition());
	}

}

void Key_ShowAbout()
{
	QString about;
	QTextStream stream(&about);
	stream << "BumpTop";
	if (GLOBAL(settings).freeOrProLevel == AL_PRO)
		stream << " Pro";
	
	stream << " v." << winOS->GetBuildNumber();
#ifdef DXRENDER
	stream << " DXR";
#endif
#ifdef _DEBUG
	stream << " (Debug-";
#else
	stream << " (Release-";
#endif
	stream << winOS->GetLocaleLanguage() << ")";
	if (winOS->GetBumpTopEdition() != Standard)
		stream << " - " << winOS->BumpTopEditionName(winOS->GetBumpTopEdition());
#ifdef TABLE
	stream << " - Table";
#endif
	stream << "\n" << QString::fromStdString(statsManager->getStats().hardware.vidRenderer);
	reauthorize(false);

	scnManager->messages()->addMessage(new Message("Key_ShowAbout", about, Message::Ok));
}

void Key_ToggleDebugKeys()
{
	
	GLOBAL(settings).enableDebugKeys = !GLOBAL(settings).enableDebugKeys;
	keyManager->init();

	// notify the user of the change in debug keys
	QString message;
	if (GLOBAL(settings).enableDebugKeys)
		message = QT_TRANSLATE_NOOP("BtUtilStrings", "Enabling Debug Keys");
	else
		message = QT_TRANSLATE_NOOP("BtUtilStrings", "Disabling Debug Keys");

	MessageClearPolicy clearPolicy;
		clearPolicy.setTimeout(2);
	scnManager->messages()->addMessage(new Message("Key_ToggleDebugKeys", message, Message::Ok, clearPolicy));
}

void Key_ToggleBubbleClusters()
{
	bubbleManager->cycleMode();
}

void Key_ToggleRepositionBounds()
{
	GLOBAL(settings).repositionIconsIfOutsideWalls = !GLOBAL(settings).repositionIconsIfOutsideWalls;

	QString msg = GLOBAL(settings).repositionIconsIfOutsideWalls ? QT_TRANSLATE_NOOP("BtUtilStrings", "Repositions On") : QT_TRANSLATE_NOOP("BtUtilStrings", "Repositions Off");
	scnManager->messages()->addMessage(new Message("Repositions", msg, Message::Ok));	
}

void Key_ExpandToPile()
{
	reauthorize(false);

	vector<BumpObject *> selList = sel->getBumpObjects();

	for (uint i = 0; i < selList.size(); i++)
	{
		if (selList[i]->isBumpObjectType(BumpActor))
		{
			Actor *data = (Actor *) selList[i];

			if (data->isActorType(FileSystem))
			{
				FileSystemActor *fsData = (FileSystemActor *) data;

				if (fsData->isFileSystemType(Folder))
				{
					// Pileize the Folder
					fsData->pileize();
				}
			}
		}
	}
}

void Key_PileizeAndGridView()
{	
	reauthorize(false);

	animManager->removeAllAnimations();
	vector<BumpObject *> selList = sel->getBumpObjects();

	for (uint i = 0; i < selList.size(); i++)
	{
		if (selList[i]->isBumpObjectType(BumpActor))
		{
			Actor *data = (Actor *) selList[i];

			if (data->isActorType(FileSystem))
			{
				FileSystemActor *fsData = (FileSystemActor *) data;

				if (fsData->isFileSystemType(Folder))
				{
					// Pileize the Folder
					FileSystemPile *pile = (FileSystemPile *) fsData->pileize();

					if (pile)
					{
						Vec3 pos = pile->getGlobalPosition();
						pos.y = repoManager->getPlaneFloorLevel() + PILE_SLATE_THICKNESS + PILE_SLATE_SPACE_ADJUSTMENT;

						animManager->finishAnimation(pile);
						if (scnManager->containsObject(pile))
						{
							pile->grid(pos);
							repoManager->update();
						}
					}
				}
			}
		}
	}

	textManager->invalidate();
}

void Key_MoreOptionsHelper(POINT p, bool enableTracking)
{
	ContextMenu menu;
	if (menu.hasActiveMenu())
		return;

	if (!_onInvokeCommandHandler.empty())
		menu.setOnInvokeCommandHandler(_onInvokeCommandHandler);

	// we simply invoke Key_MoreOptions from the marking menu, since we have "prepared"
	// (created the additional context menu items, among other things) the menu already
	//
	// however, from the keyboard, we need to make sure to call prepare
	// this mechanism is a bit clunky and should probably be refactored
	if (markingMenu->getLowerLevelActions().empty())
		markingMenu->prepare();

	vector<MenuAction *> lowerActions = markingMenu->getLowerLevelActions();
	menu.setTrackingEnabled(enableTracking);
	menu.showMenu(lowerActions, p, true, TPM_TOPALIGN | TPM_CENTERALIGN);
}

void Key_MoreOptions()
{
	POINT p;
	if(!markingMenu->isEnabled())
	{
		GetCursorPos(&p);
	}
	else
	{
		p.x = markingMenu->getCentroid().x;
		p.y = markingMenu->getCentroid().y + markingMenu->getRadius();
		
		RECT rect = {0}; // Convert window coord to screen coord
		if (GetWindowRect(winOS->GetWindowsHandle(), &rect))
		{
			p.x += rect.left;
			p.y += rect.top;
		}
	}

	Key_MoreOptionsHelper(p, false);
}

void Key_ResetBumpTopLayout()
{
	dlgManager->clearState();
	dlgManager->setPrompt(BtUtilStr->getString("ResetLayoutPrompt"));
	if (dlgManager->promptDialog(DialogYesNo))
	{
		// record this reset layout
		statsManager->getStats().bt.resetLayout++;

		// delete the scene bumpfile and reload it
		ClearBumpTop();
		CreateBumpObjectsFromWorkingDirectory();
		cam->zoomToAngledView();
		
		// Add postIntroLoad to sync all the postIt notes and initialize the photo frames
		animManager->addQueuedAnimation(AnimationEntry(cam, (FinishedCallBack) PostIntroLoad, NULL, false));
	}
}

void Key_ResetBumpTopSettings()
{
	// Retrieve GUID
	QString guid(GLOBAL(settings).guid);

	// Retrieve Pro Key
	QString proKey(GLOBAL(settings).proAuthCode);
	QString proInviteCode(GLOBAL(settings).proInviteCode);

	// Retrieve Invite code
	QString inviteCode(GLOBAL(settings).inviteCode);

	// Retrieve Facebook information
	QString fb_secret(GLOBAL(settings).fbc_secret);
	QString fb_session(GLOBAL(settings).fbc_session);
	QString fb_uid = GLOBAL(settings).fbc_uid;

	// Retrieve Twitter information
	QString twitterLogin(GLOBAL(settings).tw_login);
	QString twitterOAuthKey(GLOBAL(settings).tw_oauth_key);
	QString twitterOAuthSecret(GLOBAL(settings).tw_oauth_secret);
	QString twitterPassword(GLOBAL(settings).tw_password);

	// Backup old settings file
	QFileInfo settingsPath = make_file(winOS->GetDataDirectory(), "settings.json");
	vector<QString> settingJsonFiles = fsManager->getDirectoryContents(winOS->GetDataDirectory().absolutePath(), "settings.*");

	// Try renaming the settings file
	QString newFileName = settingsPath.absoluteFilePath() + QString::number(settingJsonFiles.size()) + ".bak";
 	if (!QFile::rename(settingsPath.absoluteFilePath(), newFileName))
 	{
		// Since the rename failed that means the Settings json file already exists. So instead we will
		// try renaming the settings json file to "settings.jsonX.bak" where "X" is between 1 and the number of files/folders 
		// inside the data directory
		for (int i = 0;i<settingJsonFiles.size();i++)
		{
			newFileName = settingsPath.absoluteFilePath() + QString::number(i) + ".bak";
			if (QFile::rename(settingsPath.absoluteFilePath(), newFileName))
				break;
		}
	}

	// Create/Load new settings file
	winOS->LoadSettingsFile();

	// Sync backed up values
	GLOBAL(settings).guid = guid;
	GLOBAL(settings).proAuthCode = proKey;
	GLOBAL(settings).proInviteCode = proInviteCode;
	GLOBAL(settings).inviteCode = inviteCode;
	GLOBAL(settings).fbc_secret = fb_secret;
	GLOBAL(settings).fbc_session = fb_session;
	GLOBAL(settings).fbc_uid = fb_uid;
	GLOBAL(settings).tw_login = twitterLogin;
	GLOBAL(settings).tw_oauth_key = twitterOAuthKey;
	GLOBAL(settings).tw_oauth_secret = twitterOAuthSecret;
	GLOBAL(settings).tw_password = twitterPassword;

	// Save settings file
 	winOS->SaveSettingsFile();

}

void Key_DeleteSceneFiles()
{
	dlgManager->clearState();
	dlgManager->setPrompt(BtUtilStr->getString("ResetLayoutPrompt"));

	if (dlgManager->promptDialog(DialogYesNo))
	{
		// Delete each scene.* file
		GLOBAL(saveSceneMutex).lock();
		QDir dataDirectory = winOS->GetDataDirectory();
		fsManager->deleteFileByName(dataDirectory.absoluteFilePath("scene.bump.bak"), true);
		fsManager->deleteFileByName(dataDirectory.absoluteFilePath("scene.bump"), true);
		fsManager->deleteFileByName(dataDirectory.absoluteFilePath("scene.pb.bump.bak"), true);
		fsManager->deleteFileByName(dataDirectory.absoluteFilePath("scene.pb.bump"), true);
		GLOBAL(saveSceneMutex).unlock();
	}
}

void Key_SendFeedback()
{
#if DISABLE_PHONING
	dlgManager->clearState();
	dlgManager->setPrompt(QT_TRANSLATE_NOOP("BtUtilStrings", "The feedback system is currently disabled."));
	dlgManager->promptDialog(DialogOK);
#else
	// try and use the email address they entered last time if they have one
	QString email = GLOBAL(settings).userEmail;
	dlgManager->clearState();
	dlgManager->setEmail(email);

	while (dlgManager->promptDialog(DialogFeedback))
	{
		QString text = dlgManager->getText(), 
			   email = dlgManager->getEmail(),
			   type = dlgManager->getType();
		QString subject = QString("[BumpTop Feedback][%1] - %2").arg(type).arg(winOS->GetGUID());
		dlgManager->clearState();
		dlgManager->setCaption(QT_TRANSLATE_NOOP("BtUtilStrings", "BumpTop Reporter"));

		// save the email address for the future if one is provided and is different than
		// the currently saved one.
		if (!email.isEmpty() && GLOBAL(settings).userEmail != email)
		{
			GLOBAL(settings).userEmail = email;	
			winOS->SaveSettingsFile();
		}
		
		if (EmailCrashReport(QT_NT("feedback@bumptop.com"), subject, text, email, QString()))
		{
			dlgManager->setPrompt(BtUtilStr->getString("FeedbackSent"));
			dlgManager->promptDialog(DialogOK);
			break;
		}else{
			dlgManager->clearState();
			dlgManager->setPrompt(BtUtilStr->getString("ErrorFeedback"));
			dlgManager->promptDialog(DialogOK);

			// bring up the old dialog so user can copy their text
			dlgManager->clearState();
			dlgManager->setEmail(email);
			dlgManager->setText(text);
		}
	}
#endif
}

void Key_ShowSettingsOpenThemesTab()
{
	QString paramsStr;
	paramsStr.append(QString(" -bumptopHWND %1").arg((unsigned int) winOS->GetWindowsHandle()));
	if (!GLOBAL(settings).inviteCode.isEmpty())
		paramsStr.append(QString(" -inviteCode %1").arg(GLOBAL(settings).inviteCode));
	if (winOS->IsKeyDown(KeyShift))
		paramsStr.append(" -admin");
	paramsStr.append(QString(" -proUrl %1").arg(getBumpTopProPageUrl("settingsComparisonChart")));
	paramsStr.append(QString(" -language %1").arg(winOS->GetLocaleLanguage()));
	paramsStr.append(QString(" -themesTab"));

	Key_ShowSettingsDialogWindow(paramsStr);
}

void Key_ThemeDialog()
{
	CustomizeWizard *themeWiz = new CustomizeWizard(CustomizeWizard::THEME);
	QString text = themeWiz->exec();

}

void Key_CreateNewPhotoFrame()
{
	// force a render so we don't get a black screen?
	rndrManager->invalidateRenderer();

	// get the photo frame dialog and reset it to default instance
	PhotoFrameDialog * pfDialog = (PhotoFrameDialog *) dlgManager->getComplexDialog(DialogPhotoFrame);
	pfDialog->resetToDefault();
	pfDialog->setRawFeed("http://api.flickr.com/services/feeds/photos_faves.gne?nsid=26799028@N06&lang=en-us&format=rss_200");

	// prompt the user as to which kind of photo frame they want
	// create a new photo frame
	if (dlgManager->promptDialog(DialogPhotoFrame))
	{
		// retrieve the type of frame to create and associated information
		PhotoFrameDialog::PhotoFrameSourceSelectedType type = pfDialog->getSelectedType();
		PhotoFrameActor * pfActor = CreatePhotoFrameHelper(pfDialog->getSourceString());

		if (!pfActor)
			return;

		if (scnManager->isInInfiniteDesktopMode)
		{
			Quat rightSideUp(90, Vec3(1,0,0));
			pfActor->setGlobalOrientation(rightSideUp);
		}
		else
		{
			// position the frame and pin it on the back wall at a random (free?) location
			Box desktopBox = GetDesktopBox();
			srand((unsigned int) time(NULL));
			int randXOffset = (rand() % 30) - 15; 
			Vec3 pfActorPos(desktopBox.center.x + randXOffset, getHeightForResetLayout(&desktopBox), desktopBox.center.z + desktopBox.extents.z);
			pfActor->setGlobalPosition(pfActorPos);
			pfActor->onPin();
		}
	}
}

void Key_RunBumptopTests()
{
#ifdef BT_UTEST
	// run bumptop tests
	runBumptopTests();
#else
	// show a message otherwise
	MessageClearPolicy clearPolicy;
		clearPolicy.setTimeout(4);
	Message * message = new Message("Key_RunBumptopTests", BtUtilStr->getString("ErrorTests"), Message::Ok, clearPolicy);
	scnManager->messages()->addMessage(message);
#endif // BT_UTEST
}

StrList GetFilteredSetForIDataObject(vector<BumpObject *>& selection, IShellFolder2 ** psfOut)
{
	QDir userDesktop(winOS->GetSystemPath(DesktopDirectory));
	QDir allUsersDesktop(winOS->GetSystemPath(AllUsersDesktopDir));

	bool hasPrevSourceDir = false; 
	QDir prevSourceDir;
	StrList filePaths;
	for (int i = 0; i < selection.size();)
	{
		ObjectType objType = selection[i]->getObjectType();
		if (objType == ObjectType(BumpPile, HardPile))
		{
			selection.erase(selection.begin() + i);
			// don't increment i, since we've erased the item at i
		}
		else if (objType == ObjectType(BumpPile, SoftPile))
		{
			Pile * p = dynamic_cast<Pile *>(selection[i]);
			selection.erase(selection.begin() + i);
			// don't increment i, since we've erased the item at i

			vector<BumpObject *> pileItems = p->getPileItems();
			for (vector<BumpObject *>::iterator pIter = pileItems.begin(); 
				pIter != pileItems.end(); pIter++)
			{
				// NOTE: this is not a typo, we are pushing back to the end of the 
				// selection, so that it is handled recursively if necessary
				selection.push_back(*pIter);
			}
		}
		else if ((objType == ObjectType(BumpActor, FileSystem)) && 
				!(objType == ObjectType(BumpActor, FileSystem, Virtual)))
		{
			QString filePath(dynamic_cast<FileSystemActor *>(selection[i])->getFullPath());
			QDir filePathBranch = parent(filePath);
			filePaths.push_back(filePath);
			if (hasPrevSourceDir && (filePathBranch != prevSourceDir))
			{
				// ERROR, we've found a mixed selection (items from the desktop and a subfolder)
				// notify the user and return

				// NOTE: it will still fail for instances where the desktops are not the same
				// if (!( (filePathBranch == userDesktop || filePathBranch == allUsersDesktop) &&
				// 	   (prevSourceDir == userDesktop || prevSourceDir == allUsersDesktop) ))
				{
					MessageClearPolicy clearPolicy;
					clearPolicy.setTimeout(4);
					Message * message = new Message("GetFilteredSetForIDataObject", 
						BtUtilStr->getString("ErrorMultipleSources"), Message::Warning, clearPolicy);
					scnManager->messages()->addMessage(message);
					selection.clear();
					return StrList();
				}
			}
			prevSourceDir = filePathBranch;
			hasPrevSourceDir = true;
			++i;
		}
		else
		{
			selection.erase(selection.begin() + i);
			// don't increment i, since we've erased the item at i
		}
	}
	*psfOut = winOS->GetShellFolderFromAbsDirPath(native(prevSourceDir));

	if (filePaths.empty())
	{
		MessageClearPolicy clearPolicy;
			clearPolicy.setTimeout(3);
		scnManager->messages()->addMessage(new Message("Key_CopySelection", BtUtilStr->getString("NoItemsSelected"), Message::Ok, clearPolicy));
	}

	assert(filePaths.size() == selection.size());
	return filePaths;
}

void Key_CopySelection()
{
	// XXX: TODO TAKE CARE OF HARD PILES

	LOG_LINE_REACHED();

	IShellFolder * desktopFolder = NULL;
	IShellFolder2 * psf = NULL;
	IShellView * psv = NULL;

	// get the shell folder
	if (SUCCEEDED(SHGetDesktopFolder(&desktopFolder))) 
	{
		{
			// create a list of all the items to be copied
			vector<BumpObject *> selection = sel->getBumpObjects();
			StrList filePaths = GetFilteredSetForIDataObject(selection, &psf);
			if (filePaths.empty())
				return;

			// reselect the proper selection
			QList<BumpObject *> newSelection;
			for (int i = 0; i < selection.size(); ++i)
			{
				selection[i]->setAlphaAnim(selection[i]->getAlpha(), 1.0f, 10);
				selection[i]->setFreshnessAlphaAnim(1.0f, 20);
				newSelection.append(selection[i]);
			}
			sel->replace(newSelection);

			// build the set of pidls that are to be acted upon
			uint numPidls = filePaths.size();
			LPCITEMIDLIST * pidlArray = new LPCITEMIDLIST[numPidls];
			for (int i = 0; i < numPidls; ++i)
			{
				pidlArray[i] = winOS->GetRelativePidlFromAbsFilePath(filePaths[i]);
			}

			// get the IDataObject representing these files
			IDataObject * pdo = NULL;
			HRESULT hr = psf->GetUIObjectOf(NULL, numPidls, pidlArray, IID_IDataObject, NULL, (void **) &pdo);

			// set this data object to be a copy
			FORMATETC typeFmt = {RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
			STGMEDIUM typeStgm = {0};
			typeStgm.tymed = TYMED_HGLOBAL;
			typeStgm.hGlobal = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(DWORD));
			*((DWORD*)GlobalLock(typeStgm.hGlobal)) = DROPEFFECT_COPY;
			GlobalUnlock(typeStgm.hGlobal);
			pdo->SetData(&typeFmt, &typeStgm, TRUE);

			// set the clipboard with these files
			OleSetClipboard(pdo);

			psf->Release();

			// display notification
			MessageClearPolicy clearPolicy;
				clearPolicy.setTimeout(3);
			QString osstr = QString(BtUtilStr->getString("FilesCopied")).arg(filePaths.size());
			scnManager->messages()->addMessage(new Message("Key_Cut/Copy", osstr, Message::Ok, clearPolicy));

			// note the action in the stats
			statsManager->getStats().bt.interaction.clipboard.copy++;
		}
		desktopFolder->Release();
	}
}

void Key_CutSelection()
{
	IShellFolder * desktopFolder = NULL;
	IShellFolder2 * psf = NULL;
	IShellView * psv = NULL;

	LOG_LINE_REACHED();

	// get the shell folder
	if (SUCCEEDED(SHGetDesktopFolder(&desktopFolder))) {
		{
			// create a list of all the items to be cut
			vector<BumpObject *> selection = sel->getBumpObjects();
			StrList filePaths = GetFilteredSetForIDataObject(selection, &psf);
			if (filePaths.empty())
				return;

			// dim each of the actors and select the proper selection
			QList<BumpObject *> newSelection;
			for (int i = 0; i < selection.size(); ++i)
			{
				selection[i]->setAlphaAnim(selection[i]->getAlpha(), 0.6f, 10);
				selection[i]->setFreshnessAlphaAnim(1.0f, 20);
				newSelection.append(selection[i]);
			}
			sel->replace(newSelection);

			// build the set of pidls that are to be acted upon
			uint numPidls = filePaths.size();
			LPCITEMIDLIST * pidlArray = new LPCITEMIDLIST[numPidls];
			for (int i = 0; i < numPidls; ++i)
			{
				pidlArray[i] = winOS->GetRelativePidlFromAbsFilePath(filePaths[i]);
			}

			// get the IDataObject representing these files
			IDataObject * pdo = NULL;
			HRESULT hr = psf->GetUIObjectOf(NULL, numPidls, pidlArray, IID_IDataObject, NULL, (void **) &pdo);

			// set this data object to be a copy
			FORMATETC typeFmt = {RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
			STGMEDIUM typeStgm = {0};
				typeStgm.tymed = TYMED_HGLOBAL;
				typeStgm.hGlobal = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(DWORD));
			*((DWORD*)GlobalLock(typeStgm.hGlobal)) = DROPEFFECT_MOVE;
			GlobalUnlock(typeStgm.hGlobal);
			pdo->SetData(&typeFmt, &typeStgm, TRUE);

			// set the clipboard with these files
			OleSetClipboard(pdo);

			psf->Release();

			// display notification
			MessageClearPolicy clearPolicy;
				clearPolicy.setTimeout(3);
			QString osstr = QString(BtUtilStr->getString("FilesCut")).arg(filePaths.size());
			scnManager->messages()->addMessage(new Message("Key_Cut/Copy", osstr, Message::Ok, clearPolicy));

			// note the action in the stats
			statsManager->getStats().bt.interaction.clipboard.cut++;
		}
		desktopFolder->Release();
	}
}

void Key_PasteSelection()
{	
	bool isCopy = true;
	bool succeeded = false;
	StrList filePaths;
	IDataObject * pdo = NULL;

	LOG_LINE_REACHED();

	if (SUCCEEDED(OleGetClipboard(&pdo)))
	{
		// get the type of the paste operation (whether we are copying or moving)
		FORMATETC typeFmt = {RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		STGMEDIUM typeStgm = {0};
		if (SUCCEEDED(pdo->GetData(&typeFmt, &typeStgm)))
		{
			DWORD * typeDword = (DWORD *) GlobalLock(typeStgm.hGlobal);
			if (*typeDword == DROPEFFECT_MOVE)
				isCopy = false;
			GlobalUnlock(typeStgm.hGlobal);
			ReleaseStgMedium(&typeStgm);
		}

		// get the list of files to paste
		FORMATETC fmt = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		STGMEDIUM stgm = {0};
		if (SUCCEEDED(pdo->GetData(&fmt, &stgm)))
		{
			UINT numFiles = DragQueryFile((HDROP) stgm.hGlobal, -1, NULL, 0);
			TCHAR path[MAX_PATH];
			for (int i = 0; i < numFiles; ++i)
			{
				DragQueryFile((HDROP) stgm.hGlobal, i, path, MAX_PATH);
				QString filePath = QString::fromUtf16((const ushort *) path);
				if (exists(filePath))
					filePaths.push_back(filePath);
			}
			ReleaseStgMedium(&stgm);
		}

		// notify the users of the number of files we got
		winOS->SetLastDropPointFromDrag(filePaths.size());

		// move the files to the desktop and let explorer know
		if (!filePaths.empty())
		{
			QString destDir = native(scnManager->getWorkingDirectory());
			QDir destDirPath(destDir);

			if (isCopy)
			{
				// NOTE: I have no idea why FOF_RENAMEONCOLLISION does not work?

				// copy each file individually, making new filenames for each file collision
				succeeded = true;
				for (int i = 0; i < filePaths.size(); ++i)
				{
					QString fileName = filename(filePaths[i]);
					int dupeCount = 0;
					while (exists(make_file(destDirPath, fileName)))
					{
						// XXX: LOCALIZATION
						QString tmpStr;
						if (dupeCount < 1)
							tmpStr = QString(BtUtilStr->getString("CopyOfFile")).arg(filename(filePaths[i]));
						else
							tmpStr = QString(BtUtilStr->getString("CopyOfCopyOfFile")).arg(dupeCount).arg(filename(filePaths[i]));
						fileName = tmpStr;
						dupeCount++;
					}

					// update the name as necessary
					succeeded = fsManager->copyFileByName(filePaths[i], destDir, fileName) && succeeded;
				}
			}
			else
			{
				succeeded = fsManager->moveFiles(filePaths, destDir);

				// let explorer know that an optimized move has taken place, and that it doesn't
				// need to delete the source files itself
				if (succeeded)
				{
					FORMATETC sucFmt = {RegisterClipboardFormat(CFSTR_PASTESUCCEEDED), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
					STGMEDIUM sucStgm = {0};
					sucStgm.tymed = TYMED_HGLOBAL;
					sucStgm.hGlobal = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(DWORD));
					*((DWORD*)GlobalLock(sucStgm.hGlobal)) = DROPEFFECT_MOVE;
					GlobalUnlock(sucStgm.hGlobal);
					pdo->SetData(&sucFmt, &sucStgm, TRUE);
				}
			}
		}

		pdo->Release();

		// display notification
		if (succeeded)
		{
			MessageClearPolicy clearPolicy;
				clearPolicy.setTimeout(3);
			QString osstr = QString(BtUtilStr->getString("FilesPasted")).arg(filePaths.size());
			scnManager->messages()->addMessage(new Message("Key_PasteSelection", osstr, Message::Ok, clearPolicy));

			// note the action in the stats
			statsManager->getStats().bt.interaction.clipboard.paste++;
		}
	}
}

void Key_LaunchExplorerWithFileSelected()
{
	if (sel->getBumpObjects().size() != 1)
		return;

	// get the selected object
	BumpObject * obj = sel->getBumpObjects().front();
	if (!obj->isObjectType(ObjectType(BumpActor, FileSystem)))
		return;

	FileSystemActor * actor = (FileSystemActor *) obj;
	QString path = actor->getFullPath();
	LPITEMIDLIST parentPidl = winOS->GetAbsolutePidlFromAbsFilePath(native(parent(path)));
	LPCITEMIDLIST pidl = winOS->GetRelativePidlFromAbsFilePath(path);

	if (actor->isFileSystemType(Virtual))
	{
		parentPidl = winOS->GetPidlFromName(Desktop);
		pidl = winOS->GetPidlFromName(winOS->GetIconTypeFromFileName(path));
	}

	SHOpenFolderAndSelectItems(parentPidl, 1, &pidl, 0);
}

void Key_LaunchExplorerWindowOnWorkingDirectory()
{
	SHELLEXECUTEINFO  sei = {0};
		sei.cbSize = sizeof(sei);
		sei.hwnd = winOS->GetWindowsHandle();
		QString dir = native(scnManager->getWorkingDirectory());
		sei.lpFile = (LPCTSTR) dir.utf16();
		sei.nShow = SW_SHOWNORMAL;
	ShellExecuteEx(&sei);
}

void Key_LaunchCommandWindowOnWorkingDirectory()
{
	SHELLEXECUTEINFO  sei = {0};
		sei.cbSize = sizeof(sei);
		sei.hwnd = winOS->GetWindowsHandle();
		sei.lpFile = L"cmd.exe";
		QString params = QString("/k cd \"%1\"").arg(native(scnManager->getWorkingDirectory()));
		sei.lpParameters = (LPCTSTR) params.utf16();
		sei.nShow = SW_SHOWNORMAL;
	ShellExecuteEx(&sei);
}

void Key_ModifySelectedPhotoFrameSource()
{
	// get the photo frame
	vector<BumpObject *> selectedObjects = sel->getBumpObjects();
	if (selectedObjects.empty())
	{
		printUnique("Key_ModifySelectedPhotoFrameSource", QT_TRANSLATE_NOOP("BtUtilStrings", "Select the Photo Frame you want to modify!"));
	}
	else
	{
		PhotoFrameActor * photoFrame = dynamic_cast<PhotoFrameActor *>(selectedObjects[0]);

		if (photoFrame)
		{
			// show the photoframe dialog with the source already set
			// get the photo frame dialog and reset it to default instance
			PhotoFrameDialog * pfDialog = (PhotoFrameDialog *) dlgManager->getComplexDialog(DialogPhotoFrame);
			photoFrame->preparePhotoFrameDialog(pfDialog);

			// prompt the user as to which kind of photo frame they want
			if (dlgManager->promptDialog(DialogPhotoFrame))
			{
				PhotoFrameSource * source = PhotoFrameActor::resolveSourceFromString(pfDialog->getSourceString());
				if (source)
				{
					photoFrame->setSource(source);
				}
				else
				{
					MessageClearPolicy clearPolicy;
						clearPolicy.setTimeout(2);
					scnManager->messages()->addMessage(new Message("Key_ModifySelectedPhotoFrameSource", QT_TRANSLATE_NOOP("BtUtilStrings", "Invalid photo frame source given"), Message::Warning, clearPolicy));
				}
			}
		}
	}
}

void Key_DeleteSelectedPhotoFrameSource()
{
	// get the photo frame
	vector<BumpObject *> selectedObjects = sel->getBumpObjects();
	PhotoFrameActor * photoFrame = dynamic_cast<PhotoFrameActor *>(selectedObjects[0]);
	if (photoFrame)
	{
		// ensure that the photo frame sources flush their cache
		photoFrame->flushCache();

		// delete it
		FadeAndDeleteActor(photoFrame);
	}
}

void Key_NextMonitor()
{
	winOS->MoveToNextMonitor();
}

void StartModeBasedOnSelection(int x, int y)
{
	
	// Selection
	uint selType = sel->getSelectionType();

	if (GLOBAL(mode) == None && !GLOBAL(touchGestureBrowseMode))
	{
		if (selType & SinglePileMemberItem || selType & MultiplePileMemberItems)
		{
			BumpObject *obj = sel->getPickedActor();

			if (obj)
			{
				if (obj->isParentType(BumpPile))
				{
					Pile *pile = (Pile *) obj->getParent();

					if (pile->getPileState() == LaidOut)
					{
						// InPile Shifting Mode
						GLOBAL(mode) = InPileShifting;

					}
					if (pile->getPileState() == Leaf)
					{
						BumpObject * leafItem = pile->getActiveLeafItem();
						Vec3 pos = leafItem->getGlobalPosition();
						Vec3 cornerPos = pos + leafItem->getDims();
						Vec3 screenPos = WorldToClient(pos);
						Vec3 cornerScreenPos = WorldToClient(cornerPos);
						Vec3 mousePos(x, y, 0);
						float d1 = cornerScreenPos.distance(screenPos);
						float d2 = mousePos.distance(screenPos);
						if (mousePos.distance(screenPos) > cornerScreenPos.distance(screenPos))
						{
							pile->tearLeafedItem();

							sel->clear();
							sel->add(leafItem);
							sel->setPickedActor(leafItem);
						}
					}
					else if (pile->getPileState() == Grid)
					{
						// 
						Vec3 prevClickPos = scnManager->sglClickPos;
						Vec3 clickPos((float) scnManager->mx, (float) scnManager->my, 0);
						if (prevClickPos.distance(clickPos) > scnManager->dblClickSize)
						{
							GLOBAL(mode) = InPileGhosting;

							vector<BumpObject *> selList = sel->getBumpObjects();
							Actor *selActor = (Actor *) sel->getPickedActor();

							for (int i = 0; i < selList.size(); i++)
							{
								BumpObject *obj = selList[i];

								// Use Temporary actors for Ghosting mode
								if (obj->isParentType(BumpPile) && obj->isBumpObjectType(BumpActor) && !(obj->getObjectType() == ObjectType(BumpActor, Temporary)))
								{
									Actor *data = new Actor();
									Actor *thisObj = (Actor *) obj;
									QString texID = thisObj->getTextureID();

									if (thisObj->isActorType(FileSystem))
									{
										FileSystemActor *fsData = (FileSystemActor *) thisObj;										
										if (fsData->isThumbnailized())
											texID = fsData->getThumbnailID();
									}

									data->pushActorType(Temporary);
									data->setDims(thisObj->getDims());
									data->setGlobalPose(thisObj->getGlobalPose());
									data->setGlobalPosition(thisObj->getGlobalPosition() + Vec3(0, thisObj->getDims().z * 2, 0));
									data->setRotation(false);
									data->setObjectToMimic(obj);
									data->setText(thisObj->getText());
									data->showText(true);

									obj->onDragEnd();
									data->onDragBegin();

									// set the drag alpha 
									data->setAlphaAnim(data->getAlpha(), 1.0f, 15);

									repoManager->addToPileSpace(data);
									data->getShapes()[0]->setGroup(PILE_SPACE_OTHER_NUMBER);
									
									// slightly dim the original actor when dragging
									obj->setAlphaAnim(obj->getAlpha(), 0.5f, 15);

									// NOTE: we must add the item to the selection before we clear
									//		 the selection on the other
									sel->add(data);
									sel->remove(obj);
									if (sel->getPickedActor() == obj) sel->setPickedActor(data);
								}
							}

							if (selActor)
							{
								Pile *pile = (Pile *) selActor->getParent();

								// Create a shuffle group
								pile->setShuffleGroup(sel->getBumpObjects());
							}
						}
					}
				}
			}
			return;
		}
		else if (selType & SingleFreeItem)
		{
#ifdef ALLOW_DRAG_CREATE_STICKY_WC_JAN10
			BumpObject *obj = sel->getPickedActor();
			if (obj && obj->isBumpObjectType(BumpActor))
			{
				Actor * actor = (Actor *) obj;
				if (actor->isActorType(Custom))
				{
					CustomActor * customActor = (CustomActor *) actor;
					CustomActor * stickyNotePadActor = scnManager->getCustomActor<StickyNotePadActorImpl>();
					if (customActor == stickyNotePadActor)
					{					
						Vec3 pos = stickyNotePadActor->getGlobalPosition();
						Vec3 cornerPos = pos + stickyNotePadActor->getDims();
						Vec3 screenPos = WorldToClient(pos);
						Vec3 cornerScreenPos = WorldToClient(cornerPos);
						Vec3 mousePos(x, y, 0);
						float d1 = cornerScreenPos.distance(screenPos);
						float d2 = mousePos.distance(screenPos);
						if (mousePos.distance(screenPos) > (cornerScreenPos.distance(screenPos) / 2.0f))
						{
							// create a new sticky note
							StickyNoteActor * stickyNote = CreateStickyNote();
							if (stickyNote)
							{
								stickyNote->setDims(stickyNotePadActor->getDims() * 0.75f);
								stickyNote->setGlobalOrientation(stickyNotePadActor->getGlobalOrientation());
								stickyNote->setGlobalPosition(stickyNotePadActor->getGlobalPosition() + (stickyNotePadActor->getFrontFacePlane().normal * 2.0f));
								repoManager->adjustToDesktopBox(stickyNote);
								stickyNote->finishAnimation();
								stickyNote->onDragBegin((FinishedDragCallBack) EditStickyNoteAfterDrop);

								sel->clear();
								sel->add(stickyNote);
								sel->setPickedActor(stickyNote);
							}
						}
					}
				}
			}
#endif
		}
	}
}

void FinishModeBasedOnSelection()
{
	
	if (GLOBAL(mode) == FanoutOnStrokeMode)
	{
		vector<Pile *> piles = scnManager->getPiles();
		for (int i = 0; i < piles.size(); ++i)
		{
			Pile * pile = piles[i];

			// Clear the lasso if there is no more need for it
			if (pile->getPileState() == LayingOut)
			{
				pile->endFanout();

				// select the last item in the pile so that the pile is selected
				sel->clear();
				sel->add(pile->getFirstItem());
			}
		}
	} 

	GLOBAL(mode) = None;
}


#ifndef DXRENDER
// Render a line to the screen.
// If dotSize is <= 0, the line will not be solid.
void RenderLine(const QList<Vec3>& points, bool inScreenSpace, int dotSize, float lineWidth)
{
	float windowHeight = (float) winOS->GetWindowHeight();
	if (points.size() > 0)
	{
		glPushAttribToken token(GL_ENABLE_BIT);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);

		glLineWidth(lineWidth);

		if (dotSize > 0)
		{
			glEnable(GL_LINE_STIPPLE);
			glLineStipple(dotSize, 0x5555);
		}

		// White with back drop shadow
		themeManager->getValueAsColor("ui.lasso.color.border.top", ColorVal()).setAsOpenGLColor();
		glBegin(GL_LINE_STRIP);
		{
			for (uint i = 0; i < points.size(); i++)
			{
				if (inScreenSpace)
					glVertex2f(points[i].x, windowHeight - points[i].y);
				else
					glVertex3f(points[i].x - 0.3f, 0.05f, points[i].z - 0.3f);
			}
		}
		glEnd();

		// Turn off blending for the black line when lassoing a selection
		if (dotSize > 0)
		{
			glDisable(GL_BLEND);
			glLineStipple(dotSize, ~0x5555);
		}

		themeManager->getValueAsColor("ui.lasso.color.border.bottom",ColorVal()).setAsOpenGLColor();
		glBegin(GL_LINE_STRIP);
		{
			for (uint i = 0; i < points.size(); i++)
			{
				if (inScreenSpace)
					glVertex2f(points[i].x, windowHeight - points[i].y);
				else
					glVertex3f(points[i].x, 0.07f, points[i].z);
			}
		}
		glEnd();
	}
}

void RenderLasso(const QList<Vec3> &lasso, bool useDottedLine, bool inScreenSpace)
{
	int dotSize = useDottedLine ? 5 : 0;
	RenderLine(lasso, inScreenSpace, dotSize);
}
#else
void RenderLasso(const QList<Vec2>& points)
{
	QColor foreground = themeManager->getValueAsColor("ui.lasso.color.border.bottom", ColorVal()).asQColor();
	QColor background = themeManager->getValueAsColor("ui.lasso.color.border.top", ColorVal()).asQColor();
	
	// Consider moving the pattern scale constant (5.0f here) to the theme file
	dxr->renderLine(points, foreground, 1.8f, 5.0f, background);
}
#endif

void CreateRandomAnimPath(NxActorWrapper *actor, Vec3 startPos, Vec3 endPos, int numFrames)
{
	
	Actor *data = GetBumpActor(actor);
	deque<Mat34> poses;
	Vec3 invFallDir = endPos - startPos;
	float deltaXAngle, deltaYAngle, deltaZAngle;
	float angleMinRange = -6.0, angleMaxRange = 6.0; // <-- Tweakable Values (These values are multiplied by the precision variable)
	int precision = 1000;
	Mat34 animMatrix, savedPose;
	Vec3 curPos;
	float x = 0, y = 0, z = 0;
	Quat ori, xRot, yRot, zRot;

	if (data)
	{
		// Calculate the distance and direction this unit has to travel in one time increment
		invFallDir = invFallDir / float(-numFrames);

		// Adjest for precisions of floats to ints
		angleMinRange *= precision;
		angleMaxRange *= precision;

		// Pick random angle rotation Deltas for the animation. Adjust for precision
		deltaXAngle = (angleMinRange + (float) (rand() % (int) (angleMaxRange - angleMinRange))) / precision;
		deltaYAngle = (angleMinRange + (float) (rand() % (int) (angleMaxRange - angleMinRange))) / precision;
		deltaZAngle = (angleMinRange + (float) (rand() % (int) (angleMaxRange - angleMinRange))) / precision;

		// Adjust the end Position on the floor
		Vec3 oldPos = startPos;
		actor->setGlobalPosition(endPos);

		// Get the pose after the animation change
		curPos = actor->getGlobalPosition();
		animMatrix = actor->getGlobalPose();
		actor->setGlobalPosition(oldPos);

		// Loop through each animation step, changing the angle and location of the actor
		for (int i = 0; i < numFrames; i++)
		{
			// Save the matrix into the animation queue
			poses.push_front(animMatrix);

			// Update the Position
			curPos += invFallDir;

			// Rotate around the axis
			x += deltaXAngle;
			y += deltaYAngle;
			z += deltaZAngle;

			// Apply the rotation
			xRot = Quat(x, Vec3(1, 0, 0));
			yRot = Quat(y, Vec3(0, 1, 0));
			zRot = Quat(z, Vec3(0, 0, 1));
			ori = Quat(90, Vec3(1, 0, 0));
			ori *= xRot;
			ori *= yRot;
			ori *= zRot;

			// Apply the quaternion to the object
			animMatrix = Mat34(Mat33(ori), curPos);
		}

		// Save the matrix into the animation queue
		poses.push_front(animMatrix);
		GetBumpActor(actor)->setPoseAnim(poses);

	}
}

bool adjustPointToInsideWalls(Vec3& newPtOut)
{
	return adjustPointToInsideWalls(newPtOut, Vec3(0.0f)); 
}

bool adjustPointToInsideWalls(Vec3& newPtOut, const Vec3& boxInset)
{	
	Box desktopBox = GetDesktopBox();
	desktopBox.extents -= boxInset;

	if (GLOBAL(DrawWalls))
	{
		//desktopBox.containsPoint(newPtOut) will sometimes return false even though the checks below all pass
		if (!desktopBox.containsPoint(newPtOut))
		{
			Vec3 ext = desktopBox.GetExtents();
			Vec3 cent = desktopBox.GetCenter();

			if (newPtOut.x > cent.x + ext.x) newPtOut.x = cent.x + ext.x;
			if (newPtOut.y > cent.y + ext.y) newPtOut.y = cent.y + ext.y;
			if (newPtOut.z > cent.z + ext.z) newPtOut.z = cent.z + ext.z;
			if (newPtOut.x < cent.x - ext.x) newPtOut.x = cent.x - ext.x;
			if (newPtOut.y < cent.y - ext.y) newPtOut.y = cent.y - ext.y;
			if (newPtOut.z < cent.z - ext.z) newPtOut.z = cent.z - ext.z;

			return true;
		}
	}

	return false;
}

bool adjustBoxToInsideWalls(Box& innerBoxOut)
{	
	Vec3 pts[8];
	Vec3 tempPt;
	bool rc = false;

	if (GLOBAL(DrawWalls))
	{
		innerBoxOut.computePoints(pts);

		for (int i = 0; i < 8; i++)
		{
			tempPt = pts[i];

			if (adjustPointToInsideWalls(tempPt))
			{
				if (pts[i].distance(tempPt) > 0.1f)
				{
					innerBoxOut.center -= (pts[i] - tempPt);
					rc = true;

					innerBoxOut.computePoints(pts);
					i = -1;
				}
			}
		}
	}

	return rc;
}

// Returns the adjusted point of the object in world space
Vec3 adjustPtOnWall(BumpObject * obj, NxActorWrapper * wall)
{
	// bound this new point by the wall dims
	Vec3 bufferOffset(kWallPositionBuffer, 0, 0);
	NxActorWrapper * pinWall = wall;
	Mat33 ori = pinWall->getGlobalOrientation();
	Mat33 oriInverse;
		ori.getInverse(oriInverse);
	Vec3 toWallCenter = obj->getGlobalPosition() - pinWall->getGlobalPosition();
	Vec3 orientedToWallCenter = oriInverse * toWallCenter;
		orientedToWallCenter.max(-(pinWall->getDims()) + bufferOffset);
		orientedToWallCenter.min(pinWall->getDims() - bufferOffset);
		orientedToWallCenter.z = -obj->getDims().z - kWallPositionBuffer;
	return wall->getGlobalPosition() + (ori * orientedToWallCenter);
}

Box GetDesktopBox(float buffer)
{
	
	// 		float yBuffer = 10.0f;	//extends wall box vertically by this amount (both up and down), because bad behaviour happens otherwise.  
	// 								//ie, piles move funny because dragging them interpenetrates the floor a little and causes constant repositioning

	const vector<NxActorWrapper*>& walls = GLOBAL(Walls);
	const Vec3List& wallsPos = GLOBAL(WallsPos);
	Vec3 extents(wallsPos[3].x - getDimensions(walls[3]).z + buffer,	// right side of the left wall
		getDimensions(walls[0]).y + buffer, 
		wallsPos[0].z - getDimensions(walls[0]).z + buffer);	// bottom side of the top wall 
	
	
	return Box(Vec3(0, wallsPos[0].y, 0), extents, Mat33(NX_IDENTITY_MATRIX));	
}

void FreezeActor( NxActorWrapper* a, bool freeze/*=true*/ )
{
	GetBumpActor(a)->setFrozen(freeze);
}

void DeleteActor(NxActorWrapper* a)
{
	if (!a) return;
	
	if (a->getUserDataType() == UserDataAvailable && ((BumpObject *) a)->isBumpObjectType(BumpActor))
	{
		Actor *Temp = GetBumpActor(a);

		if (Temp && Temp->isActorType(FileSystem))
		{
			FileSystemActor *fsData = (FileSystemActor *) Temp;

			if (fsData->isPileized())
			{
				FileSystemPile *fsPile = fsData->getPileizedPile();

				if (fsPile->getNumItems())
				{
					fsPile->folderize();
				}

				DeletePile(fsPile, true, true, false);
				fsData->setPileizedPile(NULL);
			}
		}
	}

	// Kill pointers to prevent dangling pointers
	if (sel->getPickedActor() == a) 
	{
		sel->setPickedActor(NULL);
	}

	if(GLOBAL(settings.enableTossing)) { 
		if (vecContains(GLOBAL(Tossing), GetBumpObject(a)) != -1) 
		{ 
			GLOBAL(Tossing).erase(GLOBAL(Tossing).begin() + vecContains(GLOBAL(Tossing), GetBumpObject(a))); 
		}

		//Release Actor
		if (vecContains(GLOBAL(Tossing), GetBumpObject(a)) != -1)
		{
			GLOBAL(Tossing).erase(GLOBAL(Tossing).begin() + vecContains(GLOBAL(Tossing), GetBumpObject(a)));
		}
	}
	
	BumpObject *bumpActor = (BumpObject *) GetBumpActor(a);

	if(!bumpActor)
		return;

	//check selection
	sel->remove(bumpActor);

	bumpActor->breakPin();
}

//Returns the minimum values for the AxisAligned bounded box, that encapsulates the Oriented BB 'obb'
Vec3 minAABBvalues(Box obb)
{
	Bounds bounds;
	bounds.boundsOfOBB(obb.rot, obb.center, obb.extents);
	return bounds.getMin();
}

Vec3 maxAABBvalues(Box obb)
{
	Bounds bounds;
	bounds.boundsOfOBB(obb.rot, obb.center, obb.extents);
	return bounds.getMax();
}

Vec3 WorldToClient(Vec3 p, bool reverseY, bool zeroZ)
{
	// we hit this function a lot, so to save on the calls, we will locally use the singleton
	Camera * camera = cam;
	// TODO world2window should be replaced by this
#ifdef DXRENDER
	D3DXVECTOR3 window;
	D3DXVec3TransformCoord(&window, (const D3DXVECTOR3 *)&p, &dxr->viewProj);
	window.x = (window.x + 1) / 2 * dxr->viewport.Width + dxr->viewport.X;
	if (reverseY)
		window.y = -window.y;
	window.y = (window.y + 1) / 2 * dxr->viewport.Height + dxr->viewport.Y;
	if (zeroZ)
		window.z = 0;
	return Vec3(window.x, window.y, window.z);
#else
	GLdouble wx = 0.0, wy = 0.0, wz = 0.0;  //  returned world x, y, z coords
	float realy;

	GLint * viewport = camera->glViewport();
	GLdouble * projmatrix = camera->glProjectionMatrix();
	GLdouble * mvmatrix = camera->glModelViewMatrix();
	// note viewport[3] is height of window in pixels
	gluProject (p.x, p.y, p.z, mvmatrix, projmatrix, viewport, &wx, &wy, &wz);

	realy = float(reverseY ? viewport[3] - int(wy) - 1 : int(wy));

	if(zeroZ) wz=0;
	return Vec3(float(wx),realy,float(wz));
#endif
}

//Returns what the mouse cursor translates too in world coordinates.  Default is on the floor.  Set yValue to a different world Space number to get a different point on the ray
Vec3 CurrentWorldMousePos()
{
	return ClientToWorld(mouseManager->primaryTouchX, mouseManager->primaryTouchY);
}

Vec3 ClientToWorld(int x, int y,  float height)
{
	Vec3 pt, adjustedPt;
	Ray ray;
	NxF32 dist;
	NxPlane plane = NxPlane(Vec3(0,1,0), height);
	
	// Cast the coordinates into space and figure out the Ray
	Vec3 lineStart, lineEnd, dir;
	window2world(x, y, lineStart, lineEnd);
	dir = lineEnd - lineStart;
	dir.normalize();
	ray = Ray(lineStart, dir);

	// Figure out where the ray hits the floor
	NxRayPlaneIntersect(ray, plane, dist, pt);

	return pt;
}

tuple<int, NxReal, Vec3, NxPlane> unProjectToDesktop(int x, int y, float padding)
{
	Vec3 closePlane, farPlane, point, dir;
	window2world(x, y, closePlane, farPlane);
	dir = farPlane - closePlane;
	dir.normalize();
	Ray touchRay(closePlane, dir);
	return RayIntersectDesktop(touchRay, padding);
}

void oneSecondTick()
{	
	const vector<BumpObject *>& scmObj = scnManager->getBumpObjects();
	vector<BumpObject *> selObj = sel->getBumpObjects();
	int x, y;
	Vec2 oldMousePos = GLOBAL(oldMousePos);

	// Check if the mouse moved
	winOS->GetMousePosition(x, y);
	if (oldMousePos.x != x || oldMousePos.y != y)
	{
		// Mouse Move
	}else{

		// No Mouse Move
		if (sel->getPickedActor())
		{
			for (uint i = 0; i < scmObj.size(); i++)
			{
				if (scmObj[i]->isBeingHoveredOver())
				{
					scmObj[i]->onDropHover(sel->getBumpObjects());
				}
			}

			for (uint i = 0; i < selObj.size(); i++)
			{
				selObj[i]->onDragHover();
			}
		}
	}

	GLOBAL(oldMousePos).x = float(x);
	GLOBAL(oldMousePos).y = float(y);
}

void Key_SelectAll()
{
	vector<BumpObject *> objs = scnManager->getBumpObjects();

	QList<BumpObject *> newSelection;
	for (uint i = 0; i < objs.size(); i++)
	{
		// don't add items in piles or items that are pinned
		if (!objs[i]->isParentType(BumpPile) && !objs[i]->isPinned())
			newSelection.append(objs[i]);
	}
	sel->replace(newSelection);

	printUnique("selectAll", BtUtilStr->getString("SelectAll").arg(sel->getSize()));
}

//Generates a unique GUID string 
QString generateUniqueGUID()
{
	// create a new GUID string
	GUID newGUID;
	CoCreateGuid(&newGUID);
	WCHAR newGUIDStr[129];
	StringFromGUID2(newGUID, newGUIDStr, 128);

	// set the new guid in the registry
	return QString::fromUtf16((const ushort *) newGUIDStr);
}

void * updatePileStateAfterAnim( AnimationEntry *entry )
{
	Pile *pile = (Pile *) entry->getObject();
	pile->updatePileState();
	return NULL;
}

void Key_PlayPause()
{
	vector<BumpObject *> selection = sel->getBumpObjects();
	for (int i = 0; i < selection.size(); ++i)
	{
		BumpObject * obj = selection[i];
		if (obj->getObjectType() == ObjectType(BumpActor, FileSystem))
		{
			FileSystemActor * fsActor = (FileSystemActor *) obj;
			if (fsActor->hasAnimatedTexture())
			{
				fsActor->playAnimatedTexture(!fsActor->isPlayingAnimatedTexture());
			}
		}
	}
}

void Key_OverrideFileTexture()
{
	// ensure that only one item is selected
	vector<BumpObject *> selection = sel->getBumpObjects();
	if (selection.size() != 1)
	{		
		printUnique("Key_OverrideFileTexture", BtUtilStr->getString("SelectFileOverride"));
		return;
	}

	BumpObject * obj = selection.front();
	if (!(obj->getObjectType() == ObjectType(BumpActor, FileSystem)))
	{
		printUnique("Key_OverrideFileTexture", BtUtilStr->getString("ErrorOverrideFiles"));
		return;
	}

	FileSystemActor * actor = (FileSystemActor *) obj;
	if (actor->isThumbnailized())
	{
		printUnique("Key_OverrideFileTexture", BtUtilStr->getString("ErrorOverrideThumbnails"));
		return;
	}

	// prompt to clear current override, or to set a new one
	if (!dlgManager->promptDialog(DialogChangeIcon))
	{
		QString overrideTexture;
		if (actor->getTextureOverride(overrideTexture) && 
			scnManager->containsObject(actor))
		{
			actor->setDimsToDefault();
			actor->setTextureOverride(QString());
			actor->setFilePath(actor->getFullPath());
		}
		return;
	}

	// prompt the user for the texture to be the override
	TCHAR buffer[MAX_PATH] = {0};
	OPENFILENAME ofn = {0};
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = winOS->GetWindowsHandle();
		ofn.lpstrFilter = QT_NT(L"Images\0*.jpg;*.jpeg;*.png;*.gif;*.ico\0\0");
		ofn.nFilterIndex = 1;
		ofn.lpstrFile = buffer;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = (LPCWSTR)QT_TRANSLATE_NOOP("BtUtilStrings", "Choose the icon override:").utf16();
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

	if (GetOpenFileName(&ofn))
	{
		QString filePath = QString::fromUtf16((const ushort *) ofn.lpstrFile);
		if (scnManager->containsObject(actor))
			actor->setTextureOverride(filePath);
	}
}

void orderSpatially2D(vector<BumpObject *> & objectsInOut, bool groupPileItems)
{
	if (objectsInOut.size() == 1)
		return;

	// spatially orders the set of objects, writing the new order of the objects
	// into the same vector specified

	// we're going to sort the objects by their global position by x and y
	// then using the x sorted list, we will iterate the objects in line and 
	// place them in the ordered set

	hash_map<BumpObject *, vector<BumpObject *>> pileItems;
	if (groupPileItems)
	{
		// If the item belongs to a pile, add pile to the sort vector and remove the item itself
		for (uint i = 0; i < objectsInOut.size(); i++)
		{
			if (objectsInOut[i]->isParentType(BumpPile))
			{
				pileItems[objectsInOut[i]->getParent()].push_back(objectsInOut[i]);
				objectsInOut.erase(objectsInOut.begin() + i);
				i--;
			}
		}
		hash_map<BumpObject *, vector<BumpObject *>>::iterator pileItmesIt = pileItems.begin();
		while (pileItmesIt != pileItems.end())
		{
			objectsInOut.push_back((*pileItmesIt).first);
			pileItmesIt++;
		}
	}

	vector<BumpObject *> sx = objectsInOut;
	vector<BumpObject *> sy = objectsInOut;
	map<BumpObject *, int> syi;

	// sort the objects by x/y positions
	sort(sx.begin(), sx.end(), more_x_position());
	sort(sy.begin(), sy.end(), more_z_position());

	// save the indices of each of these items now for lookup later
	for (int i = 0; i < sy.size(); ++i)
		syi.insert(make_pair(sy[i], i));

	// iterate and fill the final positions etc.
	objectsInOut.clear();
	while (!sx.empty())
	{
		BumpObject * front = sx.front();
			objectsInOut.push_back(front);
			sx.erase(sx.begin());
		Vec3 pos = front->getGlobalPosition();
		Vec3 dims = front->getDims();
		float minZ = pos.z - dims.y;
		float maxZ = pos.z + dims.y;

		for (int i = 0; i < sx.size();)
		{
			BumpObject * obj = sx[i];
			float objZ = obj->getGlobalPosition().z;
			if (minZ <= objZ && objZ <= maxZ)
			{
				objectsInOut.push_back(obj);
				sx.erase(sx.begin() + i);
			}
			else
				++i;
		}
	}

	if (groupPileItems)
	{
		// If the item is a pile of original sort items, replace the pile with spatially sorted original pile items.
		for (uint i = 0; i < objectsInOut.size(); i++)
		{
			if (pileItems.count(objectsInOut[i]))
			{
				vector<BumpObject *> & items = pileItems[objectsInOut[i]];
				orderSpatially2D(items, false);
				objectsInOut.insert(objectsInOut.begin() + i, items.begin(), items.end());
				objectsInOut.erase(objectsInOut.begin() + i + items.size());
				i += items.size() - 1;
			}
		}
	}
}

QString printVec3( Vec3 v )
{
	return QString("%1, %2, %3\n").arg(v.x).arg(v.y).arg(v.z);
}

QString printMat33( Mat33 m )
{
	NxF64 ori[9];
	m.getRowMajor(ori);
	QString matrixString = "";
	for (int i = 0; i < 9; i++)
		matrixString += QString("%1, ").arg(ori[i]);
	matrixString += "\n";
	return matrixString;
}

float getHeightForResetLayout(Box *desktopBox, float buffer)
{
	return 55.0f + buffer;
}

void Key_DumpFileSystemActorsToFile()
{
	vector<BumpObject*> actors = scnManager->getVisibleBumpActorsAndPiles();

	QString dump = "";
	for_each(BumpObject* bumpObj, actors)
	{
		dump += "\n";
		if (bumpObj->getObjectType() == ObjectType(BumpActor, FileSystem))
		{
			dump += ((FileSystemActor*)bumpObj)->getTargetPath();
			dump += "\n";
		}

		dump += QT_NT("dims:\t");
		dump += printVec3(bumpObj->getDims());
		dump += QT_NT("\nposition:\t");
		dump += printVec3(bumpObj->getGlobalPosition());
		dump += QT_NT("orientation:\t");
		dump += printMat33(bumpObj->getGlobalOrientation());
	}
	consoleWrite(dump);
	
	QFile dumpFile;
	QTextStream dumpStream(&dumpFile);
	dumpFile.setFileName(winOS->GetSystemPath(DesktopDirectory) + QT_NT("\\filesystemactors.txt"));
	if (dumpFile.open(QFile::WriteOnly))
	{
		// set the codec (it will use the local code page otherwise) for decoding the text stream
		dumpStream.setCodec(QT_NT("UTF-8"));

		dumpStream << dump;
		dumpFile.close();
	}
	

}

void Key_CreateSharingWidget()
{
#ifdef ENABLE_WEBKIT
	LOG_LINE_REACHED();
	// check if we already have one first
	vector<BumpObject *> webActors = scnManager->getBumpObjects(ObjectType(BumpActor, Webpage));
	for (int i = 0; i < webActors.size(); ++i) 
	{
		WebActor * actor = (WebActor *) webActors[i];
		if (actor->isSharingWidget()) 
		{
			// Just silently return, previously we flash the existing actor.
			sel->clear();
			return;
		}
	}
	WebActor *actor = new WebActor();
	actor->load(getSharedFolderUrl());
	actor->setDims(actor->getDefaultDims());
#endif
}

void Key_CreateFacebookWidget() 
{	
#ifdef ENABLE_WEBKIT
	LOG_LINE_REACHED();
	// check if we already have one first
	vector<BumpObject *> webActors = scnManager->getBumpObjects(ObjectType(BumpActor, Webpage));
	for (int i = 0; i < webActors.size(); ++i) 
	{
		WebActor * actor = (WebActor *) webActors[i];
		if (actor->isFacebookWidgetUrl()) 
		{
			sel->clear();
			return;
		}
	}
	// otherwise, create a new web actor
	WebActor * widgetActor = new WebActor();

	// The second parameter is true because in the case where the slideshow
	// "facebook" button is clicked with no widget, a widget will be created
	// and the resultant javascript code has to be executed.
	widgetActor->load("bumpwidget-facebook://newsfeed", true);

	// position the widget
	Box desktopBox = GetDesktopBox();
	float edgeBuffer = 30.0f;

	// Facebook widget
	if (widgetActor)
	{
		widgetActor->setGlobalOrientation(Quat(0, Vec3(1,0,0)));

		// move the Facebook widget to be near the right edge of the back wall
		Vec3 facebookWidgetpos(desktopBox.center.x - desktopBox.extents.x + widgetActor->getDims().x + edgeBuffer, 
			getHeightForResetLayout(&desktopBox), 
			desktopBox.center.z + desktopBox.extents.z);

		widgetActor->setGlobalPosition(facebookWidgetpos);
		widgetActor->onPin();
	}

#endif
}

bool isSlideshowModeActive()
{
	return cam->inSlideshow();
}

void printTimedUnique( QString key, int time, QString str )
{
	if (!str.isNull() && !str.isEmpty()) 
	{ 
		MessageClearPolicy policy; 
		policy.setTimeout(time); 
		scnManager->messages()->addMessage(new Message(key, str, Message::Ok, policy)); 
	}	
}

// Return the angle in degrees between the two vectors 
float getAngleBetween(Vec3 firstVector, Vec3 secondVector)
{
	firstVector.normalize();
	secondVector.normalize();
	return acosf(firstVector.dot(secondVector)) * (180.0f / PI);
}

bool isParallel(const Mat33& firstOri, const Mat33& secondOri)
{
	const float epsilon = 0.001f;
	Vec3 normal(0, 0, -1);
	return (getAngleBetween(firstOri * normal, secondOri * normal) < epsilon);
}

// Based off the Jarvis March or Gift Wrapping algorithm. Calculates 
// the convex hull of a set of points. It is O(n^2)
Vec3List getConvexHull(Vec3List points)
{
	int numPoints = points.size();
	assert(numPoints > 2);

	Vec3List hull;
	int startingIndex = 0;

	// Find left-most point (guaranteed to be on the hull
	for(int index = 1; index < numPoints; index++)
	{
		if (points[index].x < points[startingIndex].x)
			startingIndex = index;
	}

	// How the algorithm works.
	// 1) Create a comparison vector from the last two points on the hull
	// 2) Create vectors from all the points and the last point added to the hull
	// 3) The end point of the vector with the smallest angle between the comparison vector is the next entry on the hull
	// 4) Repeat

	Vec3 comparisonVector(0.0f, 1.0f, 0.0f);
	int lastAcceptedIndex = startingIndex;
	int beforeLastAcceptedIndex = startingIndex;
	while (lastAcceptedIndex != startingIndex || beforeLastAcceptedIndex == startingIndex)
	{
		float smallestAngle = 180.0f;
		int smallestIndex = 0;

		for (int hullCandidateIndex = 0; hullCandidateIndex < numPoints; hullCandidateIndex++)
		{
			if (hullCandidateIndex != lastAcceptedIndex && hullCandidateIndex != beforeLastAcceptedIndex)
			{
				float angle = getAngleBetween(comparisonVector, points[hullCandidateIndex] - points[lastAcceptedIndex]);			
				if (angle < smallestAngle)
				{
					smallestAngle = angle;
					smallestIndex = hullCandidateIndex;
				}
			}
		}

		hull.push_back(points[smallestIndex]);
		beforeLastAcceptedIndex = lastAcceptedIndex;
		lastAcceptedIndex = smallestIndex;
		comparisonVector = points[lastAcceptedIndex] - points[beforeLastAcceptedIndex];
	}
	return hull;
}

// Checks to see if a back up of this photo exists in a hidden directory
bool hasOriginalPhoto(FileSystemActor* photo)
{
	QString currentPath = photo->getFullPath();
	QDir fileDirectory(QFileInfo(currentPath).absoluteDir());

	// If the back up folder does not exist, there is no back up
	if (!fileDirectory.cd(originalPhotoFolderName))
		return false;

	// Check if the file exists
	return fileDirectory.exists(filename(currentPath));
}

// Backs up the photo into a hidden directory
bool backupOriginalPhoto(FileSystemActor* photo)
{
	QString currentPath = photo->getFullPath();
	QDir fileDirectory(QFileInfo(currentPath).absoluteDir());
	
	// Navigate to the back up folder
	if (!fileDirectory.cd(originalPhotoFolderName))
	{
		// If it doesn't exist, create it
		if (!fileDirectory.mkdir(originalPhotoFolderName))
			return false;
		
		fileDirectory.cd(originalPhotoFolderName);

		// Make sure the folder is set as "Hidden"
		fsManager->setFileAttributes(fileDirectory.absolutePath(), Hidden);
	}

	QString originalPath = native(make_file(fileDirectory, filename(currentPath)));
	
	// Remove a previous back-up if it exists. Otherwise, QFile::copy fails
	if (QFile::exists(originalPath))
		QFile::remove(originalPath);

	// Copy the file to the back up folder
	return QFile::copy(currentPath, originalPath);
}

// If a backup of the photo exists, this function replaces that photo
// and removes the backup
bool restoreOriginalPhoto(FileSystemActor* photo)
{
	// If there is nothing to restore, return false
	if (!hasOriginalPhoto(photo))
		return false;

	// Navigate to the original photo folder
	QString currentPath = photo->getFullPath();
	QDir fileDirectory(QFileInfo(currentPath).absoluteDir());
	if (!fileDirectory.cd(originalPhotoFolderName))
		return false;		
		
	QFile originalFile(fileDirectory.absoluteFilePath(filename(currentPath)));
	QString originalPath = native(originalFile);
	
	// QT can't overwrite files, so we delete the current one...
	if (!QFile::remove(currentPath))
	{
		return false;
	}
	
	// Move the back up file to the current path
	if (!QFile::rename(originalPath, currentPath))
	{
		return false;
	}

	// Attempt to remove the directory. Will fail if it is not empty
	fileDirectory.cd(QString(".."));
	fileDirectory.rmdir(originalPhotoFolderName);
	return true;
}

int getRemainingTrialDays()
{
	QDateTime expiryTime = QFileInfo(native(winOS->GetUserApplicationDataPath())).created().addDays(GLOBAL(trialDays));
	return QDateTime::currentDateTime().daysTo(expiryTime);	
}

void startAutomatedTests()
{
	Replayable * replay = scnManager->getReplayable();
	AutomatedTests * demo = dynamic_cast<AutomatedTests *>(replay);
	if (demo)
	{
		if (replay->getPlayState() == Replayable::Stopped)
		{
			// start the demo
			replay->play();
		}
		else
		{
			evtManager->interruptIdleTimer();
		}
	}
	else
	{
		// create a new automated demo
		demo = new AutomatedTests;
		scnManager->setReplayable(demo);
		demo->play();
	}
}

void ForcePush(Vec3 pointOfOriginScreenSpace)
{
	vector<BumpObject*> objs = scnManager->getBumpObjects();
	Vec3 origin = unProjectToDesktop(pointOfOriginScreenSpace.x, pointOfOriginScreenSpace.y).get<2>();
	for (int i = 0; i < objs.size(); i++)
	{
		Vec3 dir = objs[i]->getGlobalPosition() - origin;
		float mag = dir.magnitude();
		dir.normalize();
		Vec3 vel = dir * (100000.0f / (mag * mag));
		vel.y = 20.0f;
		objs[i]->setLinearVelocity(vel);
	}
}

deque<Mat34> slerpPose(const Mat34& begin, const Mat34& end, const int count, EaseFunctionType easeType)
{
	assert(count > 0);
	deque<Mat34> range;
	Quat beginQuat, endQuat;
	begin.M.toQuat(beginQuat);
	end.M.toQuat(endQuat);

	if (count == 1)
	{	
		range.push_back(begin);
		return range;
	}

	for (int i = 0; i < count; i++)
	{
		float t = float(i) / float(count - 1);

		// Apply Easing
		switch (easeType)
		{
		case Ease:
			t = ease(t);
			break;
		case SoftEase:
			t = softEase(t);
			break;
		default:
			break;
		}

		Quat quat;
		quat.slerp(t, beginQuat, endQuat);
		Vec3 position = lerp(begin.t, end.t, t);
		range.push_back(Mat34(quat, position));
	}

	return range;
}

// Return true if the given URL for a shared folder widget
bool isSharedFolderUrl( const QString& url )
{
	return url.startsWith("bumpsharedfolder://");
}

// Return a shared folder URL that refers to the given dir.
// This is the inverse of fromSharedFolderUrl().
// 'info' allows extra information about this sharing widget to be stored.
QString getSharedFolderUrl(const QString& info/*=QString::null*/)
{
	return QString("bumpsharedfolder://") + (info.isNull() ? "" : info);
}

// Animate the given list of objects back to their original pre-drop pose.
// A useful helper for actors that handle drag and drop.
void animateObjectsBackToPreDropPose( const vector<BumpObject *>& objs )
{
	// Animate back to the original starting pose
	for (uint i = 0; i < objs.size(); i++)
	{
		if (scnManager->containsObject(objs[i]))
			objs[i]->restoreStateFromBeforeDrag(objs[i]);
	}
}

BtUtilStrings::BtUtilStrings()
{
	_strs.insert("InternalError", QT_TR_NOOP("An Internal Error occurred, attempting to recover.\nPlease restart BumpTop now."));
	_strs.insert("TextIconLabel", QT_TR_NOOP("Enter Label for Bumpable Text Icon"));
	_strs.insert("ToggleInvisibleActors", QT_TR_NOOP("Invisible Actors Toggled"));
	_strs.insert("LaunchSettings", QT_TR_NOOP("Launching BumpTop Settings"));
	_strs.insert("LaunchTargetPrompt", QT_TR_NOOP("Enter the new launch target (Leave empty to clear the override):"));
	_strs.insert("LaunchTargetSet", QT_TR_NOOP("Launch target override set"));
	_strs.insert("LaunchTargetObj", QT_TR_NOOP("Select the object to set the launch override"));
	_strs.insert("CustomActorNamePrompt", QT_TR_NOOP("Enter the name of this custom actor:"));
	_strs.insert("CustomActorTexturePrompt", QT_TR_NOOP("Enter the path of the texture representing this actor:"));
	_strs.insert("UpgradeToPro", QT_TR_NOOP("Upgrade to the Pro version of BumpTop to get more Stickies\nand a bunch of other great features!"));
	_strs.insert("LeaveStickyNote", QT_TR_NOOP("Leave a Sticky Note"));
	_strs.insert("DoubleClickEditNote", QT_TR_NOOP("Double-click (or press Enter) to edit the Sticky Note again"));
	_strs.insert("RefreshDesktop", QT_TR_NOOP("Refreshing Desktop"));
	_strs.insert("ErrorFolderize", QT_TR_NOOP("This pile contains virtual items, and can not be named and converted to a folder."));
	_strs.insert("PileNamePrompt", QT_TR_NOOP("Enter the name of the new pile:\n(This will create a new folder and move the selected items into it.)"));
	_strs.insert("SelectDirectory", QT_TR_NOOP("Please Select a Directory to Load From"));
	_strs.insert("ClearBumpTopPrompt", QT_TR_NOOP("Would you like to Clear your BumpTop?"));
	_strs.insert("ErrorRenameObjectType", QT_TR_NOOP("Only Files and Folder Piles can be renamed!"));
	_strs.insert("ErrorRenameSelection", QT_TR_NOOP("Select the item that you wish to rename!"));
	_strs.insert("ErrorRenameIllegalChar", QT_TR_NOOP("A file name can't contain any of the following characters: \\/:*?\"<>|"));
	_strs.insert("ErrorReloginSelection", QT_TR_NOOP("Select the item that you wish to re-login to!"));
	_strs.insert("ErrorReloginSelectionUnsupported", QT_TR_NOOP("This item doesn't require login!"));
	_strs.insert("ErrorReloginSelectionFacebook", QT_TR_NOOP("Logged out of BumpTop Facebook Application"));
	_strs.insert("ErrorReloginSelectionTwitter", QT_TR_NOOP("Logged out of Twitter"));
	_strs.insert("SearchFilenames", QT_TR_NOOP("Search within filenames:"));
	_strs.insert("LaunchPrompt", QT_TR_NOOP("Are you sure that you want to launch these %1 items?"));
	_strs.insert("InfiniteBumpTop", QT_TR_NOOP("Infinite BumpTop Mode"));
	_strs.insert("NormalBumpTop", QT_TR_NOOP("Normal Desktop Mode"));
	_strs.insert("NoFilesFound", QT_TR_NOOP("No files found containing \"%1\""));
	_strs.insert("FilesFound", QT_TR_NOOP("%1 files found containing \"%2\""));
	_strs.insert("SimilarFilesFound", QT_TR_NOOP("%1 similar files found containing \"%2\""));
	_strs.insert("ScrollWheelLeafPile", QT_TR_NOOP("Use the scrollwheel or arrow keys to quickly leaf through the pile!"));
	_strs.insert("AutoPilePrompt", QT_TR_NOOP("This will sort and move ALL items on the desktop.  Do you want to continue?"));
	_strs.insert("SortIntoPileMessage", QT_TR_NOOP("%1 files sorted into %2 pile(s)"));
	_strs.insert("ErrorAutoPileNoItems", QT_TR_NOOP("There are no free items to sort!"));
	_strs.insert("ErrorAutoPileSingleItem", QT_TR_NOOP("Only one free item found!"));
	_strs.insert("ResetLayoutPrompt", QT_TR_NOOP("Delete your current BumpTop layout and regenerate from your existing windows icon positions?\n\nNote: All piles, pinning, re-sizing and BumpTop meta-data will be lost"));
	_strs.insert("FeedbackSent", QT_TR_NOOP("Thanks for your help, your feedback information was sent.\nBump On!"));
	_strs.insert("ErrorFeedback", QT_TR_NOOP("An Error occured while trying to send your feedback.\nPlease send your feedback manually to feedback@bumptop.com"));
	_strs.insert("ErrorTests", QT_TR_NOOP("This version of BumpTop was not built with Tests"));
	_strs.insert("ErrorMultipleSources", QT_TR_NOOP("Sorry, we currently do not support operations on\nselections from two separate folders!"));
	_strs.insert("NoItemsSelected", QT_TR_NOOP("No valid items selected!"));
	_strs.insert("FilesCopied", QT_TR_NOOP("%1 file(s) copied"));
	_strs.insert("FilesCut", QT_TR_NOOP("%1 file(s) cut"));
	_strs.insert("FilesPasted", QT_TR_NOOP("%1 file(s) pasted"));
	_strs.insert("CopyOfFile", QT_TR_NOOP("Copy of %1"));
	_strs.insert("CopyOfCopyOfFile", QT_TR_NOOP("Copy (%1) of %2"));
	_strs.insert("SelectFileOverride", QT_TR_NOOP("Select the file you want to override the icon for."));
	_strs.insert("ErrorOverrideFiles", QT_TR_NOOP("You can only change icons for files!"));
	_strs.insert("ErrorOverrideThumbnails", QT_TR_NOOP("You can't change the icon for thumbnailed pictures!"));
	_strs.insert("SelectAll", QT_TR_NOOP("%1 object(s) selected"));
	_strs.insert("StickyNoteLimit", QT_TR_NOOP("Creating more than two sticky notes is a PRO feature"));
	_strs.insert("WebWidgetLimit", QT_TR_NOOP("Creating more than two web widgets is a PRO feature"));
	_strs.insert("NamePileCaption", QT_TR_NOOP("Name this Pile"));
	_strs.insert("FolderizePrompt", QT_TR_NOOP("Enter the name of the new folder.\nNote: All items in this pile will be moved into this folder."));
	_strs.insert("NameFolder", QT_TR_NOOP("Name this folder"));
	_strs.insert("NameFolderAgain", QT_TR_NOOP("You have entered an invalid folder name, please try again"));
	_strs.insert("WebWidgetPrompt", QT_TR_NOOP("Enter a web address or Google Gadget snippet:"));
	_strs.insert("SharingNoFolders", QT_TR_NOOP("You can't share folders, only files"));

	//pile labels
	_strs.insert("Shortcuts", QT_TR_NOOP("Shortcuts"));
	_strs.insert("System Icons", QT_TR_NOOP("System Icons"));
	_strs.insert("Folders", QT_TR_NOOP("Folders"));
	_strs.insert("Images", QT_TR_NOOP("Images"));
	_strs.insert("Executables", QT_TR_NOOP("Executables"));
	_strs.insert("Sticky Notes", QT_TR_NOOP("Sticky Notes"));
	_strs.insert("Misc Item", QT_TR_NOOP("Misc Item"));
	_strs.insert("00Today", QT_TR_NOOP("Today"));
	_strs.insert("01Yesterday", QT_TR_NOOP("Yesterday"));
	_strs.insert("02Last Week", QT_TR_NOOP("Last Week"));
	_strs.insert("03This Month", QT_TR_NOOP("This Month"));
	_strs.insert("04This Year", QT_TR_NOOP("This Year"));
	_strs.insert("05A long time ago", QT_TR_NOOP("A long time ago")); // in a galaxy far away.

}

//Removes invalid characters from a filename string
bool stripInvalidChars(QString &fileName) {
	bool containedInvalids = false;
	QString invalids = "\\:/*?\"<>|";
	QString fixedName = "";

	for(int i = 0; i < fileName.size(); i++) {
		for(int x = 0; x < invalids.size(); x++) {
			if(fileName.at(i) == invalids.at(x)) {
				containedInvalids = true;
			} else {
				fixedName.append(fileName.at(i));
			}
		}
	}

	fileName = fixedName;
	return containedInvalids;
}

void SwitchToLibrary(QSharedPointer<Library> library)
{
	if (!library)
		return;

	if (scnManager->getCurrentLibrary() == library)
		return;

	bool createDefaultObjects = false;
	if (winOS->GetLibraryManager())
	{
		createDefaultObjects = library == winOS->GetLibraryManager()->getDesktopLibrary();
	}

	GLOBAL(skipSavingSceneFile) = false;
	SaveSceneToFile();
	ClearBumpTop();
	scnManager->setCurrentLibrary(library);
	if (!LoadSceneFromFile())
		CreateBumpObjectsFromWorkingDirectory(createDefaultObjects);
}

void RemoveLibrary(QSharedPointer<Library>& library)
{
	if (!library)
		return;

	LibraryManager* libManager = winOS->GetLibraryManager();
	if (!libManager)
		return;

	if (library->getHashKey() == libManager->getDesktopLibrary()->getHashKey())
		return;
	
	libManager->removeLibrary(library);
	
	QString sceneFileName;
	QString sceneFileNameBak;
	if (library->getHashKey().startsWith(QT_NT("usr_")))
	{
		QString sceneHash = QString(QCryptographicHash::hash(library->getFolderPaths().front().toUtf8(), QCryptographicHash::Md5).toHex());
		sceneFileName = QString_NT("%1_scene.pb.bump").arg(sceneHash);
		sceneFileNameBak = sceneFileName + QT_NT(".bak");
	}
	else
	{
		sceneFileName = QString_NT("%1_scene.pb.bump").arg(library->getName());
		sceneFileNameBak = sceneFileName + QT_NT(".bak");
	}
	QFileInfo sceneFile = make_file(winOS->GetDataDirectory(), sceneFileName);
	QFileInfo sceneFileBak = make_file(winOS->GetDataDirectory(), sceneFileNameBak);
	if (exists(sceneFile))
		fsManager->deleteFileByName(sceneFile.absoluteFilePath(), true, true);
	if (exists(sceneFileBak))
		fsManager->deleteFileByName(sceneFileBak.absoluteFilePath(), true, true);
}

void CleanUpSceneFiles()
{
	LibraryManager* libManager = winOS->GetLibraryManager();
	if (!libManager)
		return;
	
	QDir dir(winOS->GetDataDirectory());
	QStringList filters;
	filters << QT_NT("*.bump") << QT_NT("*.bump.bak");
	QStringList files = dir.entryList(filters);
	
	const QList< QSharedPointer<Library> >& libraries = libManager->getLibraries();
	QListIterator< QSharedPointer<Library> > iter(libraries);
	while (iter.hasNext())
	{
		const QSharedPointer<Library>& lib = iter.next();		
		
		QString sceneFileName;
		QString sceneFileNameBak;
		if (lib->getHashKey().startsWith(QT_NT("def_")))
		{
			sceneFileName = QString_NT("scene.pb.bump");
			sceneFileNameBak = QString_NT("scene.pb.bump.bak");
		}
		else if (lib->getHashKey().startsWith(QT_NT("lib_")))
		{
			sceneFileName = QString_NT("%1_scene.pb.bump").arg(lib->getName());
			sceneFileNameBak = sceneFileName + QT_NT(".bak");
		}
		else
		{
			QString sceneHash = QString(QCryptographicHash::hash(lib->getFolderPaths().front().toUtf8(), QCryptographicHash::Md5).toHex());
			sceneFileName = QString_NT("%1_scene.pb.bump").arg(sceneHash);
			sceneFileNameBak = sceneFileName + QT_NT(".bak");
		}

		bool mainTested = false;
		bool backUpTested = false;
		QMutableStringListIterator fileIter(files);
		while (fileIter.hasNext() && !(backUpTested && mainTested))
		{
			QString filePath = fileIter.next();
			if (filePath == sceneFileName)
			{
				fileIter.remove();
				mainTested = true;
				
			}
			if (filePath == sceneFileNameBak)
			{
				fileIter.remove();
				backUpTested = true;
			}
		}
	}

	// Erase files that still exist
	QStringListIterator fileIter(files);
	while (fileIter.hasNext())
	{
		QString filePath = fileIter.next();
		dir.remove(filePath);
	}
}

float roundOffDecimal(float number)
{
	return (float)((int)(number + 0.5f));
}

Vec3& roundOffDecimals(Vec3& vecNum)
{
	vecNum.x = roundOffDecimal(vecNum.x);
	vecNum.y = roundOffDecimal(vecNum.y);
	vecNum.z = roundOffDecimal(vecNum.z);
	return vecNum;
}

QString BtUtilStrings::getString( QString key )
{
	QHash<QString, QString>::const_iterator iter = _strs.find(key);
	if (iter != _strs.end())
		return iter.value();
	assert(false);
	return QString();
}

QString BtUtilStrings::tryGetString(QString key)
{
	QHash<QString, QString>::const_iterator iter = _strs.find(key);
	if (iter != _strs.end())
		return iter.value();
	wprintf(L"BtUtilStrings::tryGetString failed to find \"%s\" \n", key.utf16());
	return key;
}

ConsoleWriteGuard::ConsoleWriteGuard(QString k)
: key(k)
, threshold(-1)
{
	consoleWrite(QString("%2%1\n").arg(k).arg(QString(count, '\t')));
	timer.restart();
	++count;
}

ConsoleWriteGuard::ConsoleWriteGuard( QString k, int t )
: key(k)
, threshold(t)
{
	consoleWrite(QString("%2%1?\n").arg(k).arg(QString(count, '\t')));
	timer.restart();
	++count;
}

ConsoleWriteGuard::~ConsoleWriteGuard()
{
	--count;
	uint elapsed = timer.elapsed();
	if (elapsed > threshold)
		consoleWrite(QString("%3~%1\t(%2)\n").arg(key).arg(elapsed).arg(QString(count, '\t')));
}
int ConsoleWriteGuard::count = 0;
