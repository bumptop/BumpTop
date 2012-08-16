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

#ifndef BT_Kernel_h__
#define BT_Kernel_h__

class BumpObject;

class ClusterItem
{
public:
	BumpObject *obj;
	Vec2 pos;
	NxReal ratio;

	ClusterItem();
	ClusterItem(BumpObject *obj);
	
	NxReal dist(ClusterItem *k);
	NxReal distSq(ClusterItem *k);
	NxReal distPos(const Vec2 &p);
	NxReal distPosSq(const Vec2 &p);

	void update();

};

#endif // BT_Kernel_h__