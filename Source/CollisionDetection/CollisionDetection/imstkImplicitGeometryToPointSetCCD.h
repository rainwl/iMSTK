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

#include "imstkCollisionDetection.h"
#include "imstkImplicitFunctionFiniteDifferenceFunctor.h"

namespace imstk
{
template<typename T, int N> class VecDataArray;

class ImplicitGeometry;
class PointSet;
struct CollisionData;

///
/// \class ImplicitGeometryToPointSetCCD
///
/// \brief ImplicitGeometry to PointSet continous collision detection.
/// This CD method marches along the displacement of the points in the pointset
/// to converge on the zero crossing of the implicit geometry. Does not produce
/// time of impacts
///
class ImplicitGeometryToPointSetCCD : public CollisionDetection
{
public:
    ///
    /// \brief
    /// \param ImplicitGeometry
    /// \param PointSet to test collision with
    /// \param CollisionData to write too
    ///
    ImplicitGeometryToPointSetCCD(std::shared_ptr<ImplicitGeometry> implicitGeomA,
                                  std::shared_ptr<PointSet>         pointSetB,
                                  std::shared_ptr<CollisionData>    colData);

    ///
    /// \brief Detect collision and compute collision data
    ///
    void computeCollisionData() override;

private:
    std::shared_ptr<ImplicitGeometry> m_implicitGeomA;
    std::shared_ptr<PointSet>       m_pointSetB;
    ImplicitFunctionCentralGradient centralGrad;

    std::shared_ptr<VecDataArray<double, 3>> displacements;
};
}