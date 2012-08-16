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
#ifdef DXRENDER
#include "BT_DXRender.h"
#endif
#include "BT_GLTextureObject.h"
#include "BT_GLTextureManager.h"
#include "BT_RenderManager.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"

GLTextureData::GLTextureData()
: ilImageId(0)
, glTextureId(0)
, dimensions(0.0f)
, pow2Dimensions(0.0f)
, alpha(false)
, isDXTCompressed(false)
{}

GLTextureData::~GLTextureData()
{
	freeImage();
	freeTexture();
}

void GLTextureData::freeImage()
{
	// don't bother freeing memory if we are shutting down
	if (texMgr->isLoadingSuspended())
		return;

	// we MUST lock the il mutex (via the texture manager)
	// for any il call
	if (ilImageId)
	{
		texMgr->lockIlMutex();
		ilDeleteImage(ilImageId);
		texMgr->unlockIlMutex();
		ilImageId = 0;
	}
}

void GLTextureData::freeTexture()
{
#ifdef DXRENDER
	SAFE_RELEASE(glTextureId);
#else
	if (glTextureId)
	{
		// this should only occur on the main thread, so there is no
		// need to lock on any mutexes
		glDeleteTextures(1, &glTextureId);
		glTextureId = 0;
	}
#endif
}

// ----------------------------------------------------------------------------

GLTextureObject::GLTextureObject()
: state(UnknownState)
, texhelperId(0xFFFFFFFF)
, operation(0)
, srcDetail(FileIcon)
, srcPriority(IdlePriority)
, resultCode(NoError)
, srcPathFileSize(0)
, isAFile(true)
, hasBorder(false)
, persistent(false)
{
	resolveVisuals();
}

GLTextureObject::GLTextureObject( unsigned int op, QString texKey, QString texPath,
								 GLTextureDetail detail, GLTextureLoadPriority priority, bool isAFile, bool border, bool persist )
: state(UnknownState)
, texhelperId(0xFFFFFFFF)
, operation(op)
, key(texKey)
, srcPath(texPath)
, srcDetail(detail)
, srcPriority(priority)
, resultCode(NoError)
, srcPathFileSize(0)
, isAFile(isAFile)
, hasBorder(border)
, persistent(persist)
{
	assert(!(detail & CompressedImage));
	assert(!(detail & NumTextureDetails));

	resolveVisuals();
}

void GLTextureObject::resolveVisuals()
{
	static int maxTexDims = 0;
	if (maxTexDims <= 0)
	{
#ifdef DXRENDER
		maxTexDims = dxr->caps.MaxTextureWidth;
#else
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexDims);
#endif
	}

	switch (GLOBAL(settings).visuals)
	{
	case LowVisuals:
		{
			// NOTE: compression with minimum/acceptable texture dimensions
			imageDimensions = NxMath::min(maxTexDims, 1024);
			thumbDimensions = NxMath::max(imageDimensions / 8, 128);
		}
		break;
	case DefaultVisuals:
	case MediumVisuals:
		{
			// NOTE: compression, but with relatively decent texture dimensions
			imageDimensions = NxMath::min(maxTexDims, 2048);
			thumbDimensions = NxMath::max(imageDimensions / 4, 512);
		}
		break;
	case HighVisuals:
		{
			// NOTE: no compression with high-visuals, but still bounded by GL_MAX_TEXTURE_SIZE
			operation &= ~Compress;
			imageDimensions = NxMath::min(maxTexDims, 2048);
			thumbDimensions = NxMath::max(imageDimensions / 2, 1024);
		}
		break;
	default:
		assert(false);
		break;
	}
}

// ----------------------------------------------------------------------------