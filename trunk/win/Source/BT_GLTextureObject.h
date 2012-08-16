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

#ifndef BT_GLTEXTUREOBJECT
#define BT_GLTEXTUREOBJECT

class BumpObject;

enum GLTextureState
{
	UnknownState,
	ImageQueued,
	ImageLoading,
	TextureQueued,
	TextureLoaded
};

enum GLTextureLoadPriority
{
	IdlePriority,
	LowPriority,
	NormalPriority,
	HighPriority,
	ImmediatePriority
};

enum GLTextureDetail
{
	FileIcon		= 1 << 0,
	SampledImage	= 1 << 1,	// aka. Thumbnail
	MidResImage		= 1 << 2,
	HiResImage		= 1 << 3,
	CompressedImage	= 1 << 4,
	NumTextureDetails = 1 << 5	// keep this up to date!
};

enum GLImageOperation
{
	NoAction	= 1 << 0,
	Load		= 1 << 1,
	Compress	= 1 << 2,
	Reload		= 1 << 3,
	Serialize	= 1 << 4,
	NoGLLoad	= 1 << 5
};

enum GLTextureOperationResult
{
	NoError,
	ImageLoadError,
	TextureLoadError,
	TextureLoadOutOfMemory
};

/*
* The handler interface to be implemented by classes which expect to be notified
* of the progress of a texture's loading/binding
*/
class TextureEventHandler
{
public:
	virtual void onTextureLoadComplete(QString textureKey) = 0;
	virtual void onTextureLoadError(QString textureKey) = 0;
	// virtual void onTextureBindComplete(const PrioritizedTexture& texture) = 0;
	// virtual void onTextureBindError(const PrioritizedTexture& texture) = 0;
};

/*
* Represents the texture data either in it's original image form, or as a
* loaded open gl texture.
*/
struct GLTextureData
{
	ILuint ilImageId;

#ifdef DXRENDER
	IDirect3DTexture9 * glTextureId;
#else
	GLuint glTextureId;
#endif

	Vec3 dimensions;
	Vec3 pow2Dimensions;
	bool alpha;
	bool isDXTCompressed;

public:
	GLTextureData();
	~GLTextureData();

	void freeImage();
	void freeTexture();
};

/*
* Represents a texture object in any given state
*/
struct GLTextureObject
{
	GLTextureState state;
	unsigned int operation;
	unsigned int texhelperId; // Which tex helper process to use, assigned by GLTextureManager::onTimer

	QString key;
	QString srcPath;
	unsigned int srcPathFileSize;	// only used for file paths
	GLTextureDetail srcDetail;
	GLTextureLoadPriority srcPriority;

	bool isAFile; // This is true if the texture object refers to an actual file, it is false if it is a virtual icon
	bool hasBorder;
	bool persistent;

	GLTextureOperationResult resultCode;
	shared_ptr<GLTextureData> resultTextureData;

	// overrides
	int thumbDimensions;
	int imageDimensions;

private:
	void resolveVisuals();

public:
	GLTextureObject();
	GLTextureObject( unsigned int op, QString texKey, QString texPath, GLTextureDetail detail, GLTextureLoadPriority priority, bool isAFile = true, bool border = false, bool persist = false);
};
struct GLTextureObjectPriorityCmp
{
	bool operator()(const GLTextureObject& p1, const GLTextureObject& p2) const
	{
		if (p1.srcPriority == p2.srcPriority)
			return p1.srcDetail > p2.srcDetail;
		else
			return p1.srcPriority < p2.srcPriority;
	}
};

#endif // BT_GLTEXTUREOBJECT
