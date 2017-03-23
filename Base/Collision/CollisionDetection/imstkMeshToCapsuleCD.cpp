/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version B.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-B.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   =========================================================================*/

#include "imstkMeshToCapsuleCD.h"

#include "imstkCollisionData.h"
#include "imstkCapsule.h"
#include "imstkMesh.h"
#include "imstkMath.h"

#include <g3log/g3log.hpp>

namespace imstk
{

void
MeshToCapsuleCD::computeCollisionData()
{
    // Clear collisionData
    m_colData.clearAll();

    auto capsulePos = m_capsule->getPosition();
    auto height = m_capsule->getHeight() * m_capsule->getScaling();
    auto radius = m_capsule->getRadius() * m_capsule->getScaling();

    // Get position of end points of the capsule
    auto p0 = capsulePos;
    auto p1 = m_capsule->getOrientation()*Vec3d(0., height, 0.) + capsulePos;
    auto mid = 0.5*(p0 + p1);
    auto p = p1 - p0;
    auto pDotp = p.dot(p);
    auto pDotp0 = p.dot(p0);

    size_t nodeId = 0;
    for (const auto& q : m_mesh->getVertexPositions())
    {
        // First, check collision with bounding sphere
        if ((mid - q).norm() > (radius + height*0.5))
        {
            nodeId++;
            continue;
        }

        // Do the actual check
        auto alpha = (q.dot(p) - pDotp0) / pDotp;
        auto closestPoint = p0 + p*alpha;

        // If the point is inside the bounding sphere then the closest point
        // should be inside the capsule
        auto dist = (closestPoint - q).norm();
        if (dist <= radius)
        {
            auto direction = (closestPoint - q) / dist;
            auto pointOnCapsule = closestPoint - radius*direction;
            m_colData.MAColData.push_back({ nodeId, p - pointOnCapsule });
        }
        nodeId++;
    }
}

} // imstk