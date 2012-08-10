/*
 *  Copyright 2012 Google Inc. All Rights Reserved.
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "BumpTop/PolygonLevelRaySceneQuery.h"

#include <algorithm>
#include <utility>

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/for_each.h"

PolygonLevelRaySceneQuery::PolygonLevelRaySceneQuery(Ogre::SceneManager *scene_manager)
: scene_manager_(scene_manager) {
  ray_scene_query_ = scene_manager->createRayQuery(Ogre::Ray());
  ray_scene_query_->setSortByDistance(true);
}

PolygonLevelRaySceneQuery::PolygonLevelRaySceneQuery(Ogre::SceneManager *scene_manager,
                                                     const Ogre::Ray &ray)
: scene_manager_(scene_manager) {
  ray_scene_query_ = scene_manager->createRayQuery(ray);
  ray_scene_query_->setSortByDistance(true);
}

PolygonLevelRaySceneQuery::~PolygonLevelRaySceneQuery() {
  scene_manager_->destroyQuery(ray_scene_query_);
}

void PolygonLevelRaySceneQuery::setRay(const Ogre::Ray &ray) {
  ray_scene_query_->setRay(ray);
}

// Do a polygon-by-polygon check and find out precisely if something was hit
// (Ogre just does a crude AABB check)
// Code found in wiki: http://www.ogre3d.org/wiki/index.php/Raycasting_to_the_polygon_level
BumpRaySceneQueryResult& PolygonLevelRaySceneQuery::execute(void) {
  BumpTopApp::singleton()->pushGLContextAndSwitchToOgreGLContext();
  Ogre::RaySceneQueryResult& query_result = ray_scene_query_->execute();

  Ogre::Ray ray = ray_scene_query_->getRay();

  ray_scene_query_result_.clear();

  if (query_result.size() == 0) {
    BumpTopApp::singleton()->popGLContext();
    return ray_scene_query_result_;
  }

  for_each(const Ogre::RaySceneQueryResultEntry& query_result_entry, query_result) {
    // only check this result if its a hit against an entity
    if ((query_result_entry.movable != NULL) &&
        (query_result_entry.movable->getMovableType().compare("Entity") == 0)) {
      // get the entity to check
      Ogre::Entity *entity = static_cast<Ogre::Entity*>(query_result_entry.movable);

      std::pair<bool, Ogre::Real> intersection = intersectWithEntityMesh(entity, ray);

      // if we found a new closest raycast for this object, update the
      // closest_result before moving on to the next object.
      if (intersection.first) {
        Ogre::RaySceneQueryResultEntry updated_query_result_entry = query_result_entry;
        updated_query_result_entry.distance = intersection.second;
        ray_scene_query_result_.insert(ray_scene_query_result_.begin(),
                                       *(reinterpret_cast<BumpRaySceneQueryResultEntry*>(&updated_query_result_entry)));
      }
    }
  }
  BumpTopApp::singleton()->popGLContext();
  std::sort(ray_scene_query_result_.begin(), ray_scene_query_result_.end());
  return ray_scene_query_result_;
}

// Get the mesh information for the given mesh.
// Code found in Wiki: www.ogre3d.org/wiki/index.php/RetrieveVertexData
void PolygonLevelRaySceneQuery::getMeshInformation(
                                                   const Ogre::MeshPtr &mesh,
                                                   size_t &vertex_count,
                                                   Ogre::Vector3* &vertices,
                                                   size_t &index_count,
                                                   unsigned long* &indices,  // NOLINT
                                                   const Ogre::Vector3 &position,
                                                   const Ogre::Quaternion &orient,
                                                   const Ogre::Vector3 &scale) {
  vertex_count = index_count = 0;

  bool added_shared = false;
  size_t current_offset = vertex_count;
  size_t shared_offset = vertex_count;
  size_t next_offset = vertex_count;
  size_t index_offset = index_count;

  // Calculate how many vertices and indices we're going to need
  for (int i = 0; i < mesh->getNumSubMeshes(); i++) {
    Ogre::SubMesh* submesh = mesh->getSubMesh(i);

    // We only need to add the shared vertices once
    if (submesh->useSharedVertices) {
      if (!added_shared) {
        Ogre::VertexData* vertex_data = mesh->sharedVertexData;
        vertex_count += vertex_data->vertexCount;
        added_shared = true;
      }
    } else {
      Ogre::VertexData* vertex_data = submesh->vertexData;
      vertex_count += vertex_data->vertexCount;
    }

    // Add the indices
    Ogre::IndexData* index_data = submesh->indexData;
    index_count += index_data->indexCount;
  }

  // Allocate space for the vertices and indices
  vertices = new Ogre::Vector3[vertex_count];
  indices = new unsigned long[index_count];  // NOLINT

  added_shared = false;

  // Run through the submeshes again, adding the data into the arrays
  for (int i = 0; i < mesh->getNumSubMeshes(); i++) {
    Ogre::SubMesh* submesh = mesh->getSubMesh(i);

    Ogre::VertexData* vertex_data = submesh->useSharedVertices ?
    mesh->sharedVertexData :
    submesh->vertexData;
    if ((!submesh->useSharedVertices)||(submesh->useSharedVertices && !added_shared)) {
      if (submesh->useSharedVertices) {
        added_shared = true;
        shared_offset = current_offset;
      }

      const Ogre::VertexElement* posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
      Ogre::HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());
      unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
      Ogre::Real* pReal;

      for (size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize()) {
        posElem->baseVertexPointerToElement(vertex, &pReal);

        Ogre::Vector3 pt;

        pt.x = (*pReal++);
        pt.y = (*pReal++);
        pt.z = (*pReal++);

        pt = (orient * (pt * scale)) + position;

        vertices[current_offset + j].x = pt.x;
        vertices[current_offset + j].y = pt.y;
        vertices[current_offset + j].z = pt.z;
      }
      vbuf->unlock();
      next_offset += vertex_data->vertexCount;
    }

    Ogre::IndexData* index_data = submesh->indexData;

    size_t numTris = index_data->indexCount / 3;
    unsigned short* pShort;  // NOLINT
    unsigned int* pInt;
    Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;
    bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
    if (use32bitindexes) {
      pInt = static_cast<unsigned int*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
    } else {
      pShort = static_cast<unsigned short*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));  // NOLINT
    }

    for (size_t k = 0; k < numTris; ++k) {
      size_t offset = (submesh->useSharedVertices)?shared_offset:current_offset;

      unsigned int vindex = use32bitindexes? *pInt++ : *pShort++;
      indices[index_offset + 0] = vindex + offset;
      vindex = use32bitindexes? *pInt++ : *pShort++;
      indices[index_offset + 1] = vindex + offset;
      vindex = use32bitindexes? *pInt++ : *pShort++;
      indices[index_offset + 2] = vindex + offset;

      index_offset += 3;
    }
    ibuf->unlock();
    current_offset = next_offset;
  }
}

std::pair<bool, Ogre::Real> PolygonLevelRaySceneQuery::intersectWithEntityMesh(Ogre::Entity *entity,
                                                                               const Ogre::Ray &ray) {
  BumpTopApp::singleton()->pushGLContextAndSwitchToOgreGLContext();
  // mesh data to retrieve
  size_t vertex_count;
  size_t index_count;
  Ogre::Vector3 *vertices;
  unsigned long *indices;  // NOLINT

  Ogre::Real closest_distance = -1.0f;

  // get the mesh information (this is a static method)
  getMeshInformation(entity->getMesh(),
                     vertex_count,
                     vertices,
                     index_count,
                     indices,
                     entity->getParentNode()->_getDerivedPosition(),
                     entity->getParentNode()->_getDerivedOrientation(),
                     entity->getParentNode()->_getDerivedScale());

  // test for hitting individual triangles on the mesh
  bool new_closest_found = false;
  for (int i = 0; i < static_cast<int>(index_count); i += 3) {
    // check for a hit against this triangle
    std::pair<bool, Ogre::Real> hit;
    hit = Ogre::Math::intersects(ray,
                                 vertices[indices[i]],
                                 vertices[indices[i+1]],
                                 vertices[indices[i+2]],
                                 true,
                                 false);

    // if it was a hit check if its the closest
    if (hit.first) {
      if ((closest_distance < 0.0f) ||
          (hit.second < closest_distance)) {
        // this is the closest so far, save it off
        closest_distance = hit.second;
        new_closest_found = true;
      }
    }
  }

  // free the verticies and indicies memory
  delete[] vertices;
  delete[] indices;
  BumpTopApp::singleton()->popGLContext();
  if (new_closest_found) {
    return std::pair<bool, Ogre::Real>(true, closest_distance);
  } else {
    return std::pair<bool, Ogre::Real>(false, closest_distance);
  }
}

