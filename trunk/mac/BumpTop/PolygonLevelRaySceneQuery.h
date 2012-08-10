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

#ifndef BUMPTOP_POLYGONLEVELRAYSCENEQUERY_H_
#define BUMPTOP_POLYGONLEVELRAYSCENEQUERY_H_

#include <utility>
#include <vector>

struct BumpRaySceneQueryResultEntry : public Ogre::RaySceneQueryResultEntry {
  bool operator < (const RaySceneQueryResultEntry& rhs) const {
    if (movable->getRenderQueueGroup() < rhs.movable->getRenderQueueGroup()) {
      return false;
    }
    if (movable->getRenderQueueGroup() > rhs.movable->getRenderQueueGroup()) {
      return true;
    }
    Ogre::Renderable *lhs_renderable = NULL;
    if (this->movable->getMovableType().compare("Entity") == 0) {
      Ogre::Entity *entity = static_cast<Ogre::Entity*>(this->movable);
      if (entity->getNumSubEntities() == 1) {
        lhs_renderable = entity->getSubEntity(0);
      }
    }  // TODO: add support for  ManualObjects as well

    Ogre::Renderable *rhs_renderable = NULL;
    if (rhs.movable->getMovableType().compare("Entity") == 0) {
      Ogre::Entity *entity = static_cast<Ogre::Entity*>(rhs.movable);
      if (entity->getNumSubEntities() == 1) {
        rhs_renderable = entity->getSubEntity(0);
      }
    }  // TODO: add support for  ManualObjects as well

    if (lhs_renderable != NULL && rhs_renderable != NULL) {
      if (lhs_renderable->getRenderableToRenderNextTo() == rhs_renderable) {
        return lhs_renderable->renderAfterOtherRenderable();
      } else if (rhs_renderable->getRenderableToRenderNextTo() == lhs_renderable) {
        return !rhs_renderable->renderAfterOtherRenderable();
      } else if (lhs_renderable->getRenderableToRenderNextTo() ==
                 rhs_renderable->getRenderableToRenderNextTo()) {
        if (lhs_renderable->renderAfterOtherRenderable() &&
            !rhs_renderable->renderAfterOtherRenderable()) {
          return true;
        } else if (!lhs_renderable->renderAfterOtherRenderable() &&
                   rhs_renderable->renderAfterOtherRenderable()) {
          return false;
        }
      }
    }

    return this->distance < rhs.distance;
  }
};

typedef std::vector<BumpRaySceneQueryResultEntry> BumpRaySceneQueryResult;

class PolygonLevelRaySceneQuery {
 public:
  explicit PolygonLevelRaySceneQuery(Ogre::SceneManager *scene_manager);
  explicit PolygonLevelRaySceneQuery(Ogre::SceneManager *scene_manager, const Ogre::Ray &ray);
  ~PolygonLevelRaySceneQuery();

  void setRay(const Ogre::Ray &ray);
  BumpRaySceneQueryResult& execute(void);
  std::pair<bool, Ogre::Real> intersectWithEntityMesh(Ogre::Entity *entity, const Ogre::Ray &ray);
 protected:
  static void getMeshInformation(const Ogre::MeshPtr &mesh,
                                 size_t &vertex_count,
                                 Ogre::Vector3* &vertices,
                                 size_t &index_count,
                                 unsigned long* &indices,  // NOLINT
                                 const Ogre::Vector3 &position = Ogre::Vector3::ZERO,
                                 const Ogre::Quaternion &orient = Ogre::Quaternion::IDENTITY,
                                 const Ogre::Vector3 &scale = Ogre::Vector3::UNIT_SCALE);
  Ogre::RaySceneQuery* ray_scene_query_;
  Ogre::SceneManager* scene_manager_;
  BumpRaySceneQueryResult ray_scene_query_result_;
};

#endif  // BUMPTOP_POLYGONLEVELRAYSCENEQUERY_H_
