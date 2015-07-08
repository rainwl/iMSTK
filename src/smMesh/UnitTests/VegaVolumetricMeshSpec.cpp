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

#include <bandit/bandit.h>
#include <memory>

// SimMedTK includes
#include "smMesh/smVegaVolumetricMesh.h"
#include "smMesh/smSurfaceMesh.h"

using namespace bandit;

go_bandit([](){

    describe("Vega Volumetric Mesh", []() {
        it("constructs", []() {
            std::shared_ptr<smVegaVolumetricMesh> vegaMesh = std::make_shared<smVegaVolumetricMesh>();
            AssertThat(vegaMesh != nullptr, IsTrue());
        });
        it("loads mesh", []() {
            int verbose = 1;
            bool generateGraph = false;
            std::shared_ptr<smVegaVolumetricMesh> vegaMesh = std::make_shared<smVegaVolumetricMesh>(generateGraph);
            vegaMesh->loadMesh("/home/rortiz/Projects/SimMedTK/build-clang/SimMedTK-build/src/smMesh/UnitTests/SampleMesh.veg",verbose);
            AssertThat(vegaMesh->getVegaMesh() != nullptr, IsTrue());
        });
        it("generates graph", []() {
            int verbose = 1;
            bool generateGraph = true;
            std::shared_ptr<smVegaVolumetricMesh> vegaMesh = std::make_shared<smVegaVolumetricMesh>(generateGraph);
            vegaMesh->loadMesh("/home/rortiz/Projects/SimMedTK/build-clang/SimMedTK-build/src/smMesh/UnitTests/SampleMesh.veg",verbose);
            AssertThat(vegaMesh->getMeshGraph() != nullptr, IsTrue());
        });
        it("attaches surface mesh", []() {
            int verbose = 1;
            bool generateGraph = false;
            std::shared_ptr<smVegaVolumetricMesh> vegaMesh = std::make_shared<smVegaVolumetricMesh>(generateGraph);
            vegaMesh->loadMesh("/home/rortiz/Projects/SimMedTK/build-clang/SimMedTK-build/src/smMesh/UnitTests/SampleMesh.veg",verbose);

            std::shared_ptr<smSurfaceMesh> surfaceMesh = std::make_shared<smSurfaceMesh>();
            surfaceMesh->vertices.emplace_back(-2.44627, -0.903874999999999, -1.711465);
            surfaceMesh->vertices.emplace_back(-2.008655, -0.762779999999999, -1.63081);
            surfaceMesh->vertices.emplace_back(-2.248035, -0.599385, -1.41836);

            vegaMesh->attachSurfaceMesh(surfaceMesh,2.0);

            auto &weights = vegaMesh->getAttachedWeights(0);
            auto &vertices = vegaMesh->getAttachedVertices(0);

            AssertThat(weights[0] == 1 && weights[5] == 1 && weights[10] == 1, IsTrue());
        });
    });
});
