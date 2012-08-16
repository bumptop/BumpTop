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
#include "BT_BumpObject.h"
#include "BT_Camera.h"
#include "BT_Find.h"
#include "BT_GLTextureManager.h"
#include "BT_LassoMenu.h"
#include "BT_Logger.h"
#include "BT_ObjectType.h"
#ifdef DXRENDER
	#include "BT_DXRender.h"
#endif
#include "BT_Pile.h"
#include "BT_OverlayComponent.h"
#include "BT_RenderManager.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"
#include "BT_Bubbles.h"

#define RESOURCES_CFG "resources.cfg"

RenderManager::RenderManager()
: themeManagerRef(themeManager)
{
	// Init
#ifdef DXRENDER
#else
	hDeviceContext = NULL;
	hRenderContext = NULL;
	memset(&pfd, NULL, sizeof(PIXELFORMATDESCRIPTOR));
	pixelFormat = 0;
#endif
	multiSamplingEnabled = false;
	vSyncEnabled = false;
	hWnd = NULL;
	themeManager->registerThemeEventHandler(this);
}

RenderManager::~RenderManager()
{
	// skip theme change events
	if (shared_ptr<ThemeManager> tm = themeManagerRef.lock())
		tm->unregisterThemeEventHandler(this);

#ifdef DXRENDER
#else
	// Windows Cleanup
	wglMakeCurrent(NULL,  NULL);

	// Delete the current Context
	if (hRenderContext)
	{
		wglDeleteContext(hRenderContext);
		hRenderContext = NULL;
	}

	if (hDeviceContext)
	{
		ReleaseDC(hWnd, hDeviceContext);
		hDeviceContext = NULL;
	}
#endif
}

bool RenderManager::addObject(Renderable *obj)
{
	// Add if its not in the list
	if (getIndex(obj) == -1)
	{
		renderList.push_back(obj);
		return true;
	}

	return false;
}

bool RenderManager::removeObject(Renderable *obj)
{
	int indx = getIndex(obj);

	// If its in the list, remove it
	if (indx != -1)
	{
		renderList.erase(renderList.begin() + indx);
		return true;
	}

	return false;
}

void RenderManager::moveObjectToIndex(Renderable * obj, unsigned int index)
{
	for (uint i = 0; i < renderList.size(); i++)
	{
		if (renderList[i] == obj)
		{
			renderList.erase(renderList.begin() + i);
			_ASSERT(index >= 0 && index <= renderList.size());
			renderList.insert(renderList.begin() + index, obj);
			break;
		}
	}
}

unsigned int RenderManager::getRenderListSize()
{
	return renderList.size();
}

void RenderManager::onRender()
{
	uint sz = renderList.size();
	vector<Renderable *> curList;
	vector<Renderable *> selList;
	vector<Renderable *> pileList;
	vector<Renderable *> camHighlighted;

	if (cam->hasWatchedActorHighlighted())
		camHighlighted.push_back(cam->getHighlightedWatchedActor());

	// Prioritize the list
	for (uint i = 0; i < sz; i++)
	{
		BumpObject *obj = dynamic_cast<BumpObject *>(renderList[i]);
		
		// skip the highlighted object
		if (!camHighlighted.empty() && (obj == camHighlighted.front()))
			continue;

		// Ignore the ones that are not enabled or that are not visible
		if (renderList[i]->getRenderType() == WorldSpace && 
			renderList[i]->isEnabled())
		{
			// -------------------------------------------------------------------------
			// NOTE: We could implement some sort of sorting mechanism to sort items 
			//       from the furthest back to the closest to the camera.
			// -------------------------------------------------------------------------

			if (obj->isBumpObjectType(BumpPile))
			{
				pileList.push_back(obj);
			}

			if (obj->isSelected())
				selList.push_back(renderList[i]);
			else
				curList.push_back(renderList[i]);
		}
	}

	// Sort render order
	curList = sortRenderOrder(curList);

	// Rendering Passes
	prepareRenderSelectionHighlight();
		bubbleManager->drawClusters();
		renderSelectionHighlight(curList);
		renderSelectionHighlight(selList);
	finalizeRenderSelectionHighlight();
	prepareRenderShadows();
		renderShadows(curList);
		renderShadows(selList);
		renderShadows(pileList);
	finalizeRenderShadows();
	renderObjects(curList);
	
	//Fades out objects during Find as you Type
	Finder->renderGrayScreen();

	renderObjects(selList);

	// render the cam highlight object
	if (!camHighlighted.empty())
	{
		// render a dimmed quad?
		/*
		switchToOrtho();
		{
			glPushAttribToken token(GL_ENABLE_BIT);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);

			// render dimmer
			glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
			glPushMatrix();
				glBegin(GL_QUADS);
					glVertex2f(0,0);
					glVertex2f((float)winOS->GetWindowWidth(),0);
					glVertex2f((float)winOS->GetWindowWidth(),(float)winOS->GetWindowHeight());
					glVertex2f(0,(float)winOS->GetWindowHeight());
				glEnd();
			glPopMatrix();
		}
		switchToPerspective();
		*/

		renderObjects(camHighlighted);
	}

	// Validate the Rendering System
	renderIsNeeded = false;
}

int RenderManager::getIndex(Renderable *obj)
{
	// Search for the object in this list that matches the query
	for (uint i = 0; i < renderList.size(); i++)
	{
		if (renderList[i] == obj) return (int) i;
	}

	return -1;
}

void RenderManager::prepareRenderShadows()
{
#ifdef DXRENDER
	dxr->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
	dxr->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	dxr->device->LightEnable(0, false);
	dxr->device->SetRenderState(D3DRS_AMBIENT, 0xffffffff);
	dxr->device->SetRenderState(D3DRS_ZENABLE, false);
	dxr->device->SetMaterial(&dxr->shadowMaterial);
	const float shadowMat[] =
	{
		1, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	};
	dxr->device->MultiplyTransform(D3DTS_VIEW, &D3DXMATRIX(shadowMat));
#else
	const float ShadowMat[]= {
		1, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1};

	/*
	const float TransMat[] = {
		1, 0, 0, 0,
		0, 1, 0, 0, // 0.01 for less dramatic shadows
		0, 0, 1, 0,
		0, 0, 0, 1};
	*/

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	glMultMatrixf(ShadowMat);
	/*
	glMultMatrixf(TransMat);
	*/
#endif
}

void RenderManager::finalizeRenderShadows()
{
#ifdef DXRENDER
	dxr->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	dxr->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	dxr->device->LightEnable(0, true);
	dxr->device->SetRenderState(D3DRS_AMBIENT, 0);
	dxr->device->SetRenderState(D3DRS_ZENABLE, true);
	dxr->device->SetTransform(D3DTS_VIEW, &dxr->view);
#else
	glPopMatrix();
	glPopAttrib();
#endif
}

void RenderManager::renderShadows(vector<Renderable *> &curList)
{
	uint sz = curList.size();
	float maxShadowAlpha = 0.325f;
	float alpha, dist;
	BumpObject *obj;
	Vec3 pos, lPos, lDir, dims;
	// Get the position of the light
	lPos.set(_light0Position[0], _light0Position[1], _light0Position[2]);
	// NOTE: work around since we want the shadow to be in the opposite dir of the light
	lDir.set(-_light0Direction[0], _light0Direction[1], -_light0Direction[2]);

	NxPlane floor(Vec3(0, 0, 0), Vec3(0, 1, 0));
	NxRay ray;	
		ray.dir = -lDir;
		ray.dir.normalize();
	ObjectType type;
	
	// Loop through each item and render its shadow
	for (uint i = 0; i < sz; i++)
	{
		obj = (BumpObject *) curList[i];
		type = obj->getObjectType();
		dims = obj->getDims();

		// Ignore items that are pinned, invisible, or temporary
		if (obj->isPinned()) continue;
		if (type == ObjectType(BumpActor, Temporary)) continue;
		if (type == ObjectType(BumpActor, Invisible)) continue;
		if (type == ObjectType(BumpWidget, Invisible)) continue;
		if (obj->getAlpha() <= 0.1f) continue;

		// Project the shadow to the floor		
		ray.orig = obj->getGlobalPosition();
		NxRayPlaneIntersect(ray, floor, dist, pos);

		// Modulate shadow strength (alpha) according to actor's height and alpha value
		dist = NxMath::max(0.005f, NxMath::abs(dist));
		alpha = obj->getAlpha() * maxShadowAlpha * (1.0f / dist);

		// Ignore shadows of nearly invisible values
		if (alpha <= 0.01f)
			continue;
#ifdef DXRENDER
		if (obj->getObjectType() == ObjectType(BumpPile, NULL, Grid))
		{
			alpha = NxMath::min(alpha, 0.14f);
			if (dxr->shadowMaterial.Diffuse.a != alpha)
			{
				dxr->shadowMaterial.Diffuse.a = dxr->shadowMaterial.Ambient.a = NxMath::min(alpha, 0.14f);
				dxr->device->SetMaterial(&dxr->shadowMaterial);
			}
			dxr->renderSideLessBox(pos, obj->getGlobalOrientation(), dims, texMgr->getGLTextureId("Default"));
		}
		else if (!(obj->getObjectType() == ObjectType(BumpPile)) && !(obj->getObjectType() == ObjectType(BumpCluster)))
		{
			if (dxr->shadowMaterial.Diffuse.a != alpha)
			{
				dxr->shadowMaterial.Diffuse.a = dxr->shadowMaterial.Ambient.a = alpha;
				dxr->device->SetMaterial(&dxr->shadowMaterial);
			}
			dxr->renderSideLessBox(pos, obj->getGlobalOrientation(), dims, ((Actor *) obj)->getTextureNum());
		}
#else
		if (obj->getObjectType() == ObjectType(BumpPile, NULL, Grid))
		{

			// Render the slate this way because it's orientation is correct
			glColor4f(0.1f, 0.1f, 0.1f, NxMath::min(alpha, 0.14f));
			glBindTexture(GL_TEXTURE_2D, texMgr->getGLTextureId("Default"));
			ShapeVis::renderBox(pos, obj->getGlobalOrientation(), dims);
		}
		else if (!(obj->getObjectType() == ObjectType(BumpPile)))
		{
			// Render the shadow using the odd orientation of regular items
			glColor4f(0.1f, 0.1f, 0.1f, alpha);
			glBindTexture(GL_TEXTURE_2D, ((Actor *) obj)->getTextureNum());
			ShapeVis::renderSideLessBox(pos, obj->getGlobalOrientation(), dims);
		}
#endif
	}
}

void RenderManager::renderObjects(vector<Renderable *> &curList)
{
#ifdef DXRENDER
	dxr->device->SetRenderState(D3DRS_ZWRITEENABLE, false);
	dxr->device->SetMaterial(&dxr->textureMaterial);
#else
	glDepthMask(GL_FALSE);
#endif
	uint sz = curList.size();

	// Loop through all objects in the scene and render them
	for (uint i = 0; i < sz; i++)
	{
		BumpObject *obj = (BumpObject *) curList[i];

		//Piles are responsible for drawing their own children, clusters follows suit
		if (  (curList[i]->getAlpha() > 0.0f && (!obj->getParent() || Finder->isActive())  ) || obj->isBumpObjectType(BumpPile) )
		{
			// Render the object as is, no other special effects
			curList[i]->onRender();
		}
	}

#ifdef DXRENDER
	dxr->device->SetRenderState(D3DRS_ZWRITEENABLE, true);
#else
	glDepthMask(GL_TRUE);
#endif
}

void RenderManager::prepareRenderSelectionHighlight()
{
#ifdef DXRENDER
	dxr->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	dxr->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	dxr->device->SetRenderState(D3DRS_ZENABLE, false);
#else
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	// Use a special texture for selection
	glBindTexture(GL_TEXTURE_2D, texMgr->getGLTextureId("icon.selection.alphaMask"));
#endif
}

void RenderManager::finalizeRenderSelectionHighlight()
{
#ifdef DXRENDER
	dxr->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	dxr->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	dxr->device->SetRenderState(D3DRS_ZENABLE, true);
#else
	glPopAttrib();
#endif
}

void RenderManager::renderSelectionHighlight(vector<Renderable *> &curList)
{
#ifdef DXRENDER
	D3DMATERIAL9 material = {0};
	IDirect3DTexture9 * texture = texMgr->getGLTextureId("icon.selection.alphaMask");
#endif
	uint sz = curList.size();

	// Loop through all items and figure out weather or not they are selected or being hovered over
	for (uint i = 0; i < sz; i++)
	{
		BumpObject *obj = (BumpObject *) curList[i];
		Actor *aData = obj->isBumpObjectType(BumpActor) ? (Actor *) obj : NULL;
		Pile *pData = obj->isBumpObjectType(BumpPile) ? (Pile *) obj : NULL;
		Pile *pParent = aData && aData->isParentType(BumpPile) ? (Pile *) aData->getParent() : NULL;
		Vec3 dims = obj->getDims();
		Vec3 pos = obj->getGlobalPosition();
		Mat33 ori = obj->getGlobalOrientation();

		// Ignore special cases
		if (!obj->isSelected() && !obj->isBeingHoveredOver() && !obj->isAnimating(FreshnessAnim)) continue;
		if (obj->isBeingHoveredOver() && !obj->isSourceValid()) continue;
		if (aData && aData->isActorType(Invisible)) continue;
		if (aData && aData->isActorType(Temporary)) continue;
		if (pData && pData->getPileState() != Stack && pData->getPileState() != Leaf && pData->getPileState() != Grid) continue;

		// skip highlights until the pile items are not animating
		if (pData && (pData->getPileState() == Stack))
		{
			if (pData->isAnimating(PoseAnim))
				continue;
		}

		// Pick the color		
		if (obj->isAnimating(FreshnessAnim))
		{
			// blend if necessary
			ColorVal blendWith(0);
			if (obj->isBeingHoveredOver()) 
			{
				blendWith = hoverColor;
			}
			else if (obj->isSelected())
			{
				blendWith = selColor;
			}

			float freshnessAlpha = softEase(obj->getFreshnessAlpha());
			float finalAlpha = (blendWith.bigEndian.a / 255.0f) + (freshnessAlpha * ((freshColor.bigEndian.a - blendWith.bigEndian.a) / 255.0f));
			Vec3 c1(freshColor.bigEndian.r, freshColor.bigEndian.g, freshColor.bigEndian.b);
			Vec3 c2(blendWith.bigEndian.r, blendWith.bigEndian.g, blendWith.bigEndian.b);
			Vec3 c = c2 + (freshnessAlpha * (c1 - c2));
#ifdef DXRENDER
			material.Diffuse = D3DXCOLOR(c.x / 255.0f, c.y / 255.0f, c.z / 255.0f, finalAlpha);
#else
			glColor4f(c.x / 255.0f, c.y / 255.0f, c.z / 255.0f, finalAlpha);
#endif
		}
		else if (obj->isBeingHoveredOver())
#ifdef DXRENDER
			material.Diffuse = D3DXCOLOR(hoverColor.bigEndian.r / 255.0f, hoverColor.bigEndian.g / 255.0f, hoverColor.bigEndian.b / 255.0f, hoverColor.bigEndian.a / 255.0f);
#else
			glColor4f(hoverColor.bigEndian.r / 255.0f, hoverColor.bigEndian.g / 255.0f, hoverColor.bigEndian.b / 255.0f, hoverColor.bigEndian.a / 255.0f);
#endif
		else if (obj->isSelected())
		{
			float expectedAlpha = selColor.bigEndian.a / 255.0f;
			if (pData && pData->getPileState() == Grid)			
				expectedAlpha = NxMath::min(obj->getAlpha(), expectedAlpha);
#ifdef DXRENDER
			material.Diffuse = D3DXCOLOR(selColor.bigEndian.r / 255.0f, selColor.bigEndian.g / 255.0f, selColor.bigEndian.b / 255.0f, expectedAlpha);
#else
			glColor4f(selColor.bigEndian.r / 255.0f, selColor.bigEndian.g / 255.0f, selColor.bigEndian.b / 255.0f, expectedAlpha);
#endif
		}

		if (aData)
		{
			// Special case for single Item
			if (pParent && pParent->getPileState() != LaidOut)
			{
				if (pParent->getPileState() == Leaf && pParent->getActiveLeafItem())
				{
					pos.y = pParent->getActiveLeafItem()->getGlobalPosition().y;
				}
				else
				{
					// For a gridded pile, draw the highlight on the face of the phantom actor
					pos = pParent->getFrontFacePlane().project(pos);
				}
#ifdef DXRENDER
				dxr->device->SetRenderState(D3DRS_ZENABLE, true);
#else
				glEnable(GL_DEPTH_TEST);	
#endif
			}
			else
			{
				pos.y -= dims.z;
#ifdef DXRENDER
				dxr->device->SetRenderState(D3DRS_ZENABLE, false);
#else 
				glDisable(GL_DEPTH_TEST);
#endif
			}

			float highlightBuffer = 1.125f;
			dims = Vec3(NxMath::max(dims.x * highlightBuffer, dims.x + 2), 
						NxMath::max(dims.y * highlightBuffer, dims.y + 2), 0.2f);
		}
		else if (pData)
		{
			if (pData->getNumItems() > 0)
			{
				// Use the orientation of the bottom most item
				ori = pData->getLastItem()->getGlobalOrientation();
			}

			// Special case for Piles
			float margin = 4.0f;
			pos.y -= dims.z;
			dims = Vec3(dims.x + margin, dims.y + margin, 1.0f);
		}

		// Geometry Rendering
#ifdef DXRENDER
		material.Ambient = D3DXCOLOR(1, 1, 1, 1);
		dxr->device->SetMaterial(&material);
		dxr->renderSideLessBox(pos, ori, dims, texture);
#else
		ShapeVis::renderSideLessBox(pos, ori, dims);
#endif
	}
}


bool RenderManager::initGL()
{
#ifndef DXRENDER
	int bpp = winOS->GetWindowBpp();

	// Create the Pixel Format
	pfd.nSize = sizeof (PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	if (winOS->IsWindowsVersionGreaterThanOrEqualTo(WindowsVista))
	{
		 pfd.dwFlags |= PFD_SUPPORT_COMPOSITION;
	}
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = bpp;
	pfd.cAlphaBits = (bpp == 32) ? 8 : 0;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;

	// Get the window Handle
	hWnd = winOS->GetWindowsHandle();
	if (!hWnd) return false;

	// Init the Device Context
	hDeviceContext = GetDC(hWnd);
	if (!hDeviceContext) return false;	

	// Set the pixel Format
	pixelFormat = ChoosePixelFormat(hDeviceContext, &pfd);
	SetPixelFormat(hDeviceContext, pixelFormat, &pfd);

	// Create the Render Context
	hRenderContext = wglCreateContext(hDeviceContext);
	if (!hRenderContext) return false;

	// Make the device context our rendering context
	wglMakeCurrent(hDeviceContext, hRenderContext);

	// Turn on GLEW
	glewInit();

	// Init VSync and Multisampling
	bool doMultisample = ((bpp < 32) || initMultiSample());
#endif

	if (initSceneRendering()) return true;
	return false;
}

#ifdef DXRENDER
#else
//Returns true if it the function returned without error
bool RenderManager::setVSync(bool enable)
{
	PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = NULL;
	QString extensions((char *) glGetString(GL_EXTENSIONS));

	// Check for VSync Support
	if (extensions.contains("WGL_EXT_swap_control"))
	{
		// Get address's of both functions and save them
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC) wglGetProcAddress("wglSwapIntervalEXT");

		// Set the VSync
		if(wglSwapIntervalEXT && wglSwapIntervalEXT(enable))
		{
			vSyncEnabled = enable;
			return true;
		}
	}

	return false; 
}

bool RenderManager::initMultiSample()
{
	int valid;
	UINT numFormats;
	float fAttributes[] = {0, 0};

	// check if there is a user override for the anti aliasing
	bool hasUserAAOverride = false;
	DWORD userDisableAA = winOS->getRegistryDwordValue("DisableAntiAliasing", hasUserAAOverride);
	if (hasUserAAOverride)
	{
		// this overrides both the settings.xml and the auto profiling result
		// disable anti aliasing if the user says so
		if (userDisableAA) return true;
	}
	else
	{
		// disable anti aliasing if the auto profiling says so
		if (winOS->getRegistryDwordValue("DisableAntiAliasing")) return true;

		// User disabled Multisampling, don't do anything else
		if (!GLOBAL(settings).useAntiAliasing) return true;
	}

	// Don't bother if Multisampling is turned off by hardware
	if (!GLEW_ARB_multisample) 
	{
		scnManager->isMultiSamplingSupported = false;
		return true;
	}

	// Get Our Pixel Format
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC) wglGetProcAddress("wglChoosePixelFormatARB");	
	if (!wglChoosePixelFormatARB) 
	{
		scnManager->isMultiSamplingSupported = false;
		return false;
	}

	// These Attributes Are The Bits We Want To Test For In Our Sample
	// Everything Is Pretty Standard, The Only One We Want To 
	// Really Focus On Is The SAMPLE BUFFERS ARB And WGL SAMPLES
	// These Two Are Going To Do The Main Testing For Whether Or Not
	// We Support Multisampling On This Hardware.
	int iAttributes[] =
	{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_ACCELERATION_ARB,	WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB,		32,
		WGL_ALPHA_BITS_ARB,		8,
		WGL_DEPTH_BITS_ARB,		16,
		WGL_STENCIL_BITS_ARB,	0,
		WGL_DOUBLE_BUFFER_ARB,	GL_TRUE,
		WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
		WGL_SAMPLES_ARB,		4,
		0, 0
	};
	// NOTE: we set the color bpp now
	iAttributes[7] = winOS->GetWindowBpp(); // WGL_COLOR_BITS_ARB

	// First We Check To See If We Can Get A Pixel Format For 4 Samples
	valid = wglChoosePixelFormatARB(hDeviceContext, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);

	// If We Returned True, And Our Format Count Is Less Than 1
	if (valid && numFormats < 1)
	{
		// Our Pixel Format With 4 Samples Failed, Test For 2 Samples
		iAttributes[19] = 2;
		valid = wglChoosePixelFormatARB(hDeviceContext, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);

		// No MultiSampling is available
		if (valid && numFormats < 1) 
		{
			scnManager->isMultiSamplingSupported = false;
			return false;
		}
	}

	// Recreate the BT window
	winOS->DestroyBTWindow();
	winOS->CreateBTWindow();

	// Recreate the Device Context
	hWnd = winOS->GetWindowsHandle();
	hDeviceContext = GetDC(hWnd);

	// MultiSampling is available!
	SetPixelFormat(hDeviceContext, pixelFormat, &pfd);

	hRenderContext = wglCreateContext(hDeviceContext);
	if (!hRenderContext) 
	{
		scnManager->isMultiSamplingSupported = false;
		setMultisamplingEnabled(false);
		return false;
	}

	// Apply the render context to the device context
	wglMakeCurrent(hDeviceContext, hRenderContext);
	multiSamplingEnabled = true;

	return true;
}
#endif

bool RenderManager::initSceneRendering()
{
	// Lighting, just position and direction, which is used for shadow
	_light0Position[0] = 0.0f;
	_light0Position[1] = 110.0f;
	_light0Position[2] = 0.0f;
	_light0Position[3] = 0.0f;
	_light0Direction[0] = 0.7f;
	_light0Direction[1] = -1.0f;
	_light0Direction[2] = 0.7f;
	_light0Direction[3] = 1.0f;

#ifndef DXRENDER
	// Set the clear color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);

	switch (GLOBAL(settings).visuals)
	{
	case LowVisuals:
		break;
	default:	// every other case {default, medium, high} visuals
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		break;
	}

	// we should be ensuring that all our normal vectors are normalized instead of relying on ogl to do so for us.
	// shoule NEVER use glEnable(GL_NORMALIZE)
	glEnable(GL_RESCALE_NORMAL);

	GLubyte halftone[] = {
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55}; 
	glPolygonStipple(halftone);

	// multisampling
	if (GLEW_ARB_multisample && rndrManager->isMultisamplingEnabled())
		glEnable(GL_MULTISAMPLE_ARB);
	else
		glDisable(GL_MULTISAMPLE_ARB);
#endif

	return true;
}

void RenderManager::invalidateRenderer()
{
	renderIsNeeded = true;
}

bool RenderManager::isRenderRequired()
{
	return renderIsNeeded;
}

vector<Renderable *> RenderManager::sortRenderOrder(vector<Renderable *> &curList)
{
	vector<Renderable *> regularPriority, lowPriority;

	// Loop through all the items in the list and sort them by priority
	for (uint i = 0; i < curList.size(); i++)
	{
		BumpObject *obj = (BumpObject *) curList[i];

		// Prioritize Actors over other Items
		if (obj->getObjectType() == ObjectType(BumpPile, NULL, Grid))
		{
			lowPriority.push_back(curList[i]);
		}else{
			regularPriority.push_back(curList[i]);
		}
	}

	return mergeVectors(regularPriority, lowPriority);
}

void RenderManager::onSize(int width, int height)
{
#ifdef DXRENDER
	dxr->onResize(width, height);
#else
	// Dont bother doing anything if we dont have the device
	if (hDeviceContext && hRenderContext)
	{
		// Resize based on new size
		glViewport(0, 0, width, height);
		cam->markglDataDirty();
	}
#endif
}

void RenderManager::destroy()
{
#ifdef DXRENDER
#else
	// Set The Current Active Rendering Context To Zero
	wglMakeCurrent(NULL, NULL);

	if (hRenderContext)
	{
		wglDeleteContext (hRenderContext);	
		hRenderContext = NULL;
	}

	if (hDeviceContext)
	{
		ReleaseDC(hWnd, hDeviceContext);	
		hDeviceContext = NULL;
	}
#endif
}

bool RenderManager::isMultisamplingEnabled() const
{
	return multiSamplingEnabled;
}

void RenderManager::setMultisamplingEnabled(bool state)
{
	winOS->setRegistryDwordValue("DisableAntiAliasing", (state ? 0 : 1));
	multiSamplingEnabled = state;

	if (state)
	{
		printUnique("RenderManager::setMultisamplingEnabled", QT_TR_NOOP("Anti-Aliasing has been enabled to improve the image quality.\nPlease restart to see the changes!"));
	}
	else
	{
		printUnique("RenderManager::setMultisamplingEnabled", QT_TR_NOOP("Anti-Aliasing has been disabled to improve your frame rate.\nPlease restart to see the changes!"));
	}
}

void RenderManager::onThemeChanged()
{
	LOG("RenderManager::onThemeChanged");
	hoverColor = themeManager->getValueAsColor("ui.icon.highlight.color.hover",ColorVal());
	selColor = themeManager->getValueAsColor("ui.icon.highlight.color.selection",ColorVal());
	freshColor = themeManager->getValueAsColor("ui.icon.highlight.color.freshness",ColorVal());
}
