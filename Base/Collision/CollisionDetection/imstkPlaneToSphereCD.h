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

#ifndef imstkPlaneToSphereCD_h
#define imstkPlaneToSphereCD_h

#include <memory>

#include "imstkCollisionDetection.h"

namespace imstk
{

class Plane;
class Sphere;
class CollisionData;

///
/// \class PlaneToSphereCD
///
/// \brief Plane to sphere collision detection
///
class PlaneToSphereCD : public CollisionDetection
{
public:

    ///
    /// \brief Constructor
    ///
    PlaneToSphereCD(std::shared_ptr<Plane> planeA,
                    std::shared_ptr<Sphere> sphereB,
                    CollisionData& colData) :
        CollisionDetection(CollisionDetection::Type::PlaneToSphere, colData),
        m_planeA(planeA),
        m_sphereB(sphereB)
    {}

    ///
    /// \brief Destructor
    ///
    ~PlaneToSphereCD() = default;

    ///
    /// \brief Detect collision and compute collision data
    ///
    void computeCollisionData() override;

private:

    std::shared_ptr<Plane> m_planeA;    ///>
    std::shared_ptr<Sphere> m_sphereB;  ///>
};

}

#endif // ifndef imstkPlaneToSphereCD_h