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

#include "BT_TexHelperManager.h"
#include "TexHelperResponse.h"

struct Pipe
{
	HANDLE out_w; //write handle to std out for child process
	HANDLE out_r; //read handle to std out for child process
	HANDLE in_w; //write handle to std in for child process
	HANDLE in_r; //read handle to std in for child process
};

TexHelperManager::~TexHelperManager()
{
	SAFE_DELETE_ARRAY(_pipes);
	SAFE_DELETE_ARRAY(_pipeBusy);
	
	if (_processInfos)
	{
		for (unsigned int i = 0; i < _processCount; i++)
			TerminateProcess(_processInfos[i].hProcess, 1);
	}
	SAFE_DELETE_ARRAY(_processInfos);
	SAFE_DELETE_ARRAY(_startupInfos);
}

TexHelperManager::TexHelperManager(QString processPath, unsigned int processCount)
{
	_processPath = processPath;
	_processCount = processCount;

	//SYSTEM_INFO sysInfo;
	//GetNativeSystemInfo(&sysInfo);
	//const unsigned int MINIMUM_PROCESS_COUNT = 2;
	//_processCount = sysInfo.dwNumberOfProcessors - 1 < MINIMUM_PROCESS_COUNT ? MINIMUM_PROCESS_COUNT : sysInfo.dwNumberOfProcessors - 1;
		
	_processInfos = new PROCESS_INFORMATION [_processCount];
	_startupInfos = new STARTUPINFO [_processCount];
	_pipes = new Pipe [_processCount];
	_pipeBusy = new bool [_processCount];
	
	for (unsigned int i = 0; i < _processCount; i++)
	{
		_pipeBusy[i] = false;

		SECURITY_ATTRIBUTES sa = {0}; 
		sa.nLength = sizeof(SECURITY_ATTRIBUTES); 
		sa.bInheritHandle = true; 
		sa.lpSecurityDescriptor = NULL; 

		if (!CreatePipe(&_pipes[i].out_r, &_pipes[i].out_w, &sa, 0x40000)) // Create a pipe for the child process's STDOUT. 
			puts("Failed StdoutRd CreatePipe \n"); 
		if (!SetHandleInformation(_pipes[i].out_r, HANDLE_FLAG_INHERIT, 0)) // Ensure the read handle to the pipe for STDOUT is not inherited.
			puts("Failed Stdout SetHandleInformation \n"); 
		if (!CreatePipe(&_pipes[i].in_r, &_pipes[i].in_w, &sa, 1024)) // Create a pipe for the child process's STDIN. 
			puts("Stdin CreatePipe \n"); 
		if (!SetHandleInformation(_pipes[i].in_w, HANDLE_FLAG_INHERIT, 0)) // Ensure the write handle to the pipe for STDIN is not inherited. 
			puts("Stdin SetHandleInformation \n"); 

		restartPipe(i);
	}
}

// Returns true if response is read and valid, false if no response available and not valid
bool readResponse(HANDLE inHandle, TexHelperResponse & response)
{
	unsigned long read = 0, available = 0, left = 0;
	PeekNamedPipe(inHandle, NULL, 0, NULL, &available, &left);
	if (sizeof(response) <= available)
	{
		read = 0;
		bool result = ReadFile(inHandle, &response, sizeof(response), &read, NULL);
		result &= sizeof(response) == read;
		_ASSERT(result);
		_ASSERT(response.calculateCheckSum() == response.CheckSum);
		if (!result)
			puts("TexHelperManager readResponse failed");
		if (response.calculateCheckSum() != response.CheckSum)
			puts("TexHelperManager readResponse checksum failed");
		return result;
	}
	else
		return false;
}

bool clearPipe(HANDLE inHandle)
{
	unsigned long read = 0, available = 0, left = 0;
	PeekNamedPipe(inHandle, NULL, 0, NULL, &available, &left);
	if (0 < available)
	{
		unsigned char * buffer = new unsigned char [available];
		ReadFile(inHandle, buffer, available, &read, NULL);
		delete [] buffer;
		return true;
	}
	else
		return false;
}

bool TexHelperManager::isPipeBusy(unsigned int texhelperId) const
{
	_ASSERT(texhelperId < _processCount);
	return _pipeBusy[texhelperId];
}

void TexHelperManager::freePipe(unsigned int texhelperId)
{
	QMutexLocker mutexLocker(&_mutex);
	_ASSERT(texhelperId < _processCount);
	_ASSERT(_pipeBusy[texhelperId]);
	_pipeBusy[texhelperId] = false;
}

unsigned int TexHelperManager::getPipe()
{
	QMutexLocker mutexLocker(&_mutex);
	for (unsigned int i = 0; i< _processCount; i++) 
	{
		if (!_pipeBusy[i])
		{
			_pipeBusy[i] = true;
			return i;
		}
	}
	VASSERT(0, QString_NT("TexHelperManager::getPipe no available pipe, assigning new process id 0xFFFFFFFF (ignore)"));
	return 0xFFFFFFFF;
}

void TexHelperManager::restartPipe(unsigned int texhelperId)
{
	_ASSERT(texhelperId < _processCount);
	if (_processInfos[texhelperId].hProcess)
		TerminateProcess(_processInfos[texhelperId].hProcess, 1);

	clearPipe(_pipes[texhelperId].out_r);

	SECURITY_ATTRIBUTES sa = {0}; 
	sa.nLength = sizeof(SECURITY_ATTRIBUTES); 
	sa.bInheritHandle = true; 
	sa.lpSecurityDescriptor = NULL; 

	ZeroMemory(&_processInfos[texhelperId], sizeof(*_processInfos));
	ZeroMemory(&_startupInfos[texhelperId], sizeof(*_startupInfos));

	_startupInfos[texhelperId].cb = sizeof(STARTUPINFO); 
	_startupInfos[texhelperId].hStdError = NULL;
	_startupInfos[texhelperId].hStdOutput = _pipes[texhelperId].out_w;
	_startupInfos[texhelperId].hStdInput = _pipes[texhelperId].in_r;
	_startupInfos[texhelperId].dwFlags |= STARTF_USESTDHANDLES;

	bool success = CreateProcess((wchar_t *)_processPath.utf16(), 
		NULL,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		CREATE_NO_WINDOW | DETACHED_PROCESS,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&_startupInfos[texhelperId],  // STARTUPINFO pointer 
		&_processInfos[texhelperId]);  // receives PROCESS_INFORMATION 
	_ASSERT(success);
}

int TexHelperManager::texHelperOperation(QString command, unsigned int texhelperId, unsigned long long lifeTime, TexHelperResponse * outResponse, void ** outData)
{
	LARGE_INTEGER frequency, anchorTime, currentTime, maximumTime;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&anchorTime);
	maximumTime.QuadPart = anchorTime.QuadPart + lifeTime * frequency.QuadPart / 1000;
	_ASSERT(maximumTime.QuadPart > anchorTime.QuadPart);
	_ASSERT(texhelperId < _processCount);
	_ASSERT(_pipeBusy[texhelperId]);
	
	unsigned long readyRead = 0;
	TexHelperResponse readyResponse;
	ReadFile(_pipes[texhelperId].out_r, &readyResponse, sizeof(readyResponse), &readyRead, NULL); // Wait for ready
	_ASSERT(sizeof(readyResponse) == readyRead);
	_ASSERT(readyResponse.Ready && !readyResponse.Restart && !readyResponse.Finish);
	_ASSERT(readyResponse.calculateCheckSum() == readyResponse.CheckSum);

	if (readyResponse.calculateCheckSum() != readyResponse.CheckSum)
	{
		puts("TexHelperManager::texHelperOperation ready response checksum fail");
		restartPipe(texhelperId);
		readyRead = 0;
		readyResponse = TexHelperResponse();
		ReadFile(_pipes[texhelperId].out_r, &readyResponse, sizeof(readyResponse), &readyRead, NULL); // Wait for ready
		_ASSERT(sizeof(readyResponse) == readyRead);
		_ASSERT(readyResponse.calculateCheckSum() == readyResponse.CheckSum);
	}

	unsigned long written = 0; 
	WriteFile(_pipes[texhelperId].in_w, command.utf16(), command.size() * 2 + 2, &written, NULL);
	_ASSERT(command.size() * 2 + 2 == written);

	do
	{
		TexHelperResponse response;
		
		if (!readResponse(_pipes[texhelperId].out_r, response))
		{
			Sleep(10);
			QueryPerformanceCounter(&currentTime);
			continue;
		}

		_ASSERT(response.calculateCheckSum() == response.CheckSum);
		if (response.calculateCheckSum() != response.CheckSum)
		{
			puts("TexHelperManager::texHelperOperation finish response checksum fail");
			break; // Failed, restart pipe outside of loop.
		}
		
		if (outResponse)
			*outResponse = response;
		if (response.ImageData)
		{
			_ASSERT(response.Finish && !response.Fail);
			_ASSERT(outResponse);
			_ASSERT(outData);
			unsigned int imageDataSize = response.ImageWidth * response.ImageHeight * response.ImageBytePerPixel;
			unsigned char * imageDataBuffer = new unsigned char [imageDataSize];
			unsigned long read = 0;
			bool result = ReadFile(_pipes[texhelperId].out_r, imageDataBuffer, imageDataSize, &read, NULL);
			HRESULT lastError = GetLastError();
			_ASSERT(imageDataSize == read);
			if (outData && outResponse)
			{
				SAFE_DELETE_ARRAY(*outData);
				*outData = imageDataBuffer;
			}
			else
			{
				SAFE_DELETE_ARRAY(imageDataBuffer);
				_ASSERTE(!"TexHelperManager received unexpected pixel data");
			}
		}
				
		if (response.Restart)
		{
			restartPipe(texhelperId);
			response.Fail = true;
			wprintf(L"TexHelperManager pipe %u restarted %s \n", texhelperId, command.utf16());
		}

		return response.Fail;
	}
	while (currentTime.QuadPart <= maximumTime.QuadPart);

	restartPipe(texhelperId);
	wprintf(L"TexHelperManager pipe %u op time-out(%I64u)ms; restart pipe; %s \n", texhelperId, (currentTime.QuadPart - anchorTime.QuadPart) * 1000 / frequency.QuadPart, command.utf16());
	if (outResponse)
		*outResponse = TexHelperResponse(false, true, true);

	return 1;
}