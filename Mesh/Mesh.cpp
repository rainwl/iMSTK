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

#include "Mesh/Mesh.h"

#include <limits>

// VEgaFEM include
#include "objMesh.h"

// SimMedTK includes
#include "Rendering/GLRenderer.h"
#include "Rendering/Viewer.h"
#include "Core/Factory.h"

BaseMesh::BaseMesh()
{
//     SDK::getInstance()->registerMesh(safeDownCast<BaseMesh>());
}

void BaseMesh::updateOriginalVertsWithCurrent()
{
    origVerts = vertices;
}

/// \brief constructor
Mesh::Mesh()
{
    triangles = 0;
    texCoord = 0;
    triNormals = 0;
    vertNormals = 0;
    triTangents = 0;
    vertTangents = 0;
    nbrTriangles = 0;
    nbrTexCoordForTrainglesOBJ = 0;
    type = core::ClassType::Mesh;
    isTextureCoordAvailable = false;
    tangentChannel = false;
    this->setRenderDelegate(
      Factory<RenderDelegate>::createConcreteClass(
        "MeshRenderDelegate"));
}

/// \brief destructor
Mesh::~Mesh()
{
    delete [] triangles;
    delete [] texCoord;
    delete [] triNormals;
    delete [] vertNormals;
    delete [] triTangents;
    delete [] vertTangents;

}

/// \brief
void Mesh::allocateAABBTris()
{
    this->triAABBs.resize(nbrTriangles);
    this->updateTriangleAABB();
}

/// \brief
void CalculateTangentArray(int vertexCount, const core::Vec3d *vertex,
                           const core::Vec3d *normal, const TexCoord *texcoord,
                           long triangleCount, const Triangle *triangle,
                           core::Vec3d *tangent)
{

    core::Vec3d *tan1 = new core::Vec3d[vertexCount * 2];
    core::Vec3d *tan2 = tan1 + vertexCount;
    memset(tan1, 0, vertexCount * sizeof(core::Vec3d) * 2);

    for (long a = 0; a < triangleCount; a++)
    {
        long i1 = triangle->vert[0];
        long i2 = triangle->vert[1];
        long i3 = triangle->vert[2];

        const core::Vec3d& v1 = vertex[i1];
        const core::Vec3d& v2 = vertex[i2];
        const core::Vec3d& v3 = vertex[i3];

        const TexCoord& w1 = texcoord[i1];
        const TexCoord& w2 = texcoord[i2];
        const TexCoord& w3 = texcoord[i3];

        float x1 = v2[0] - v1[0];
        float x2 = v3[0] - v1[0];
        float y1 = v2[1] - v1[1];
        float y2 = v3[1] - v1[1];
        float z1 = v2[2] - v1[2];
        float z2 = v3[2] - v1[2];

        float s1 = w2.u - w1.u;
        float s2 = w3.u - w1.u;
        float t1 = w2.v - w1.v;
        float t2 = w3.v - w1.v;

        float r = 1.0F / (s1 * t2 - s2 * t1);
        core::Vec3d sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
                     (t2 * z1 - t1 * z2) * r);
        core::Vec3d tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
                     (s1 * z2 - s2 * z1) * r);

        tan1[i1] += sdir;
        tan1[i2] += sdir;
        tan1[i3] += sdir;

        tan2[i1] += tdir;
        tan2[i2] += tdir;
        tan2[i3] += tdir;

        triangle++;
    }

    for (long a = 0; a < vertexCount; a++)
    {
        core::Vec3d n = normal[a];
        core::Vec3d t = tan1[a];
        tangent[a] = (t - n * n.dot(t));
        tangent[a].normalize();
    }

    delete[] tan1;
}

/// \brief calucate the triangle tangents
void Mesh::calcTriangleTangents()
{

    int t;

    // First calculate the triangle tangents
    for (t = 0; t < nbrTriangles; t++)
    {
        Triangle *tmpTri = &triangles[t];
        core::Vec3d *v0 = &vertices[tmpTri->vert[0]];
        core::Vec3d *v1 = &vertices[tmpTri->vert[1]];
        core::Vec3d *v2 = &vertices[tmpTri->vert[2]];
        TexCoord *t0 = &texCoord[tmpTri->vert[0]];
        TexCoord *t1 = &texCoord[tmpTri->vert[1]];
        TexCoord *t2 = &texCoord[tmpTri->vert[2]];

        if (this->meshFileType == BaseMesh::MeshFileType::ThreeDS)
        {
            calculateTangent(*v2, *v1, *v0, *t2, *t1, *t0, triTangents[t]);
        }
        else if (this->meshFileType == BaseMesh::MeshFileType::Obj)
        {
            calculateTangent_test(*v0, *v1, *v2, *t0, *t1, *t2, triTangents[t]);
        }
    }

    //calculate the vertex normals
    if (this->meshFileType == BaseMesh::MeshFileType::ThreeDS || this->meshFileType == BaseMesh::MeshFileType::Obj)
    {
        for (int v = 0; v < nbrVertices; v++)
        {
            vertTangents[v][0] = vertTangents[v][1] = vertTangents[v][2] = 0;

            for (size_t i = 0; i < vertTriNeighbors[v].size(); i++)
            {
                vertTangents[v] += triTangents[vertTriNeighbors[v][i]];
            }

            vertTangents[v].normalize();
            vertTangents[v] = (vertTangents[v] - vertNormals[v] * vertNormals[v].dot(vertTangents[v]));
            vertTangents[v].normalize();
        }
    }
}

/// \brief calucate the triangle tangent for rendering purposes
void Mesh::calculateTangent(core::Vec3d& p1, core::Vec3d& p2, core::Vec3d& p3, TexCoord& t1, TexCoord& t2, TexCoord& t3, core::Vec3d& t)
{

    core::Vec3d v1;
    core::Vec3d v2;

    v1[0] = p2[0] - p1[0];
    v1[1] = p2[1] - p1[1];
    v1[2] = p2[2] - p1[2];

    v2[0] = p3[0] - p1[0];
    v2[1] = p3[1] - p1[1];
    v2[2] = p3[2] - p1[2];

    float bb1 = t2.v - t1.v;
    float bb2 = t3.v - t1.v;

    t[0] = bb2 * v1[0] - bb1 * v2[0];
    t[1] = bb2 * v1[1] - bb1 * v2[1];
    t[2] = bb2 * v1[2] - bb1 * v2[2];

    t.normalize();
}

/// \brief
void Mesh::calculateTangent_test(core::Vec3d& p1, core::Vec3d& p2, core::Vec3d& p3, TexCoord& t1, TexCoord& t2, TexCoord& t3, core::Vec3d& t)
{

    core::Vec3d v1;
    core::Vec3d v2;

    v1[0] = p2[0] - p1[0];
    v1[1] = p2[1] - p1[1];
    v1[2] = p2[2] - p1[2];

    v2[0] = p3[0] - p1[0];
    v2[1] = p3[1] - p1[1];
    v2[2] = p3[2] - p1[2];

    float tt1 = t2.u - t1.u;
    float tt2 = t3.u - t1.u;

    float bb1 = t2.v - t1.v;
    float bb2 = t3.v - t1.v;
    float r = 1.0F / (tt1 * bb2 - tt2 * bb1);
    t[0] = (bb2 * v1[0] - bb1 * v2[0]) * r;
    t[1] = (bb2 * v1[1] - bb1 * v2[1]) * r;
    t[2] = (bb2 * v1[2] - bb1 * v2[2]) * r;
}

/// \brief calculates the normal of the vertex
void Mesh::updateVertexNormals()
{
    core::Vec3d temp = core::Vec3d::Zero();

    for (int i = 0; i < nbrVertices; i++)
    {
        for (size_t j = 0; j < vertTriNeighbors[i].size(); j++)
        {
            temp += triNormals[vertTriNeighbors[i][j]];
        }

        vertNormals[i] = temp;
        vertNormals[i].normalize();
        temp = core::Vec3d::Zero();
    }
}

/// \brief updates the normal of all the triangle
void Mesh::updateTriangleNormals()
{

    for (int i = 0; i < nbrTriangles; i++)
    {
        triNormals[i] = calculateTriangleNormal(i).normalized();
    }
}

/// \brief calculates the normal of a triangle
core::Vec3d Mesh::calculateTriangleNormal(int triNbr)
{

    core::Vec3d v[3];
    Triangle temp = this->triangles[triNbr];

    v[0] = this->vertices[temp.vert[0]];
    v[1] = this->vertices[temp.vert[1]];
    v[2] = this->vertices[temp.vert[2]];

    return (v[1] - v[0]).cross(v[2] - v[0]).normalized();
}

/// \brief allocates vertices and related array
bool Mesh::initVertexArrays(int nbr)
{

    if (nbr < 0)
    {
        return false;
    }

    this->nbrVertices = nbr;
    this->vertices.resize(nbr);
    this->origVerts.resize(nbr);
    this->vertNormals = new core::Vec3d[nbr];
    this->vertTangents = new core::Vec3d[nbr];
    this->texCoord = new TexCoord[nbr];
    return true;
}

/// \brief allocates triangle and related array
bool Mesh::initTriangleArrays(int nbr)
{

    if (nbr < 0)
    {
        return false;
    }

    this->nbrTriangles = nbr;

    this->triangles = new Triangle[nbr];
    this->triNormals = new core::Vec3d[nbr];
    this->triTangents = new core::Vec3d[nbr];
    return true;
}

/// \brief initializes the vertex neighbors
void Mesh::initVertexNeighbors()
{

    int i;
    vertTriNeighbors.resize(nbrVertices);

    for (i = 0; i < nbrTriangles; i++)
    {
        vertTriNeighbors[triangles[i].vert[0]].push_back(i);
        vertTriNeighbors[triangles[i].vert[1]].push_back(i);
        vertTriNeighbors[triangles[i].vert[2]].push_back(i);
    }
}

/// \brief initializes the vertex neighbors
void Mesh::calcNeighborsVertices()
{

    int i;
    int triangleIndex;
    int candidate[3];

    vertVertNeighbors.resize(nbrVertices);

    for (i = 0; i < nbrVertices; i++)
    {
        for (size_t j = 0; j < vertTriNeighbors[i].size(); j++)
        {
            triangleIndex = vertTriNeighbors[i][j];
            candidate[0] = triangles[triangleIndex].vert[0];
            candidate[1] = triangles[triangleIndex].vert[1];
            candidate[2] = triangles[triangleIndex].vert[2];

            if (candidate[0] == i)
            {
                candidate[0] = -1;
            }

            if (candidate[1] == i)
            {
                candidate[1] = -1;
            }

            if (candidate[2] == i)
            {
                candidate[2] = -1;
            }

            for (size_t k = 0; k < vertVertNeighbors[i].size(); k++)
            {
                if (vertVertNeighbors[i][k] == candidate[0])
                {
                    candidate[0] = -1;
                }

                if (vertVertNeighbors[i][k] == candidate[1])
                {
                    candidate[1] = -1;
                }

                if (vertVertNeighbors[i][k] == candidate[2])
                {
                    candidate[2] = -1;
                }
            }

            if (candidate[0] != -1)
            {
                vertVertNeighbors[i].push_back(candidate[0]);
            }

            if (candidate[1] != -1)
            {
                vertVertNeighbors[i].push_back(candidate[1]);
            }

            if (candidate[2] != -1)
            {
                vertVertNeighbors[i].push_back(candidate[2]);
            }

        }
    }
}

/// \brief
void Mesh::upadateAABB()
{
    double minx = std::numeric_limits<double>::max();
    double miny = std::numeric_limits<double>::max();
    double minz = std::numeric_limits<double>::max();
    double maxx = -std::numeric_limits<double>::max();
    double maxy = -std::numeric_limits<double>::max();
    double maxz = -std::numeric_limits<double>::max();

    for (int i = 0; i < nbrVertices; i++)
    {
        minx = std::min(vertices[i][0], minx);
        miny = std::min(vertices[i][1], miny);
        minz = std::min(vertices[i][2], minz);

        maxx = std::max(vertices[i][0], maxx);
        maxy = std::max(vertices[i][1], maxy);
        maxz = std::max(vertices[i][2], maxz);
    }

    aabb.aabbMin[0] = minx - (maxx - minx) * SIMMEDTK_MESH_AABBSKINFACTOR;
    aabb.aabbMin[1] = miny - (maxy - miny) * SIMMEDTK_MESH_AABBSKINFACTOR;
    aabb.aabbMin[2] = minz - (maxz - minz) * SIMMEDTK_MESH_AABBSKINFACTOR;

    aabb.aabbMax[0] = maxx + (maxx - minx) * SIMMEDTK_MESH_AABBSKINFACTOR;
    aabb.aabbMax[1] = maxy + (maxy - miny) * SIMMEDTK_MESH_AABBSKINFACTOR;
    aabb.aabbMax[2] = maxz + (maxz - minz) * SIMMEDTK_MESH_AABBSKINFACTOR;
}

/// \brief
void Mesh::calcEdges()
{
    Edge edge;
    edges.reserve(SIMMEDTK_MESH_RESERVEDMAXEDGES);

    for (int i = 0; i < nbrVertices; i++)
    {
        for (size_t j = 0; j < vertVertNeighbors[i].size(); j++)
        {
            if (vertVertNeighbors[i][j] > i)
            {
                edge.vert[0] = i;
                edge.vert[1] = vertVertNeighbors[i][j];
                edges.push_back(edge);
            }
        }
    }
}

/// \brief
void Mesh::translate(float p_offsetX, float p_offsetY, float p_offsetZ)
{

    for (int i = 0; i < nbrVertices; i++)
    {
        vertices[i][0] = vertices[i][0] + p_offsetX;
        vertices[i][1] = vertices[i][1] + p_offsetY;
        vertices[i][2] = vertices[i][2] + p_offsetZ;

        origVerts[i][0] = vertices[i][0] + p_offsetX;
        origVerts[i][1] = vertices[i][1] + p_offsetY;
        origVerts[i][2] = vertices[i][2] + p_offsetZ;
    }

    upadateAABB();
}

/// \brief
void Mesh::translate(core::Vec3d p_offset)
{

    for (int i = 0; i < nbrVertices; i++)
    {
        vertices[i] = vertices[i] + p_offset;
        origVerts[i] = origVerts[i] + p_offset;
    }

    upadateAABB();
}

/// \brief
void Mesh::scale(core::Vec3d p_scaleFactors)
{

    for (int i = 0; i < nbrVertices; i++)
    {
        vertices[i][0] = vertices[i][0] * p_scaleFactors[0];
        vertices[i][1] = vertices[i][1] * p_scaleFactors[1];
        vertices[i][2] = vertices[i][2] * p_scaleFactors[2];

        origVerts[i][0] = origVerts[i][0] * p_scaleFactors[0];
        origVerts[i][1] = origVerts[i][1] * p_scaleFactors[1];
        origVerts[i][2] = origVerts[i][2] * p_scaleFactors[2];
    }

    upadateAABB();
}

/// \brief
void Mesh::rotate(const Matrix33d &p_rot)
{

    for (int i = 0; i < nbrVertices; i++)
    {
        vertices[i] = p_rot * vertices[i];
        origVerts[i] = p_rot * origVerts[i];
        vertNormals[i] = p_rot * vertNormals[i];
    }

    for (int i = 0; i < nbrTriangles; i++)
    {
        triNormals[i] = p_rot * triNormals[i];
    }

    calcTriangleTangents();
    upadateAABB();
}

/// \brief
void Mesh::updateTriangleAABB()
{
    for (int i = 0; i < nbrTriangles; i++)
    {
        // min
        triAABBs[i].aabbMin[0] = std::min(vertices[triangles[i].vert[0]][0], vertices[triangles[i].vert[1]][0]);
        triAABBs[i].aabbMin[0] = std::min(triAABBs[i].aabbMin[0] ,   vertices[triangles[i].vert[2]][0]);

        triAABBs[i].aabbMin[1] = std::min(vertices[triangles[i].vert[0]][1], vertices[triangles[i].vert[1]][1]);
        triAABBs[i].aabbMin[1] = std::min(triAABBs[i].aabbMin[1] ,   vertices[triangles[i].vert[2]][1]);

        triAABBs[i].aabbMin[2] = std::min(vertices[triangles[i].vert[0]][2], vertices[triangles[i].vert[1]][2]);
        triAABBs[i].aabbMin[2] = std::min(triAABBs[i].aabbMin[2] ,   vertices[triangles[i].vert[2]][2]);

        //max
        triAABBs[i].aabbMax[0] = std::max(vertices[triangles[i].vert[0]][0], vertices[triangles[i].vert[1]][0]);
        triAABBs[i].aabbMax[0] = std::max(triAABBs[i].aabbMax[0] ,   vertices[triangles[i].vert[2]][0]);

        triAABBs[i].aabbMax[1] = std::max(vertices[triangles[i].vert[0]][1], vertices[triangles[i].vert[1]][1]);
        triAABBs[i].aabbMax[1] = std::max(triAABBs[i].aabbMax[1] ,   vertices[triangles[i].vert[2]][1]);

        triAABBs[i].aabbMax[2] = std::max(triAABBs[i].aabbMax[2],    vertices[triangles[i].vert[2]][2]);
    }
}

/// \brief
void Mesh::checkCorrectWinding()
{
    int x[3];
    int p[3];

    for (int i = 0; i < nbrTriangles; i++)
    {
        x[0] = triangles[i].vert[0];
        x[1] = triangles[i].vert[1];
        x[2] = triangles[i].vert[2];

        for (int j = 0; j < nbrTriangles; j++)
        {
            if (j == i)
            {
                continue;
            }

            p[0] = triangles[j].vert[0];
            p[1] = triangles[j].vert[1];
            p[2] = triangles[j].vert[2];

            if (x[0] == p[0] && x[1] == p[1])
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }

            if (x[0] == p[1] && x[1] == p[2])
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }

            if (x[0] == p[2] && x[1] == p[0])
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }

            if (x[1] == p[0] && x[2] == p[1])
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }

            if (x[1] == p[1] && x[2] == p[2])
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }

            if (x[1] == p[2] && x[2] == p[0])
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }

            if (x[2] == p[0] && x[0] == p[1])
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }

            if (x[2] == p[1] && x[0] == p[2])
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }

            if (x[2] == p[2] && x[0] == p[0])
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }
        }
    }
}

TextureAttachment::TextureAttachment()
{
}

bool BaseMesh::isMeshTextured()
{
    return isTextureCoordAvailable;
}

void BaseMesh::assignTexture( int p_textureId )
{
    TextureAttachment attachment;
    attachment.textureId = p_textureId;

    if ( p_textureId > 0 )
    {
        this->textureIds.push_back( attachment );
    }
}
void BaseMesh::assignTexture(const std::string& p_referenceName)
{
    int textureId;
    TextureAttachment attachment;

    if (TextureManager::findTextureId(p_referenceName, textureId) == SIMMEDTK_TEXTURE_OK)
    {
        attachment.textureId = textureId;
        this->textureIds.push_back(attachment);
    }
}
LineMesh::LineMesh( int p_nbrVertices ) : BaseMesh()
{
    nbrVertices = p_nbrVertices;
    vertices.reserve( nbrVertices );
    origVerts.reserve( nbrVertices );
    edgeAABBs = new AABB[nbrVertices - 1];
    texCoord = new TexCoord[nbrVertices];
    edges = new Edge[nbrVertices - 1];
    nbrEdges = nbrVertices - 1;
    isTextureCoordAvailable = false;
    createAutoEdges();
}
LineMesh::LineMesh( int p_nbrVertices, bool autoEdge ) : BaseMesh()
{
    nbrVertices = p_nbrVertices;
    vertices.reserve( nbrVertices );
    origVerts.reserve( nbrVertices );
    texCoord = new TexCoord[nbrVertices];

    /// Edge AABB should be assigned by the instance
    edgeAABBs = nullptr;

    /// Edges should be assigned by the instance
    edges = nullptr;

    /// Number of edges should be assigned by the instance
    nbrEdges = 0;

    isTextureCoordAvailable = false;

    if ( autoEdge )
    {
        createAutoEdges();
    }
}
void LineMesh::createAutoEdges()
{
    for ( int i = 0; i < nbrEdges; i++ )
    {
        edges[i].vert[0] = i;
        edges[i].vert[1] = i + 1;
    }
}
void LineMesh::updateAABB()
{
    AABB tempAABB;
    core::Vec3d minOffset( -2.0, -2.0, -2.0 );
    core::Vec3d maxOffset( 1.0, 1.0, 1.0 );
    core::Vec3d minEdgeOffset( -0.1, -0.1, -0.1 );
    core::Vec3d maxEdgeOffset( 0.1, 0.1, 0.1 );

    tempAABB.aabbMin[0] = std::numeric_limits<double>::max();
    tempAABB.aabbMin[1] = std::numeric_limits<double>::max();
    tempAABB.aabbMin[2] = std::numeric_limits<double>::max();

    tempAABB.aabbMax[0] = -std::numeric_limits<double>::max();
    tempAABB.aabbMax[1] = -std::numeric_limits<double>::max();
    tempAABB.aabbMax[2] = -std::numeric_limits<double>::max();

    for ( int i = 0; i < nbrEdges; i++ )
    {
        ///min
        edgeAABBs[i].aabbMin[0] = std::min( vertices[edges[i].vert[0]][0], vertices[edges[i].vert[1]][0] );
        edgeAABBs[i].aabbMin[1] = std::min( vertices[edges[i].vert[0]][1], vertices[edges[i].vert[1]][1] );
        edgeAABBs[i].aabbMin[2] = std::min( vertices[edges[i].vert[0]][2], vertices[edges[i].vert[1]][2] );
        edgeAABBs[i].aabbMin += minEdgeOffset;
        tempAABB.aabbMin[0] = std::min( tempAABB.aabbMin[0], edgeAABBs[i].aabbMin[0] );
        tempAABB.aabbMin[1] = std::min( tempAABB.aabbMin[1], edgeAABBs[i].aabbMin[1] );
        tempAABB.aabbMin[2] = std::min( tempAABB.aabbMin[2], edgeAABBs[i].aabbMin[2] );

        ///max
        edgeAABBs[i].aabbMax[0] = std::max( vertices[edges[i].vert[0]][0], vertices[edges[i].vert[1]][0] );
        edgeAABBs[i].aabbMax[1] = std::max( vertices[edges[i].vert[0]][1], vertices[edges[i].vert[1]][1] );
        edgeAABBs[i].aabbMax[2] = std::max( vertices[edges[i].vert[0]][2], vertices[edges[i].vert[1]][2] );
        edgeAABBs[i].aabbMax += maxEdgeOffset;
        tempAABB.aabbMax[0] = std::max( tempAABB.aabbMax[0], edgeAABBs[i].aabbMax[0] );
        tempAABB.aabbMax[1] = std::max( tempAABB.aabbMax[1], edgeAABBs[i].aabbMax[1] );
        tempAABB.aabbMax[2] = std::max( tempAABB.aabbMax[2], edgeAABBs[i].aabbMax[2] );
    }

    tempAABB.aabbMin += minOffset;
    tempAABB.aabbMax += maxOffset;
    aabb = tempAABB;
}
void LineMesh::translate( float p_offsetX, float p_offsetY, float p_offsetZ )
{

    for ( int i = 0; i < nbrVertices; i++ )
    {
        vertices[i][0] = vertices[i][0] + p_offsetX;
        vertices[i][1] = vertices[i][1] + p_offsetY;
        vertices[i][2] = vertices[i][2] + p_offsetZ;
    }

    updateAABB();
}
void LineMesh::translate( core::Vec3d p_offset )
{

    for ( int i = 0; i < nbrVertices; i++ )
    {
        vertices[i] = vertices[i] + p_offset;
        origVerts[i] = origVerts[i] + p_offset;
    }

    updateAABB();
}
void LineMesh::rotate( Matrix33d p_rot )
{

    for ( int i = 0; i < nbrVertices; i++ )
    {
        vertices[i] = p_rot * vertices[i];
        origVerts[i] = p_rot * origVerts[i];
    }

    updateAABB();
}
void LineMesh::scale( core::Vec3d p_scaleFactors )
{

    for ( int i = 0; i < nbrVertices; i++ )
    {
        vertices[i][0] = vertices[i][0] * p_scaleFactors[0];
        vertices[i][1] = vertices[i][1] * p_scaleFactors[1];
        vertices[i][2] = vertices[i][2] * p_scaleFactors[2];

        origVerts[i][0] = origVerts[i][0] * p_scaleFactors[0];
        origVerts[i][1] = origVerts[i][1] * p_scaleFactors[1];
        origVerts[i][2] = origVerts[i][2] * p_scaleFactors[2];
    }

    updateAABB();
}

bool LineMesh::isMeshTextured()
{
    return isTextureCoordAvailable;
}

int Mesh::getNumTriangles() const
{
    return this->nbrTriangles;
}

int Mesh::getNumEdges() const
{
    return this->edges.size();
}

void Mesh::updateSurfaceMeshFromVegaFormat(std::shared_ptr<ObjMesh> vegaSurfaceMesh)
{
    Vec3d p;
    //copy the vertex co-ordinates
    for(int i=0; i<this->nbrVertices ; i++)
    {
       p = vegaSurfaceMesh->getPosition(i);
       this->vertices[i][0] = p[0];
       this->vertices[i][1] = p[1];
       this->vertices[i][2] = p[2];
    }
}

bool Mesh::importSurfaceMeshFromVegaFormat(std::shared_ptr<ObjMesh> vegaSurfaceMesh, const bool perProcessingStage)
{
    if (!vegaSurfaceMesh)
        return false;

    if(!vegaSurfaceMesh->isTriangularMesh())
    {
        if (this->log != nullptr)
        {
            this->log->addError("Error : SimMedTK supports only triangular surface mesh. Vega mesh is not a triangle mesh!");
            return false;
        }
    }

    int i, threeI;

    // temporary arrays
    int numVertices(0);
    double* vertices;
    int numTriangles(0);
    int* triangles;
    //Int * numGroups;
	//Int ** triangleGroups;

    vertices = nullptr;
    triangles = nullptr;
    vegaSurfaceMesh->exportGeometry(&numVertices, &vertices, &numTriangles , &triangles, nullptr, nullptr);

    this->nbrVertices = numVertices;
    this->nbrTriangles = numTriangles;

    initVertexArrays(numVertices);
    initTriangleArrays(numTriangles);

    /*delete this->triangles;
    this->triangles = new Triangle[this->nbrTriangles];*/

    //copy the triangle connectivity information
    for(i=0; i<this->nbrTriangles ; i++)
    {
        threeI = 3*i;
        this->triangles[i].vert[0] = triangles[threeI+0];
        this->triangles[i].vert[1] = triangles[threeI+1];
        this->triangles[i].vert[2] = triangles[threeI+2];
    }

    //this->vertices.resize(this->nbrVertices);
    //copy the vertex co-ordinates
    for(i=0; i<this->nbrVertices ; i++)
    {
        this->vertices[i][0] = vertices[3 * i + 0];
        this->vertices[i][1] = vertices[3 * i + 1];
        this->vertices[i][2] = vertices[3 * i + 2];
    }

    if(perProcessingStage){
        updateOriginalVertsWithCurrent();
    }

    //deallocate temporary arrays
    delete [] triangles;
    delete [] vertices;

    return 1;

}
