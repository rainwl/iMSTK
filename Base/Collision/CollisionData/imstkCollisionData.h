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

#ifndef imstkCollisionData_h
#define imstkCollisionData_h

// imstk
#include "imstkMath.h"

namespace imstk
{

///
/// \struct PositionDirectionCollisionData
///
/// \brief Point-penetration depth collision data
///
struct PositionDirectionCollisionData
{
    Vec3d posA;
    Vec3d posB;
    Vec3d dirAtoB;
    double penetrationDepth;
};

///
/// \struct MeshToAnalyticalPointDepthCollisionData
///
/// \brief Mesh to analytical point-penetration depth collision data
///
struct MeshToAnalyticalPointDepthCollisionData
{
    int nodeId;
    Vec3d penetrationVector;
};

///
/// \struct VertexTriangleCollisionData
///
/// \brief Vertex-triangle collision data
///
struct VertexTriangleCollisionData
{
    size_t vertexIdA;
    size_t triIdB;
    float time;

    VertexTriangleCollisionData(size_t vIdA, size_t fIdB, float t = -1)
    {
        vertexIdA = vIdA;
        triIdB = fIdB;
        time = t;
    }
};

///
/// \struct TriangleVertexCollisionData
///
/// \brief Triangle-vertex collision data
///
struct TriangleVertexCollisionData
{
    size_t triIdA;
    size_t vertexIdB;
    float time;

    TriangleVertexCollisionData(size_t fIdA, size_t vIdB, float t = -1)
    {
        triIdA = fIdA;
        vertexIdB = vIdB;
        time = t;
    }
};

///
/// \struct EdgeEdgeCollisionData
///
/// \brief Edge-Edge collision data
///
struct EdgeEdgeCollisionData
{
    std::pair<size_t, size_t> edgeIdA;
    std::pair<size_t, size_t> edgeIdB;
    float time;

    EdgeEdgeCollisionData(size_t eA_v1, size_t eA_v2, size_t eB_v1, size_t eB_v2, float t = -1)
    {
        edgeIdA = std::pair<size_t, size_t>(eA_v1, eA_v2);
        edgeIdB = std::pair<size_t, size_t>(eB_v1, eB_v2);
        time = t;
    }
};

///
/// \struct CollisionData
///
/// \brief Class that is the holder of all types of collision data
///
class CollisionData
{
public:

    void clearAll()
    {
        PDColData.clear();
        VTColData.clear();
        TVColData.clear();
        EEColData.clear();
    }

    std::vector<PositionDirectionCollisionData> PDColData; //!< Position Direction collision data
    std::vector<VertexTriangleCollisionData> VTColData;    //!< Vertex Triangle collision data
    std::vector<TriangleVertexCollisionData> TVColData;    //!< Triangle Vertex collision data
    std::vector<EdgeEdgeCollisionData> EEColData;          //!< Edge Edge collision data

    std::vector<MeshToAnalyticalPointDepthCollisionData> PDMeshAnaColData;
};

}

#endif // ifndef imstkCollisionData_h