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

#pragma once

struct Pipe;
struct TexHelperResponse;

//http://msdn.microsoft.com/en-us/library/ms682499(VS.85).aspx
class TexHelperManager
{
	QMutex _mutex;
	QString _processPath;
	unsigned int _processCount;
	PROCESS_INFORMATION * _processInfos;
	STARTUPINFO * _startupInfos;
	Pipe * _pipes;
	bool * _pipeBusy;
	
	TexHelperManager();

public:
	TexHelperManager(QString processPath, unsigned int processCount);
	~TexHelperManager();
	void freePipe(unsigned int texhelperId);
	unsigned int getPipe(); // Returns 0xFFFFFFFF if no free pipe, else sets a free pipe busy and returns its id
	void restartPipe(unsigned int texhelperId);
	bool isPipeBusy(unsigned int texhelperId) const;
	
	// If outResponse indicates ImageData, caller needs to delete [] outData.
	int texHelperOperation(QString command, unsigned int texhelperId, unsigned long long lifeTime, TexHelperResponse * outResponse, void ** outData);
};
