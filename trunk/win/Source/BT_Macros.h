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

#ifndef _BT_MACROS_
#define _BT_MACROS_

// BumpTop Defines
#define gray3(grayVal) grayVal, grayVal, grayVal
#define white3 1.0f, 1.0f, 1.0f
#define black3 0.0f, 0.0f, 0.0f
#define red3 1.0, 0.0, 0.0
#define green3 0.0, 1.0, 0.0
#define blue3 0.0, 0.0, 1.0
#define yellow3 1.0, 1.0, 0.0
#define turquosie3 0.0, 1.0, 0.0
#define purple3 1.0, 0.0, 1.0
#define orange3 1.0, 0.5, 1.0
#define  GLUT_LEFT_BUTTON                   0x0000
#define  GLUT_MIDDLE_BUTTON                 0x0001
#define  GLUT_RIGHT_BUTTON                  0x0002
#define  GLUT_DOWN                          0x0000
#define  GLUT_UP                            0x0001
#define  GLUT_LEFT                          0x0000
#define  GLUT_ENTERED                       0x0001
#define DEGTORAD(deg) 0.01745329251994329547f * (deg)
#define RADTODEG(rad) 57.29577951308232286465f * (rad)

// Serialization
#define SERIALIZE_MAX_SIZE							1024

#endif
