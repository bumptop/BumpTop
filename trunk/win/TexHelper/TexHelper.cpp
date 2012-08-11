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
#include "TexHelperResources.cpp"
#include "TexHelperResponse.h"

#define IL_ASSERT_NO_ERROR() {ILenum IL_ERROR_CODE = ilGetError(); if (IL_NO_ERROR != IL_ERROR_CODE) \
								{ if (!outHandle) printf("IL_ERROR 0x%.4X %s(%d) \n", IL_ERROR_CODE, __FILE__, __LINE__); \
								_ASSERT(false); } }

HANDLE inHandle = NULL, outHandle = NULL;

void writeResponse(TexHelperResponse & response)
{
	response.CheckSum = response.calculateCheckSum();

	if (!outHandle)
		return;
	unsigned long written = 0;
	bool result = WriteFile(outHandle, &response, sizeof(response), &written, NULL);
	HRESULT hr =  GetLastError();
	if (FAILED(hr))
	{
		_ASSERT(ERROR_BROKEN_PIPE == hr || ERROR_NO_DATA == hr); // error codes from parent closing pipe
		ExitProcess(1); // Parent process will recreate if necessary
	}
	_ASSERT(result && written == sizeof(response));
}

bool extractFlagParamQuoted(QString commandLine, QString flag, QString& paramOut)
{
	int pos = commandLine.indexOf(flag);
	if (pos > -1)
	{
		int quotPos = commandLine.indexOf("\"", pos + 1);
		if (quotPos > -1)
		{
			int endQuotPos = commandLine.indexOf("\"", quotPos + 1);
			if (endQuotPos > -1)
			{
				paramOut = commandLine.mid(quotPos+1, endQuotPos-quotPos-1).toLower();
				return true;
			}
		}
	} 
	return false;
}

bool extractFlagParamRaw(QString commandLine, QString flag, QString& paramOut)
{
	int pos = commandLine.indexOf(flag);
	if (pos > -1)
	{
		int offset = pos + flag.size();
		int spacePos = commandLine.indexOf(" ", offset);
		if (spacePos == -1)
			commandLine.size();

		paramOut = commandLine.mid(offset, spacePos-offset).toLower();
		return true;
	}
	return false;
}

int closestPowerOfTwo( int i )
{
	// if i's a power of two, then return i
	if (!(i & (i - 1)) && i)
		return i;

	int j = 1;
	int prev = 1;
	while (j < i)
	{
		prev = j;
		j <<= 1;
	}
	if (i-prev < j-i)
		return prev;
	else
		return j;
}

QString native(const QDir& dir)
{
	return QDir::toNativeSeparators(dir.absolutePath());
}

QDir parent(const QFileInfo& path)
{
	return path.dir();
}

void scaleAspectToMaxDimensionsOf(int maxDim)
{
	IL_ASSERT_NO_ERROR()
	int width = ilGetInteger(IL_IMAGE_WIDTH);
	int height = ilGetInteger(IL_IMAGE_HEIGHT);
	float aspect = (float) width / height;
	int closestPowTwo = closestPowerOfTwo(max(width, height));
	if (closestPowTwo != maxDim)
	{
		IL_ASSERT_NO_ERROR()
		closestPowTwo = min(closestPowTwo, maxDim);
		int newWidth = width >= height ? closestPowTwo : closestPowTwo * aspect;
		int newHeight = width > height ? closestPowTwo / aspect :  closestPowTwo;
		// choose appropriate scaling algorithm
		// iluImageParameter(ILU_FILTER, ILU_SCALE_TRIANGLE);
		iluScale(newWidth, newHeight, ilGetInteger(IL_IMAGE_DEPTH));
		IL_ASSERT_NO_ERROR()
	}
}

bool getMonitorDimensions(int& width, int& height)
{
	// find the bumptop window
	HWND bumpTopHwnd = FindWindow(L"BumpTop", NULL);

	// find the dimensions of the monitor that it's on
	HMONITOR wrMon = MonitorFromWindow(bumpTopHwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFOEX mi;
	mi.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(wrMon, &mi);

	width = mi.rcWork.right - mi.rcWork.left;
	height = mi.rcWork.bottom - mi.rcWork.top;
	return true;
}

DWORD NullExceptionFilter(EXCEPTION_POINTERS *pointers, DWORD dwException)
{
	return EXCEPTION_EXECUTE_HANDLER;
}

HRESULT (WINAPI* pfnSHCreateItemFromParsingName)(PCWSTR, IBindCtx *, REFIID, void **) = NULL;
HMODULE shell32Module = NULL;
IThumbnailCache * thumbnailCache = NULL;

void InitializeShell()
{
	HRESULT hr;

	shell32Module = GetModuleHandle(L"shell32");
	_ASSERT(shell32Module);

	(FARPROC&) pfnSHCreateItemFromParsingName = GetProcAddress(shell32Module, "SHCreateItemFromParsingName");
	_ASSERT(pfnSHCreateItemFromParsingName);

	hr = CoCreateInstance(CLSID_LocalThumbnailCache, NULL, CLSCTX_INPROC_SERVER, IID_IThumbnailCache, (LPVOID *) &(thumbnailCache));
	_ASSERT(SUCCEEDED(hr));
}

HBITMAP ExtractThumbnail_Vista(QString texturePath, IShellItem ** psiOut, ISharedBitmap ** psbOut)
{
	// ensure valid outputs
	if (!psiOut) return NULL; else (*psiOut) = NULL;
	if (!psbOut) return NULL; else (*psbOut) = NULL;

	// try and create an instance of the IThumbnailCache interface
	HRESULT hr;
	HBITMAP hbitmap;
	bool bitmapExtracted = false;	
	
	if (!shell32Module)
		InitializeShell();
	if (thumbnailCache)
	{
		// try and get the IShellItem interface for the specified item
		if (shell32Module)
		{
			if (pfnSHCreateItemFromParsingName)
			{
				GUID shellItemID = {0x7e9fb0d3, 0x919f, 0x4307, {0xab, 0x2e, 0x9b,0x18, 0x60, 0x31, 0x0c, 0x93}};
				hr = pfnSHCreateItemFromParsingName((LPCWSTR) texturePath.utf16(), NULL, shellItemID, (void **) &(*psiOut));

				if (SUCCEEDED(hr))
				{
					// try and extract the thumbnail from the item
					WTS_CACHEFLAGS cacheFlags;
					WTS_THUMBNAILID thumbnailId;

					// NOTE: using WTS_EXTRACT: Explorer extracts the thumbnail if it is not cached
					hr = thumbnailCache->GetThumbnail((*psiOut), 256, WTS_EXTRACT, &(*psbOut), &cacheFlags, &thumbnailId);

					if (SUCCEEDED(hr))
					{
						// try and extract the HBITMAP from the ISharedBitmap and load it into open gl
						if (SUCCEEDED((*psbOut)->GetSharedBitmap(&hbitmap)))
							return hbitmap;
						SAFE_RELEASE(*psbOut);
					}
					SAFE_RELEASE(*psiOut);
				}
			}
		}
	}
	return NULL;
}

bool isInvalidBitmap(const BITMAP& bm)
{
	const unsigned char minAlphaColor = (1 << 5) - 1;	// 31
	UINT height = bm.bmHeight;
	UINT width = bm.bmWidth;
	const unsigned char * scan0 = (const unsigned char *) bm.bmBits;
	
	if ((height > 0) && (width > 0) && (bm.bmBitsPixel == 32))
	{
		unsigned int x = height / 2;
		unsigned int y = width / 2;

		// check the horizontal cross-section
		const unsigned char * horizontalStart = scan0 + 4 * width * (height / 2) + 3;
		const unsigned char * horizontalEnd = horizontalStart + width * 4;
		for (const unsigned char * i = horizontalStart; i < horizontalEnd; i += 4)
		{
			if (*i > minAlphaColor)
				return false;
		}

		// check the vertical cross-section
		const unsigned char * verticalStart = scan0 + (width / 2) * 4 + 3;
		const unsigned char * verticalEnd = verticalStart + width * 4 * height;
		for (const unsigned char * i = verticalStart; i < verticalEnd; i += width * 4)
		{
			if (*i > minAlphaColor)
				return false;
		}

		return true;
	}
	else
	{
		assert(false);
	}
	return true;
}

int operationVistaExtract(QString file, QString cacheDirectory, QString thumbnailFile, bool & extractThumbnailSucceeded)
{
	extractThumbnailSucceeded = false;
	
	//uint ilImageId = -1;
	IShellItem * psi = NULL;
	ISharedBitmap * psb = NULL;
	HBITMAP hbitmap = NULL;
	int imgId = 0;

	//	 NOTE: some thumbnailers such as Adobe Acrobat try and generate thumbnails asynchronously
	//	 which means that we can't extract the vista thumbnail immediately (it still returns
	//	 a valid image).  For now, we will keep polling for the thumbnail for a period of time
	//	 until we get it.  To save the alpha check for all thumbnails, we will only limit the
	//	 extra check to those formats that we know have problems.
	const unsigned int maxTryCount = 8;
	const unsigned int sleepDuration = 250;
	unsigned int tryCount = 0;
	bool isPossibleBadThumbnailExtracter = file.endsWith(".pdf");
	while (tryCount < maxTryCount)
	{
		hbitmap = ExtractThumbnail_Vista(file, &psi, &psb);
		if (!hbitmap)
			break;

		// check if this is a valid hbitmap
		if (isPossibleBadThumbnailExtracter)
		{
			BITMAP bm = {0};
			GetObject(hbitmap, sizeof(BITMAP), &bm);
			if (isInvalidBitmap(bm)) 
				Sleep(sleepDuration); // keep sleeping
			else 
				break; // we've (finally) found a good bitmap
		}
		else // this is a valid bitmap
			break;
		
		// release the com references and try and load the bitmap again
		SAFE_RELEASE(psb);
		SAFE_RELEASE(psi);
		++tryCount;
	}
	// try and load the actual hbitmap into an il image
	if (hbitmap)
	{
		IL_ASSERT_NO_ERROR()
		imgId = ilGenImage();
		ilBindImage(imgId);
		if (ilutSetHBitmap(hbitmap))
		{
			//ilImageId = imgId;
			IL_ASSERT_NO_ERROR()
			QString thumbnailFilePath = cacheDirectory + thumbnailFile;
			QFileInfo tfpInfo(thumbnailFilePath);
			QDir().mkpath(native(parent(tfpInfo)));

			int format = ilGetInteger(IL_IMAGE_FORMAT);
			if (format == IL_RGB ||	format == IL_BGR) // use DXT1 otherwise
				ilSetInteger(IL_DXTC_FORMAT, IL_DXT1);
			else // use DXT5 for images with alpha or image types we don't know much about
				ilSetInteger(IL_DXTC_FORMAT, IL_DXT5);
			IL_ASSERT_NO_ERROR()
			// Save the compressed thumbnail
			
			if (outHandle)
			{
				TexHelperResponse response(false, true, false);

				response.ImageWidth = ilGetInteger(IL_IMAGE_WIDTH);
				response.ImageHeight = ilGetInteger(IL_IMAGE_HEIGHT);
				response.ImageBytePerPixel = ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);
				response.ImageFormat = ilGetInteger(IL_IMAGE_FORMAT);
				response.ImageData = true;

				unsigned int imageDataSize = response.ImageWidth * response.ImageHeight * response.ImageBytePerPixel;
				ILubyte * imageData = ilGetData();
				IL_ASSERT_NO_ERROR();

				writeResponse(response); //Write pixel data first, save to disk later.
				//TexHelperManager will return with response, and loadSampledImage will load image, while texhelper saves
				//DDS and does compression. Next texHelperOperation will wait for the ready message.

				unsigned long written = 0;
				bool result = WriteFile(outHandle, imageData, imageDataSize, &written, NULL);
				HRESULT lastError = GetLastError();

				_ASSERT(result && written == imageDataSize);
			}

			if (ilSave(IL_DDS, (ILstring) thumbnailFilePath.utf16()))
			{
				extractThumbnailSucceeded = true;
			}
			else
			{
				IL_ASSERT_NO_ERROR()
			}
		}						

		// we need to release the valid bitmap ref here, since it wasn't done above
		SAFE_RELEASE(psb);
		SAFE_RELEASE(psi);
		if (imgId)
			ilDeleteImage(imgId);
	}
	else
		writeResponse(TexHelperResponse(false, true, true));
		
	ILenum Error;
	Error = ilGetError();			
	return Error; // Supposed to exit program if vista extract. It's 0 because DevIL is not used.
}

int onMain(const unsigned short * cmdLine)
{
	QString commandLine = QString::fromUtf16(cmdLine).trimmed();
	QString shortFileFlag("-f\"");
	QString file;
	QString shortCacheDirFlag("-d\"");
	QString cacheDirectory;
	QString shortThumbnailFileFlag("-t\"");
	QString thumbnailFile;
	QString shortCompressFileFlag("-c\"");
	QString compressFile;
	QString shortThumbnailLengthFlag(" -b");
	QString thumbnailLength;
	QString shortImageLengthFlag(" -x");
	QString imageLength;
	QString freeVersionFlag(" -e");
	QString freeVersion;
	QString disableLibSquishFlag(" -q\"");

	QString vistaThumbnailExtractFlag (" -v");
	QString vistaThumbnailExtract;
	QString dummyStr;

	// NOTE: only one of the thumbnail/compression flags needs to be set
	bool cf = extractFlagParamQuoted(commandLine, shortCompressFileFlag, compressFile) && 
		extractFlagParamRaw(commandLine, shortImageLengthFlag, imageLength);
	bool tf = extractFlagParamQuoted(commandLine, shortThumbnailFileFlag, thumbnailFile) && 
		extractFlagParamRaw(commandLine, shortThumbnailLengthFlag, thumbnailLength);
	bool vistaExtract = extractFlagParamQuoted(commandLine, vistaThumbnailExtractFlag, vistaThumbnailExtract);
	bool validOp = extractFlagParamQuoted(commandLine, shortFileFlag, file) &&
		extractFlagParamQuoted(commandLine, shortCacheDirFlag, cacheDirectory);

	// Make sure the file and cache directory are non-empty
	validOp = validOp && !file.isEmpty() && !cacheDirectory.isEmpty();

	// Check if one of the flags is true
	validOp = validOp && (tf || cf || vistaExtract);

	// Check that the parameters for at least one of the flags is true;
	validOp = validOp && ((!thumbnailFile.isEmpty() && !thumbnailLength.isEmpty()) || (!compressFile.isEmpty() && !imageLength.isEmpty()) || vistaExtract);

	bool isFreeVersion = extractFlagParamQuoted(commandLine, freeVersionFlag, freeVersion);
	bool disableLibSquish = extractFlagParamQuoted(commandLine, disableLibSquishFlag, dummyStr);
	bool compressionSucceeded = false;
	bool thumbnailingSucceeded = false;
	bool extractThumbnailSucceeded = false;
	if (validOp)
	{
		if (vistaExtract)
		{
			int result = operationVistaExtract(file, cacheDirectory, thumbnailFile, extractThumbnailSucceeded);
			return result; // Supposed to exit program if vista extract
		}

		int thumbnailDims = thumbnailLength.toInt();
		int imageDims = imageLength.toInt();

		if (!disableLibSquish)
			ilEnable(IL_SQUISH_COMPRESS);
		ilEnable(IL_FILE_OVERWRITE);

		IL_ASSERT_NO_ERROR()

		// try and load the image into memory
		ILuint ilImageId = ilGenImage();
		ilBindImage(ilImageId);

		IL_ASSERT_NO_ERROR();
		
		if (ilLoadImage((ILconst_string) file.utf16()))
		{
			IL_ASSERT_NO_ERROR()

			int format = ilGetInteger(IL_IMAGE_FORMAT);
			bool isCompressibleFormat = (format == IL_RGB) || (format == IL_BGR) ||	
				(format == IL_RGBA) || (format == IL_BGRA);

			// NOTE: without generating an OpenGL context, I dont' think we can query for texture compression support. 
			// For now, ASSUME that if the compression flag is set, that compression is supported
			if (!compressFile.isEmpty() && isCompressibleFormat)
			{
				QString compressFilePath = cacheDirectory + compressFile;
				QFileInfo cfpInfo(compressFilePath);

				// NOTE: bound the image dims by the dims of the working area
				int width = imageDims;
				int height = imageDims;
				getMonitorDimensions(width, height);
				imageDims = min(imageDims, max(width, height));

				// scale it down to image dims if it's larger than that
				scaleAspectToMaxDimensionsOf(imageDims);

				int format = ilGetInteger(IL_IMAGE_FORMAT);
				if (format == IL_RGB ||	format == IL_BGR) // use DXT1 otherwise
					ilSetInteger(IL_DXTC_FORMAT, IL_DXT1);
				else // use DXT5 for images with alpha or image types we don't know much about
					ilSetInteger(IL_DXTC_FORMAT, IL_DXT5);
				
				IL_ASSERT_NO_ERROR()

				QDir().mkpath(native(parent(cfpInfo)));
				if (ilSave(IL_DDS, (ILstring) compressFilePath.utf16()))
					compressionSucceeded = true;
			}
			else
			{
				compressionSucceeded = true;
			}

			// downsample for the thumbnail 
			if (!thumbnailFile.isEmpty())
			{
				QString thumbnailFilePath = cacheDirectory + thumbnailFile;
				QFileInfo tfpInfo(thumbnailFilePath);

				scaleAspectToMaxDimensionsOf(thumbnailDims);

				if (!thumbnailFile.isEmpty()) 
				{
					int width = ilGetInteger(IL_IMAGE_WIDTH);
					int height = ilGetInteger(IL_IMAGE_HEIGHT);
					int overlayWidth = min(width, height);
					if (isFreeVersion && overlayWidth > 10 && (file.endsWith(".psd") || file.endsWith(".pdd") || file.endsWith(".hdr") || file.endsWith(".jp2")))
					{
						// if we don't convert the image, the overlay shows up as a yellow square
						ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE); 

						IL_ASSERT_NO_ERROR()

						// load the overlay
						ILuint ilOverlayId = ilGenImage();
						ilBindImage(ilOverlayId);
						if (overlayWidth < 128)
							ilLoadL(IL_PNG, xml_res_file_0, (ILuint)xml_res_size_0);
						else if (overlayWidth < 256)
							ilLoadL(IL_PNG, xml_res_file_1, (ILuint)xml_res_size_1);
						else
							ilLoadL(IL_PNG, xml_res_file_2, (ILuint)xml_res_size_2);

						iluImageParameter(ILU_FILTER, ILU_SCALE_TRIANGLE);
						iluScale(overlayWidth, overlayWidth, 0);
						ilBindImage(ilImageId);

						IL_ASSERT_NO_ERROR()

						int y = (height - overlayWidth) / 2;
						int x = (width - overlayWidth) / 2;
						ilOverlayImage(ilOverlayId, x, y, 0);
					}
				}

				// NOTE: see about as to why we can't test for S3TC compression. Simply use the filename check for now
				// if (GLEW_EXT_texture_compression_s3tc)
				if (thumbnailFile.endsWith(".dds") && isCompressibleFormat)
				{
					if (format == IL_RGB ||	format == IL_BGR) // use DXT1 otherwise
						ilSetInteger(IL_DXTC_FORMAT, IL_DXT1);
					else // use DXT5 for images with alpha or image types we don't know much about
						ilSetInteger(IL_DXTC_FORMAT, IL_DXT5);
					
					IL_ASSERT_NO_ERROR()

					QDir().mkpath(native(parent(tfpInfo)));
					if (ilSave(IL_DDS, (ILstring) thumbnailFilePath.utf16()))
						thumbnailingSucceeded = true;
					else
						IL_ASSERT_NO_ERROR()
				}
				else
				{
					// set the jpeg compression to, say, 90%?
					ilHint(IL_COMPRESSION_HINT, IL_USE_COMPRESSION);
					ilSetInteger (IL_JPG_QUALITY, 90);
					QDir().mkpath(native(parent(tfpInfo)));
					if (ilSave(IL_JPG, (ILstring) thumbnailFilePath.utf16()))
						thumbnailingSucceeded = true;
					else
						IL_ASSERT_NO_ERROR()
				}

				IL_ASSERT_NO_ERROR()
			}

			// free the image
			ilDeleteImage(ilImageId);

			IL_ASSERT_NO_ERROR()

			// return result
			if ((compressFile.isEmpty() || compressionSucceeded) &&	(thumbnailFile.isEmpty() || thumbnailingSucceeded))
			{
				writeResponse(TexHelperResponse(false, true, false));
				return 0;
			}
		}
		else
		{
			if (ilImageId)
				ilDeleteImage(ilImageId);

			ILenum errorCode = ilGetError(); // This is reached often, due to bad or missing files
			while (IL_INVALID_EXTENSION == errorCode || IL_INVALID_FILE_HEADER == errorCode || IL_FILE_READ_ERROR == errorCode || IL_COULD_NOT_OPEN_FILE == errorCode)
				errorCode = ilGetError(); // IL_INVALID_FILE_HEADER is followed by many IL_FILE_READ_ERROR
						
			errorCode = ilGetError();
			_ASSERT(IL_NO_ERROR == errorCode);
		}
	}

	// return error
	writeResponse(TexHelperResponse(false, true, true));
	return 1;
}

int _tmain(int argc, _TCHAR* argv[])
{
	const wchar_t * originalCommandLine = GetCommandLine();
	const wchar_t * cmdLine = originalCommandLine;
	int cmdCount = 0;
	CommandLineToArgvW((wchar_t *)cmdLine, &cmdCount);
	int result = -1;
	
	// initialize DevIL
	ilInit();
	iluInit();
	ilutInit();

	OleInitialize(NULL);
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if (1 == cmdCount) // If started with no command line arguments, then loop and wait for command through pipe
	{
		outHandle = GetStdHandle(STD_OUTPUT_HANDLE); 
		inHandle = GetStdHandle(STD_INPUT_HANDLE); 
		writeResponse(TexHelperResponse(true, false, false));
	}

	do
	{
		TexHelperResponse response;

		if (1 == cmdCount) // Retrieve command from inHandle
		{
			wchar_t input [512] = {0};
			unsigned long read = 0;
			bool res = ReadFile(inHandle, input, sizeof(input), &read, NULL);
			if (!res)
				return 1;
			input[read/2 + 1] = 0;
			cmdLine = input;
		}
		
		__try
		{
			result = onMain((unsigned short *)cmdLine);
		}
		__except(NullExceptionFilter(GetExceptionInformation(), GetExceptionCode())) // libsquish may throw errors
		{
			result = 1;
			response = TexHelperResponse(false, true, true);
			response.Restart = true;
			writeResponse(response);
			ExitProcess(1);
		}

		IL_ASSERT_NO_ERROR()
		
		if (1 == cmdCount)
			writeResponse(TexHelperResponse(true, false, false)); // Write ready message, which must follow the finish message

	}
	while (1 == cmdCount); // If program started with no command line, then loop and wait for command

	CoUninitialize();
	OleUninitialize();

	return result;
}