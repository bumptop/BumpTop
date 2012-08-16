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

#include "BT_WindowsHelper.h"
#include "BT_VistaFileOperation.h"

LPITEMIDLIST getAbsolutePidlFromAbsFilePath(LPWSTR path)
{
	LPITEMIDLIST pidl = NULL;
	LPITEMIDLIST pidlAbsolute = NULL;

	// namespace extension root (desktop) for parsing path
	LPSHELLFOLDER psfDesktop = NULL;
	if (FAILED(SHGetDesktopFolder(&psfDesktop)))
		return pidl;

	// parse path for absolute PIDL
	if (FAILED(psfDesktop->ParseDisplayName(NULL, NULL, path, NULL, &pidl, NULL)))
		return pidl;

	// get the absolute pidl, do not need to free pidlTemp
	pidlAbsolute = ILClone(pidl);

	// cleanup
	CoTaskMemFree(pidl);
	psfDesktop->Release();

	return pidlAbsolute;
}

bool moveFilesVistaHelper(QList<LPWSTR>& src, LPWSTR destFolder, QList<FileOperationResult>& results)
{
	VistaFileOperation fileOp;
	return fileOp.moveFiles(src, destFolder, results);
}