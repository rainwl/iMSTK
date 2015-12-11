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
#include "Solvers/ForwardGaussSeidel.h"
#include "Solvers/BackwardGaussSeidel.h"
#include "Solvers/ForwardSOR.h"
#include "Solvers/BackwardSOR.h"
#include "Solvers/DirectLinearSolver.h"
#include "Solvers/ConjugateGradient.h"
#include "Core/Matrix.h"
#include "Testing/ReadSparseMatrix.h"

using namespace bandit;
using namespace core;

go_bandit([]()
{
    describe("Solver", []()
    {
        SparseMatrixd A;
        Vectord b;
        ReadSparseMatrix("../Testing/MatrixData/bcsstk27.mtx", A);
        Matrixd Adense(A);
        b.setRandom(A.cols());
        Vectord solution(A.cols());
        auto BGSSolver = std::make_shared<BackwardGaussSeidel>(A, b);
        auto BSORSolver = std::make_shared<BackwardSOR>(A, b);
        auto CGSolver = std::make_shared<ConjugateGradient>(A, b);
        auto DDSolver = std::make_shared<DirectLinearSolver<Matrixd>>(Adense, b);
        auto DSSolver = std::make_shared<DirectLinearSolver<SparseMatrixd>>(A, b);
        auto FGSSolver = std::make_shared<ForwardGaussSeidel>(A, b);
        auto FSORSolver = std::make_shared<ForwardSOR>(A, b);
        it("constructs ", [&]()
        {
            AssertThat(BGSSolver != nullptr, IsTrue());
            AssertThat(BSORSolver != nullptr, IsTrue());
            AssertThat(CGSolver != nullptr, IsTrue());
            AssertThat(DDSolver != nullptr, IsTrue());
            AssertThat(DSSolver != nullptr, IsTrue());
            AssertThat(FGSSolver != nullptr, IsTrue());
            AssertThat(FSORSolver != nullptr, IsTrue());
        });
        it("CG solves", [&]()
        {
            solution.setRandom();
            CGSolver->setMaximumIterations(A.cols());
            CGSolver->setTolerance(1e-12);
            CGSolver->solve(solution);
            AssertThat(CGSolver->getResidual().norm(), IsLessThan(1e-8));
        });
        it("ForwardGaussSeidel solves", [&]()
        {
            solution.setRandom();
            FGSSolver->setMaximumIterations(100);
            FGSSolver->setTolerance(1e-12);
            FGSSolver->solve(solution);
            AssertThat(FGSSolver->getResidual().norm(), IsLessThan(1e-10));
        });
        it("BackwardGaussSeidel solves", [&]()
        {
            solution.setRandom();
            BGSSolver->setMaximumIterations(A.cols());
            BGSSolver->setTolerance(1e-12);
            BGSSolver->solve(solution);
            AssertThat(BGSSolver->getResidual().norm(), IsLessThan(1e-10));
        });
        it("ForwardSOR solves", [&]()
        {
            solution.setRandom();
            BGSSolver->setMaximumIterations(A.cols());
            BGSSolver->setTolerance(1e-12);
            BGSSolver->solve(solution);
            AssertThat(BGSSolver->getResidual().norm(), IsLessThan(1e-10));
        });
        it("BackwardSOR solves", [&]()
        {
            solution.setRandom();
            BGSSolver->setMaximumIterations(A.cols());
            BGSSolver->setTolerance(1e-12);
            BGSSolver->solve(solution);
            AssertThat(BGSSolver->getResidual().norm(), IsLessThan(1e-10));
        });
        it("DirectLinearSolver solves dense system", [&]()
        {
            solution.setRandom();
            DDSolver->solve(solution);
            AssertThat((b - A * solution).norm(), IsLessThan(1e-10));
        });
        it("DirectLinearSolver solves sparse system", [&]()
        {
            solution.setRandom();
            DSSolver->solve(solution);
            AssertThat((b - A * solution).norm(), IsLessThan(1e-10));
        });
    });
});
