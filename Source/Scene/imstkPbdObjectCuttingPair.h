/*=========================================================================

Library: iMSTK

Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
& Imaging in Medicine, Rensselaer Polytechnic Institute.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0.txt

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=========================================================================*/

#pragma once

#include "imstkObjectInteractionPair.h"
#include "imstkVecDataArray.h"

#include <unordered_set>
#include <vector>

namespace imstk
{
class CollidingObject;
class PbdObject;
class SurfaceMesh;

///
/// \class PbdObjectCuttingPair
///
/// \brief This class defines a cutting pair between a PbdObject and a CollidingObject
///
class PbdObjectCuttingPair : public ObjectInteractionPair
{
public:
    PbdObjectCuttingPair(std::shared_ptr<PbdObject> pbdObj, std::shared_ptr<CollidingObject> cutObj);
    virtual ~PbdObjectCuttingPair() override = default;

    void apply();

protected:

    ///
    /// \brief Add new vertices to pbdObj
    ///
    void addVertices(std::shared_ptr<SurfaceMesh> pbdMesh,
                     std::shared_ptr<VecDataArray<double, 3>> vertices,
                     std::shared_ptr<VecDataArray<double, 3>> initialVertices);

    ///
    /// \brief Modify current vertices of pbdObj
    ///
    void modifyVertices(std::shared_ptr<SurfaceMesh> pbdMesh,
                        std::shared_ptr<std::vector<size_t>> vertexIndices,
                        std::shared_ptr<VecDataArray<double, 3>> vertices,
                        std::shared_ptr<VecDataArray<double, 3>> initialVertices);

    ///
    /// \brief Add new elements to pbdObj
    ///
    void addTriangles(std::shared_ptr<SurfaceMesh> pbdMesh,
                      std::shared_ptr<VecDataArray<int, 3>> elements);

    ///
    /// \brief Modify exsiting elements of pbdObj
    ///
    void modifyTriangles(std::shared_ptr<SurfaceMesh> pbdMesh,
                         std::shared_ptr<std::vector<size_t>> elementIndices,
                         std::shared_ptr<VecDataArray<int, 3>> elements);

    std::shared_ptr<std::unordered_set<size_t>> m_removeConstraintVertices = std::make_shared<std::unordered_set<size_t>>();
    std::shared_ptr<std::unordered_set<size_t>> m_addConstraintVertices    = std::make_shared<std::unordered_set<size_t>>();
};
}