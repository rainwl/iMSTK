// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
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
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------


#ifndef SMMESHCOLLISIONMODEL_H
#define SMMESHCOLLISIONMODEL_H

// STL includes
#include <memory>

// SimMedTK includes
#include "smMesh/smMesh.h"
#include "smMesh/smSurfaceMesh.h"
#include "smMesh/smVolumeMesh.h"
#include "smCollision/smSurfaceTree.h"
#include "smCollision/smOctreeCell.h"
#include "smGeometry/smMeshModel.h"

///
/// @brief Mesh representation of a model.
/// Used for collision detection algorithms.
/// This class facilitates collision detection between a mesh and another
/// model.
///
/// @see smMeshToMeshCollision
///
class smMeshCollisionModel : public smMeshModel
{
public:
    using AABBNodeType = smOctreeCell;
    using AABBTreeType = smSurfaceTree<AABBNodeType>;
    using NodePairType = std::pair<std::shared_ptr<AABBNodeType>,std::shared_ptr<AABBNodeType>>;

public:
    smMeshCollisionModel();
    ~smMeshCollisionModel();

    ///
    /// @brief Set internal mesh data structure
    ///
    void setMesh(std::shared_ptr<smMesh> modelMesh);

    ///
    /// @brief Returns pointer to axis aligned bounding box hierarchy
    ///
    std::shared_ptr<AABBTreeType> getAABBTree();

    ///
    /// @brief Loads a triangular mesh and stores it.
    ///
    void loadTriangleMesh(const std::string &meshName, const smMeshFileType &type);

    ///
    /// @brief Set internal AABB tree
    ///
    void setAABBTree(std::shared_ptr<AABBTreeType> modelAabbTree);

    ///
    /// @brief Initialize AABB tree data structure
    ///
    void initAABBTree(const int &numLevels = 6);

private:
    std::shared_ptr<AABBTreeType> aabbTree; // Bounding volume hierarchy
};

#endif // SMMESHMODEL_H