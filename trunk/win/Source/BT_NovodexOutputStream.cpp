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
#include "BT_DialogManager.h"
#include "BT_NovodexOutputStream.h"
#include "BT_Util.h"

/*
void NovodexOutputStream::reportError(NxErrorCode code, const char * message, const char *file, int line)
{
	if (IsDebuggerPresent())
		__debugbreak();
	QString msg = QString("Novodex Error: %1, %2, %3 [%4]\n").arg((int)code).arg(message).arg(file).arg(line);
	dlgManager->clearState();
	dlgManager->setPrompt(msg);
	dlgManager->promptDialog(DialogCaptionOnly);
}

NxAssertResponse NovodexOutputStream::reportAssertViolation(const char * message, const char *file, int line)
{
	if (IsDebuggerPresent())
		__debugbreak();
	QString msg = QString("Novodex Assert: %1, %2 [%3]\n").arg(message).arg(file).arg(line);
	dlgManager->clearState();
	dlgManager->setPrompt(msg);
	dlgManager->promptDialog(DialogCaptionOnly);
	return NX_AR_CONTINUE;
}

void NovodexOutputStream::print(const char * message)
{
	consoleWrite(QString("Novodex Output: %1\n").arg(message));
}
*/