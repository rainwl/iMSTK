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

#include "imstkGridBasedNeighborSearch.h"
#include <g3log/g3log.hpp>

namespace imstk
{
template<class Real>
void GridBasedNeighborSearch<Real>::setSearchRadius(const Real radius)
{
    m_SearchRadius    = radius;
    m_SearchRadiusSqr = radius * radius;
}

template<class Real>
std::vector<std::vector<size_t>>
GridBasedNeighborSearch<Real>::getNeighbors(const StdVT_Vec3r& points)
{
    std::vector<std::vector<size_t>> result;
    getNeighbors(result, points, points);
    return result;
}

template<class Real>
void GridBasedNeighborSearch<Real>::getNeighbors(std::vector<std::vector<size_t>>&result, const StdVT_Vec3r& points)
{
    getNeighbors(result, points, points);
}

template<class Real>
void GridBasedNeighborSearch<Real>::getNeighbors(std::vector<std::vector<size_t>>&result, const StdVT_Vec3r& setA, const StdVT_Vec3r& setB)
{
    if(std::abs(m_SearchRadius) < Real(1e-8))
    {
        LOG(FATAL) << "Neighbor search radius is zero";
    }

    // firstly compute the bounding box of points in setB
    const auto hugeNumber = std::numeric_limits<Real>::max();
    Vec3r lowerCorner(hugeNumber, hugeNumber, hugeNumber);
    Vec3r upperCorner(-hugeNumber, -hugeNumber, -hugeNumber);

    for(auto& ppos : setB)
    {
        for(int d = 0; d < 3; ++d)
        {
            lowerCorner[d] = (ppos[d] < lowerCorner[d]) ? ppos[d] : lowerCorner[d];
            upperCorner[d] = (ppos[d] > upperCorner[d]) ? ppos[d] : upperCorner[d];
        }
    }

    // the upper corner need to be expanded a bit, to avoid round-off error during computation
    upperCorner += Vec3d(m_SearchRadius, m_SearchRadius, m_SearchRadius) * Real(0.1);

    // resize grid to fit the bounding box covering setB
    m_Grid.initialize(lowerCorner, upperCorner, m_SearchRadius);

    // clear all particle lists in each grid cell
    m_Grid.loopAllCellData([](auto& list) { list.resize(0); });

    // collect particle indices of points in setB into their corresponding cells
    for(size_t p = 0; p < setB.size(); ++p)
    {
        m_Grid.getCellData(setB[p]).push_back(p);
    }

    // for each point in setA, collect setB neighbors within the search radius
    result.resize(setA.size());
    for(size_t p = 0; p < setA.size(); ++p)
    {
        auto& pneighbors = result[p];

        // important: must clear the old result (if applicable)
        pneighbors.resize(0);

        const auto ppos    = setA[p];
        const auto cellIdx = m_Grid.template getCellIndexFromCoordinate<int>(ppos);

        for(int k = -1; k <= 1; ++k)
        {
            int cellZ = cellIdx[2] + k;
            if(!m_Grid.template isValidCellIndex<2>(cellZ))
            {
                continue;
            }
            for(int j = -1; j <= 1; ++j)
            {
                int cellY = cellIdx[1] + j;
                if(!m_Grid.template isValidCellIndex<1>(cellY))
                {
                    continue;
                }
                for(int i = -1; i <= 1; ++i)
                {
                    int cellX = cellIdx[0] + i;
                    if(!m_Grid.template isValidCellIndex<0>(cellX))
                    {
                        continue;
                    }

                    // get index q of point in setB
                    for(auto q : m_Grid.getCellData(cellX, cellY, cellZ))
                    {
                        const auto qpos = setB[q];
                        const auto d2   = (ppos - qpos).squaredNorm();
                        if(d2 < m_SearchRadiusSqr)
                        {
                            pneighbors.push_back(q);
                        }
                    }
                }
            }
        }
    }
}
} // end namespace imstk

// Explicit instantiate class GridBasedNeighborSearch
template class imstk::GridBasedNeighborSearch<double>;