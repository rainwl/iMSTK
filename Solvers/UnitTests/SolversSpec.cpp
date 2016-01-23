// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#include <bandit/bandit.h>
#include <memory>
#include <iostream>

// SimMedTK includes
#include "Solvers/ForwardGaussSeidel.h"
#include "Solvers/BackwardGaussSeidel.h"
#include "Solvers/ForwardSOR.h"
#include "Solvers/BackwardSOR.h"
#include "Solvers/DirectLinearSolver.h"
#include "Solvers/ConjugateGradient.h"
#include "Solvers/NewtonMethod.h"
#include "Core/Matrix.h"
#include "Testing/ReadSparseMatrix.h"
#include "Testing/ReadPaths.h"

using namespace bandit;
using namespace core;


auto paths = imstk::ReadPaths("./SolversConfig.paths");

const std::array<std::string,4> matrixFileNames =
{
    std::get<imstk::Path::Binary>(paths)+"/662_bus.mtx",
    std::get<imstk::Path::Binary>(paths)+"/494_bus.mtx",
    std::get<imstk::Path::Binary>(paths)+"/685_bus.mtx",
    std::get<imstk::Path::Binary>(paths)+"/1138_bus.mtx",
};

template<typename SolverType>
void SolveSystem(std::shared_ptr<SolverType> S, double epsilon = 1e-12)
{
    S->setTolerance(epsilon);
    for (int i = 0; i < matrixFileNames.size(); ++i)
    {
        SparseMatrixd A,AT;
        Vectord b,solution;
        ReadSparseMatrix(matrixFileNames[i],A);
        for (int k = 0; k < A.outerSize(); ++k)
        {
            A.coeffRef(k,k) *= 50;
        }
        solution.setZero(A.cols());
        b.setRandom(A.cols());
        auto system = std::make_shared<LinearSystem<SparseMatrixd>>(A,b);
        S->setSystem(system);
        S->solve(solution);
        AssertThat(S->getError(solution)/b.squaredNorm(), IsLessThan(epsilon));
    }
}


go_bandit([]()
{
    SparseMatrixd A;
    Vectord b;
    ReadSparseMatrix(matrixFileNames[0], A);
    Matrixd Adense(A);
    b.setRandom(A.cols());
    Vectord solution(A.cols());

    const double linearEspsilon= 1e-12;
    const double nonLinearEpsilon = 1e-12;
    describe("Backward Gauss-Seidel solver", [&]()
    {
        auto BGSSolver = std::make_shared<BackwardGaussSeidel>();
        it("constructs ", [&]()
        {
            AssertThat(BGSSolver != nullptr, IsTrue());
        });
        it("solves", [&]()
        {
            BGSSolver->setMaximumIterations(A.cols());
            SolveSystem(BGSSolver,linearEspsilon);
        });

    });

    describe("Forward Gauss-Seidel linear solver",[&]()
    {
        auto FGSSolver = std::make_shared<ForwardGaussSeidel>();
        it("constructs ", [&]()
        {
            AssertThat(FGSSolver != nullptr, IsTrue());
        });

        it("solves", [&]()
        {
            FGSSolver->setMaximumIterations(100);
            SolveSystem(FGSSolver,linearEspsilon);
        });
    });

    describe("Conjugate gradient solver",[&]()
    {
        auto CGSolver = std::make_shared<ConjugateGradient>();
        it("constructs ", [&]()
        {
            AssertThat(CGSolver != nullptr, IsTrue());
        });

        it("solves", [&]()
        {
            CGSolver->setMaximumIterations(A.cols());
            SolveSystem(CGSolver,linearEspsilon);
        });
    });

    describe("Backward SOR",[&]()
    {
        auto BSORSolver = std::make_shared<BackwardSOR>();
        it("constructs ", [&]()
        {
            AssertThat(BSORSolver != nullptr, IsTrue());
        });

        it("solves", [&]()
        {
            BSORSolver->setMaximumIterations(A.cols());
            SolveSystem(BSORSolver,linearEspsilon);
        });
    });

    describe("Forward SOR",[&]()
    {
        auto FSORSolver = std::make_shared<ForwardSOR>();
        it("constructs ", [&]()
        {
            AssertThat(FSORSolver != nullptr, IsTrue());
        });

        it("solves", [&]()
        {
            FSORSolver->setMaximumIterations(A.cols());
            SolveSystem(FSORSolver,linearEspsilon);
        });
    });

    describe("Direct solver for dense systems",[&]()
    {
        auto DDSolver = std::make_shared<DirectLinearSolver<Matrixd>>(Adense, b);
        it("constructs ", [&]()
        {
            AssertThat(DDSolver != nullptr, IsTrue());
        });

        it("solves", [&]()
        {
            solution.setRandom();
            DDSolver->solve(solution);
            AssertThat((b - A * solution).norm(), IsLessThan(linearEspsilon));
        });
    });

    auto DSSolver = std::make_shared<DirectLinearSolver<SparseMatrixd>>(A, b);
    describe("Direct solver for sparse systems (With Sparse LU)",[&]()
    {
        it("constructs ", [&]()
        {
            AssertThat(DSSolver != nullptr, IsTrue());
        });

        it("solves", [&]()
        {
            solution.setRandom();
            DSSolver->solve(solution);
            AssertThat((b - A * solution).norm(), IsLessThan(linearEspsilon));
        });
    });

    describe("Newton non-linear solver",[&]()
    {
        auto NewtonSolver = std::make_shared<NewtonMethod>();
        it("constructs ", [&]()
        {
            AssertThat(NewtonSolver != nullptr, IsTrue());
        });

        it("solves", [&]()
        {
            core::Vectord y(2);
            auto F = [&](const Vectord &x) -> const Vectord&
            {
                y(0) = (x(0)+3)*(x(1)*x(1)*x(1)-7) + 18;
                y(1) = std::sin(x(1)*std::exp(x(0))-1);
                return y;
            };

            std::vector<Eigen::Triplet<double>> tripletList;
            core::SparseMatrixd J(2,2);
            auto DF = [&](const Vectord &x) -> const core::SparseMatrixd&
            {
                core::Matrixd M(2,2);
                M(0,0) = x(1)*x(1)*x(1)-7;
                M(0,1) = 3*x(1)*x(1)*(x(0)+3);
                M(1,0) = x(1)*std::exp(x(0))*std::cos(x(1)*std::exp(x(0))-1);
                M(1,1) = std::exp(x(0))*std::cos(x(1)*std::exp(x(0))-1);

                J = M.sparseView();
                return J;
            };

            NewtonSolver->setSystem(F);
            NewtonSolver->setAbsoluteTolerance(nonLinearEpsilon);
            NewtonSolver->setRelativeTolerance(nonLinearEpsilon);
            NewtonSolver->setJacobian(DF);
            NewtonSolver->setLinearSolver(DSSolver);

            Vectord x(2);
            x.setZero();
            NewtonSolver->solve(x);

            AssertThat(F(x).norm(), IsLessThan(NewtonSolver->getAbsoluteTolerance()));

        });
    });
});