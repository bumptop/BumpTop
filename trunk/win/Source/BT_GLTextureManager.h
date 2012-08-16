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

#ifndef BT_GLTEXTUREMANAGER
#define BT_GLTEXTUREMANAGER

//#include "BT_ThumbsDBReader.h"
#include "BT_Singleton.h"
#include "BT_ThemeManager.h"
#include "BT_GLTextureObject.h"

/*
 * Typedefs for the containers we're going to be using in the texture manager.
 */
struct TexturesListIndex;

struct TexturePriorityIndexCmp;
class ThreadableTextureUnit;

typedef priority_queue<TexturesListIndex, vector<TexturesListIndex>, TexturePriorityIndexCmp> PrioritizedTextureIndices;
typedef QHash<QString, GLTextureObject> TexturesList;
typedef vector<ThreadableTextureUnit *> TextureThreads;
//typedef QHash<QString, shared_ptr<ThumbsDBReader> > ThumbsDbs;

/*
 * Texture index wrapper class for comparison in a priority queue.
 */
struct TexturesListIndex
{
	TexturesList * texturesList;
	QString key;

public:
	TexturesListIndex(TexturesList * list, QString k)
		: texturesList(list), key(k)
	{}
	TexturesListIndex(const TexturesListIndex& other)
		: texturesList(other.texturesList), key(other.key)
	{}
	void operator=(const TexturesListIndex& other)
	{
		texturesList = other.texturesList;
		key = other.key;
	}
};
struct TexturePriorityIndexCmp
{
	bool operator()(const TexturesListIndex& t1, const TexturesListIndex& t2) const
	{
		TexturesList::const_iterator iter1 = t1.texturesList->find(t1.key);
		TexturesList::const_iterator iter2 = t2.texturesList->find(t2.key);
		assert(iter1 != t1.texturesList->end());
		assert(iter2 != t2.texturesList->end());
		GLTextureLoadPriority p1 = iter1.value().srcPriority;
		GLTextureLoadPriority p2 = iter2.value().srcPriority;
		if (p1 == p2)
			return iter1.value().srcDetail > iter2.value().srcDetail;
		else
			return p1 < p2;
	}
};

struct TexHelperResponse;
class TexHelperManager;
/*
 * Manages the set of all textures, including those that have loaded, are loading, or queued to load.
 */
class GLTextureManager : public ThemeEventHandler
{
	Q_DECLARE_TR_FUNCTIONS(GLTextureManager)

	weak_ptr<ThemeManager> themeManagerRef;
	
	static const float frameThicknessPercent;
	static const QColor frameBorderColor;

	// manager vars
	QDir _textureDirectory;
	QDir _cacheDirectory;
#ifdef DXRENDER
	IDirect3DTexture9 * _defaultGLTextureId;
#else
	unsigned int _defaultGLTextureId;
#endif
	bool _suspendLoading;
	const int _maxNumLoadingThreads;

	// the main list of textures, and the data structures involved in managing
	// this list
	TexturesList _textures;
	PrioritizedTextureIndices _imagesQueued;
	PrioritizedTextureIndices _texturesQueued;
	TextureThreads _loadingThreads;
	TextureThreads _invalidatedLoadingThreads;
	TexHelperManager * _texHelperManager;

	// posted textures from other threads, to be merged with the main list at 
	// first chance
	mutable QMutex _postedTexturesMutex;
	TexturesList _postedTextures;

	// devil IL mutex, must be locked before any il calls are made
	mutable QMutex _ilMutex;

	// TODO: reimplement BT_ThumbsDBReader
	// win32, thumbs db list
	//mutable QMutex _thumbsDbsMutex;
	//ThumbsDbs _thumbsDbs;

private:
	// helper funcs
	void initGLParams();
	void removeFromPrioritizedTextureIndices(PrioritizedTextureIndices& queue, QString key);
	void updateTextureWithILImageId(GLTextureObject& obj, int imgId);
	
	// Texture modification functions
	void applyFrameBorder(QImage& image, float thicknessPercent, QColor frameColor);

	// queues up all the bumptop system images for loading
	void queueBumpTopTextures();

	// loads the image into a devil IL image
	int loadImageThread(GLTextureObject obj);
	int loadFileIcon(const GLTextureObject& obj);
	int loadSampledImage(const GLTextureObject& obj);
	int loadHiResImage(QString loadPath);

	// ensure that the different files are available
	int createTexHelperProcess(QString texHelperAppStr, QString commandLine, unsigned int texhelperId, int lifetime, TexHelperResponse * outResponse, void ** outData);
	QString ensureCompressedImage(const GLTextureObject& obj);
	QString ensureThumbnail(const GLTextureObject& obj, bool compressed);

	QString generateCacheFilePath(const QString& filename, int imageDim);
	QString generateCacheFilePath(const QString& filePath, int imageDim, const QString& extension);

	void killLoadingThreads();

	// Singleton
	friend class Singleton<GLTextureManager>;
	GLTextureManager();

public:
	~GLTextureManager();
	bool initIL();
	bool init();

	bool isLoadingSuspended() const;

	// il mutex locks
	void lockIlMutex();
	void unlockIlMutex();

	bool loadTexture(GLTextureObject& obj);
	bool loadPersistentTexture(GLTextureObject& obj);
	bool postTexture(GLTextureObject& obj);
	bool loadBlockingTexture(GLTextureObject& obj, bool loadMipMaps, bool freeImageData);
	bool loadQueuedTexture(GLTextureObject& objectOut);
	IDirect3DTexture9* loadDefaultTexture();
	bool hasTexture(QString key);
	void deleteTexture(QString key);

	inline bool hasTexturesOfState(GLTextureState detail) const;
	inline bool hasTextureDetail(QString key, unsigned int detail) const;
	inline bool isTextureState(QString key, unsigned int state) const;
#ifdef DXRENDER
	inline IDirect3DTexture9 * getGLTextureId(const QString & key) const;
#else
	inline unsigned int getGLTextureId(const QString & key) const;
#endif
	inline bool getTextureAlpha(QString key) const;
	inline Vec3 getTextureDims(QString key) const;
	inline Vec3 getTexturePow2Dims(QString key) const;

	void scaleToPowerOfTwo();
	
	inline QString getTexturePathFromId(QString key) const;
	bool swapLoadedTextureKeys(const QString& key1, const QString& key2);

	// bool pushDetail(QString key, unsigned int detail);
	// bool popDetail(QString key, unsigned int detail);

	// win32, thumbsdb
	bool hasWinThumbnail(QString path);
	void appendWinThumbnailSearchPath(QString path);
	void removeWinThumbnailSearchPath(QString path);

	// XXX: shouldn't really be a part of the Texture Manager...
	void takeScreenshot(QString filePathOut) const;
	QString scaleFacebookImage(QString filePath);
	bool cropPhoto(QString srcPath, QString destPath, float top_crop_pct, float right_crop_pct, float bottom_crop_pct, float left_crop_pct);

	void onTimer();
	virtual void onThemeChanged();
	void onPowerSuspend();
	void onPowerResume();
	void setSuspendLoading(bool val);

	void deleteNonPersistentTextures();
	
};

// -----------------------------------------------------------------------------

#include "BT_GLTextureManager.inl"

// -----------------------------------------------------------------------------

/*
 * Free floating helpers
 */
bool isPowerOfTwo(int i);
unsigned int closestPowerOfTwo(int i);
unsigned int nextPowerOfTwo(int i);

// -----------------------------------------------------------------------------
#define texMgr Singleton<GLTextureManager>::getInstance()
// -----------------------------------------------------------------------------

#endif // BT_GLTEXTUREMANAGER
