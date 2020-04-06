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

#include "imstkLogUtility.h"
#include "imstkMath.h"
#include "imstkParallelUtils.h"

#include <vtkSmartPointer.h>

#include <memory>
#include <numeric>
#include <queue>
#include <unordered_set>
#include <set>

class vtkCellArray;
class vtkPolyData;
class vtkPointData;
class vtkPoints;
class vtkUnstructuredGrid;

namespace imstk
{
class HexahedralMesh;
class LineMesh;
class SurfaceMesh;
class TetrahedralMesh;
class VolumetricMesh;

namespace GeometryUtils
{
///
/// \brief Converts vtk polydata into a imstk surface mesh
///
std::unique_ptr<SurfaceMesh> convertVtkPolyDataToSurfaceMesh(const vtkSmartPointer<vtkPolyData> vtkMesh);

///
/// \brief Converts vtk polydata into a imstk surface mesh
///
std::unique_ptr<LineMesh> convertVtkPolyDataToLineMesh(const vtkSmartPointer<vtkPolyData> vtkMesh);

///
/// \brief Get imstk volumetric mesh given vtkUnstructuredGrid as input
///
std::unique_ptr<VolumetricMesh> convertVtkUnstructuredGridToVolumetricMesh(const vtkSmartPointer<vtkUnstructuredGrid> vtkMesh);

///
/// \brief Converts imstk surface mesh into a vtk polydata
///
vtkSmartPointer<vtkPolyData> convertSurfaceMeshToVtkPolyData(const SurfaceMesh& imstkMesh);

///
/// \brief Converts imstk line mesh into a vtk polydata
///
vtkSmartPointer<vtkPolyData> convertLineMeshToVtkPolyData(const LineMesh& imstkMesh);

///
/// \brief Converts imstk tetrahedral mesh into a vtk unstructured grid
///
vtkSmartPointer<vtkUnstructuredGrid> convertTetrahedralMeshToVtkUnstructuredGrid(const TetrahedralMesh& imstkMesh);

///
/// \brief Converts imstk hexahedral mesh into a vtk unstructured grid
///
vtkSmartPointer<vtkUnstructuredGrid> convertHexahedralMeshToVtkUnstructuredGrid(const HexahedralMesh& imstkMesh);

///
/// \brief Copy from vtk points to a imstk vertices array (StdVectorOfVec3d)
///
void copyVerticesFromVtk(vtkPoints* const points, StdVectorOfVec3d& vertices);

///
/// \brief Copies vertices from imstk structure to VTK one
///
void copyVerticesToVtk(const StdVectorOfVec3d& vertices, vtkPoints* points);

///
/// \brief Copies cells of the given dimension from imstk structure to VTK one
///
template<size_t dim>
void copyCellsToVtk(const std::vector<std::array<size_t, dim>>& cells, vtkCellArray* vtkCells);

///
/// \brief
///
template<size_t dim>
void copyCellsFromVtk(vtkCellArray* vtkCells, std::vector<std::array<size_t, dim>>& cells);

///
/// \brief
///
void copyPointDataFromVtk(vtkPointData* const pointData, std::map<std::string, StdVectorOfVectorf>& dataMap);

///
/// \brief Combines two input surface meshes
/// Refer <a href="https://vtk.org/doc/nightly/html/classvtkAppendPolyData.html#details">vtkAppendPolyData</a> class
/// for more details
///
///
std::unique_ptr<SurfaceMesh> combineSurfaceMesh(const SurfaceMesh& surfaceMesh1, const SurfaceMesh& surfaceMesh2);

///
/// \brief Converts an imstk SurfaceMesh to a LineMesh, removing duplicate edges. Cell indices not preserved
/// Refer <a href="https://vtk.org/doc/nightly/html/classvtkExtractEdges.html#details">vtkExtractEdges</a> class
/// for more details
///
std::unique_ptr<LineMesh> surfaceMeshToLineMesh(const SurfaceMesh& surfaceMesh);

///
/// \brief Config for smooth polydata filter
///
struct smoothPolydataConfig
{
    int numberOfIterations  = 20;
    double relaxationFactor = 0.01;
    double convergence      = 0.0;
    double featureAngle     = 45.0;
    double edgeAngle = 15.0;
    bool featureEdgeSmoothing = false;
    bool boundarySmoothing    = true;
};

///
/// \brief Smooths a SurfaceMesh using laplacian smoothening
/// Refer <a href="https://vtk.org/doc/nightly/html/classvtkSmoothPolyDataFilter.html#details">vtkSmoothPolyDataFilter</a>
/// for more details
///
std::unique_ptr<SurfaceMesh> smoothSurfaceMesh(const SurfaceMesh&          surfaceMesh,
                                               const smoothPolydataConfig& c);

///
/// \brief Sub-divdes a SurfaceMesh using linear subdivision
/// Refer <a href="https://vtk.org/doc/nightly/html/classvtkLinearSubdivisionFilter.html#details">vtk linear subdivision</a>
/// for more details
///
std::unique_ptr<SurfaceMesh> linearSubdivideSurfaceMesh(const SurfaceMesh& surfaceMesh, const int numSubdivisions = 1);

///
/// \brief Sub-divides an input imstk SurfaceMesh using loop subdivision algorithm
/// Refer <a href="https://vtk.org/doc/nightly/html/classvtkLoopSubdivisionFilter.html#details">vtk loop subdivision</a>
/// for more details
///
std::unique_ptr<SurfaceMesh> loopSubdivideSurfaceMesh(const SurfaceMesh& surfaceMesh, const int numSubdivisions = 1);

///
/// \brief Create a tetrahedral mesh based on a uniform Cartesian mesh
/// \param aabbMin  the small conner of a box
/// \param aabbMax  the large conner of a box
/// \param nx number of elements in the x-direction
/// \param ny number of elements in the y-direction
/// \param nz number of elements in the z-direction
///
/// \note Refer: Dompierre, Julien & Labbé, Paul & Vallet, Marie-Gabrielle & Camarero, Ricardo. (1999).
/// How to Subdivide Pyramids, Prisms, and Hexahedra into Tetrahedra.. 195-204.
std::shared_ptr<TetrahedralMesh> createUniformMesh(const Vec3d& aabbMin, const Vec3d& aabbMax, const size_t nx, const size_t ny, const size_t nz);

///
/// \brief Create a tetrahedral mesh cover
///
std::shared_ptr<TetrahedralMesh> createTetrahedralMeshCover(const SurfaceMesh& surfMesh, const size_t nx, const size_t ny, size_t nz);

///
/// \brief Enumeration for reordering method
///
enum class MeshNodeRenumberingStrategy
{
    ReverseCuthillMckee // Reverse Cuthill-Mckee
};

///
/// \brief Reorder indices in a connectivity to reduce bandwidth
///
/// \param[in] neighbors array of neighbors of each vertex; eg, neighbors[i] is an object containing all neighbors of vertex-i
/// \param[i] method reordering method; see \ref ReorderMethod
///
/// \return the permutation vector that map from new indices to old indices
///
template<typename NeighborContainer>
std::vector<size_t> reorderConnectivity(const std::vector<NeighborContainer>& neighbors, const MeshNodeRenumberingStrategy& method = MeshNodeRenumberingStrategy::ReverseCuthillMckee);

///
/// \brief Reorder using Reverse Cuthill-Mckee
///
/// \param[in] conn element-to-vertex connectivity
/// \param[in] numVerts number of vertices
/// \param[in] method reordering method; see \ref ReorderMethod
///
/// \return the permutation vector that maps from new indices to old indices
///
template<typename ElemConn>
std::vector<size_t> reorderConnectivity(const std::vector<ElemConn>& conn, const size_t numVerts, const MeshNodeRenumberingStrategy& method = MeshNodeRenumberingStrategy::ReverseCuthillMckee);
} // namespace GeometryUtils
} // namespace imstk