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

#include <iostream>

#include "imstkNewtonSolver.h"
#include "imstkIterativeLinearSolver.h"
#include "imstkConjugateGradient.h"

namespace imstk
{

NewtonSolver::NewtonSolver():
    linearSolver(std::make_shared<ConjugateGradient>()),
    forcingTerm(0.9),
    absoluteTolerance(1e-3),
    relativeTolerance(1e-6),
    gamma(0.9),
    etaMax(0.9),
    maxIterations(50),
    useArmijo(true) {}


void
NewtonSolver::solveGivenState(Vectord& x)
{
    if(!this->m_nonLinearSystem)
    {
        LOG(WARNING) << "NewtonMethod::solve - nonlinear system is not set to the nonlinear solver";
        return;
    }

    // Compute norms, set tolerances and other temporaries
    double fnorm = this->m_nonLinearSystem->evaluateF(x).norm();
    double stopTolerance = this->absoluteTolerance + this->relativeTolerance * fnorm;

    this->linearSolver->setTolerance(stopTolerance);

    Vectord dx = x;

    for(size_t i = 0; i < this->maxIterations; ++i)
    {
        if(fnorm < stopTolerance)
        {
            return;
        }
        this->updateJacobian(x);
        this->linearSolver->solve(dx);
        this->m_updateIterate(-dx,x);

        double newNorm = fnorm;

        newNorm = this->armijo(dx, x, fnorm);

        if(this->forcingTerm > 0.0 && newNorm > stopTolerance)
        {
            double ratio = newNorm / fnorm; // Ratio of successive residual norms
            this->updateForcingTerm(ratio, stopTolerance, fnorm);

            // Reset tolerance in the linear solver according to the new forcing term
            // to avoid over solving of the system.
            this->linearSolver->setTolerance(this->forcingTerm);
        }

        fnorm = newNorm;
    }
}

void
NewtonSolver::solve()
{
    if (!this->m_nonLinearSystem)
    {
        LOG(WARNING) << "NewtonMethod::solve - nonlinear system is not set to the nonlinear solver";
        return;
    }

    auto u = this->m_nonLinearSystem->getUnknownVector();
    Vectord du = u;
    du.setZero();

    for (size_t i = 0; i < 1; ++i)
    {
        du.setZero();
        this->updateJacobian(u);
        this->linearSolver->solve(du);
        u -= du;
        this->m_nonLinearSystem->m_FUpdate(u);
    }
}

void
NewtonSolver::updateJacobian(const Vectord& x)
{
    // Evaluate the Jacobian and sets the matrix
    if (!this->m_nonLinearSystem)
    {
        LOG(WARNING) << "NewtonMethod::updateJacobian - nonlinear system is not set to the nonlinear solver";
        return;
    }

    auto &b = this->m_nonLinearSystem->m_F(x);
    auto &A = this->m_nonLinearSystem->m_dF(x);

    if (A.innerSize() == 0)
    {
        LOG(WARNING) << "NewtonMethod::updateJacobian - Size of matrix is 0!";
        return;
    }

    auto linearSystem = std::make_shared<LinearSolverType::LinearSystemType>(A, b);
    this->linearSolver->setSystem(linearSystem);
}

void
NewtonSolver::updateForcingTerm(const double ratio, const double stopTolerance, const double fnorm)
{
    double eta = this->gamma * ratio * ratio;
    double forcingTermSqr = this->forcingTerm * this->forcingTerm;

    // Save guard to prevent the forcing term to become too small for far away iterates
    if(this->gamma * forcingTermSqr > .1)
    {
        // TODO: Log this
        eta = std::max(eta, this->gamma * forcingTermSqr);
    }

    this->forcingTerm = std::max(std::min(eta, this->etaMax), 0.5 * stopTolerance / fnorm);
}


void
NewtonSolver::setLinearSolver(std::shared_ptr< NewtonSolver::LinearSolverType > newLinearSolver)
{
    this->linearSolver = newLinearSolver;
}


std::shared_ptr<NewtonSolver::LinearSolverType>
NewtonSolver::getLinearSolver() const
{
    return this->linearSolver;
}


void
NewtonSolver::setAbsoluteTolerance(const double aTolerance)
{
    this->absoluteTolerance = aTolerance;
}


double
NewtonSolver::getAbsoluteTolerance() const
{
    return this->absoluteTolerance;
}

} // imstk