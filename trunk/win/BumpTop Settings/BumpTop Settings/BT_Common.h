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

#include <wx/wx.h>
#include <wx/cmdline.h>
#include <wx/filepicker.h>
#include <wx/spinctrl.h>
#include <wx/notebook.h>
#include <wx/dir.h>
#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/hyperlink.h>
#include <wx/snglinst.h>
#include <wx/fontdlg.h>
#include <wx/fontenum.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/file.h>

#include <wx/stdpaths.h>
#include <wx/xrc/xmlres.h>
#include <wx/sysopt.h>

#include <json/value.h>
#include <json/reader.h>
#include <json/writer.h>

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>

#ifdef WIN32
	#include <shlobj.h>
	#include <shellapi.h>
#endif

using namespace std;