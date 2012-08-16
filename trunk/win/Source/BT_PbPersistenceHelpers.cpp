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
#include "BT_PbPersistenceHelpers.h"
#include "Nx.pb.h"

Vec3 fromPbVec3(const PbVec3& vec)
{
	return Vec3(vec.x(), vec.y(), vec.z());
}

Quat fromPbQuat(const PbQuat& quat)
{
	Quat q;
	q.setWXYZ(quat.w(), quat.x(), quat.y(), quat.z());
	return q;
}

void toPbVec3(const Vec3& vec, PbVec3 * pbVec)
{
	pbVec->set_x((float) vec.x);
	pbVec->set_y((float) vec.y);
	pbVec->set_z((float) vec.z);
}

void toPbQuat(const Quat& quat, PbQuat * pbQuat)
{
	pbQuat->set_w((float) quat.w);
	pbQuat->set_x((float) quat.x);
	pbQuat->set_y((float) quat.y);
	pbQuat->set_z((float) quat.z);
}