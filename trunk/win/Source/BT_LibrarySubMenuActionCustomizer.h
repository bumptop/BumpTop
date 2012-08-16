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

#ifndef _BT_LIBRARYSUBMENUACTIONCUSTOMIZER_
#define _BT_LIBRARYSUBMENUACTIONCUSTOMIZER_

#include "BT_Common.h"
#include "BT_MenuActionCustomizer.h"

class Library;

class LibrarySubMenuActionCustomizer : public MenuActionCustomizer
{
	Q_DECLARE_TR_FUNCTIONS(LibrarySubMenuActionCustomizer)
	
	QSharedPointer<Library> _library;

public:
	LibrarySubMenuActionCustomizer(QSharedPointer<Library> library);
	void update(MenuAction *action);
	void execute(MenuAction *action);
};

#endif // _BT_LIBRARYSUBMENUACTIONCUSTOMIZER_
