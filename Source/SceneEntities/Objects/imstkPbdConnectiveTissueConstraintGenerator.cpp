/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCollider.h"
#include "imstkCollisionUtils.h"
#include "imstkEntity.h"
#include "imstkLineMesh.h"
#include "imstkPbdBaryPointToPointConstraint.h"
#include "imstkPbdConnectiveTissueConstraintGenerator.h"
#include "imstkPbdConstraintFunctor.h"
#include "imstkPbdMethod.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdSystem.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkTriangleToTetMap.h"
#include "imstkVisualModel.h"

#include "imstkProximitySurfaceSelector.h"
#include "imstkConnectiveStrandGenerator.h"

namespace imstk
{
void
PbdConnectiveTissueConstraintGenerator::connectLineToTetMesh(std::shared_ptr<PbdMethod> pbdObj, PbdConstraintContainer& constraints)
{
    auto                         tetMesh  = std::dynamic_pointer_cast<TetrahedralMesh>(pbdObj->getPhysicsGeometry());
    std::shared_ptr<SurfaceMesh> surfMesh = tetMesh->extractSurfaceMesh();

    // Setup a map to figure out what tet the tri is from for attachment to the tet
    TriangleToTetMap triToTetMap;
    triToTetMap.setParentGeometry(tetMesh);
    triToTetMap.setChildGeometry(surfMesh);
    triToTetMap.setTolerance(m_tolerance);
    triToTetMap.compute();

    auto lineMesh = std::dynamic_pointer_cast<LineMesh>(m_connectiveStrandObj->getPhysicsGeometry());
    // Find all vertices of the line mesh that are coincident with the surface of mesh A
    int verticesConnected = 0;
    for (int vertId = 0; vertId < lineMesh->getNumVertices(); vertId++)
    {
        const Vec3d vertexPosition = lineMesh->getVertexPosition(vertId);
        double      minSqrDist     = IMSTK_FLOAT_MAX;

        int nearestTriangleId = -1;
        for (int triId = 0; triId < surfMesh->getNumCells(); triId++)
        {
            const Vec3d& x1 = surfMesh->getVertexPosition(surfMesh->getCells()->at(triId)[0]);
            const Vec3d& x2 = surfMesh->getVertexPosition(surfMesh->getCells()->at(triId)[1]);
            const Vec3d& x3 = surfMesh->getVertexPosition(surfMesh->getCells()->at(triId)[2]);

            int         ptOnTriangleCaseType;
            const Vec3d closestPtOnTri = CollisionUtils::closestPointOnTriangle(vertexPosition,
                x1, x2, x3, ptOnTriangleCaseType);

            const double sqrDist = (closestPtOnTri - vertexPosition).squaredNorm();
            if (sqrDist < minSqrDist)
            {
                minSqrDist = sqrDist;
                nearestTriangleId = triId;
            }
        }

        // If the vertex is not on the surface mesh, ignore it.
        if (minSqrDist > m_tolerance)
        {
            continue;
        }

        verticesConnected++;

        const int   tetId   = triToTetMap.getParentTetId(nearestTriangleId);
        const Vec4d weights = tetMesh->computeBarycentricWeights(tetId, vertexPosition);
        const int   objId   = pbdObj->getPbdBody()->bodyHandle;

        // Constraint between point on tet to the vertex
        auto                       vertToTetConstraint = std::make_shared<PbdBaryPointToPointConstraint>();
        std::vector<PbdParticleId> ptsA = {
            { objId, (*tetMesh->getCells())[tetId][0] },
            { objId, (*tetMesh->getCells())[tetId][1] },
            { objId, (*tetMesh->getCells())[tetId][2] },
            { objId, (*tetMesh->getCells())[tetId][3] } };

        std::vector<double> weightsA = { weights[0], weights[1], weights[2], weights[3] };

        // Ligament vertex end on the gallblader
        std::vector<PbdParticleId> ptsB     = { { m_connectiveStrandObj->getPbdBody()->bodyHandle, vertId } };
        std::vector<double>        weightsB = { 1.0 };
        vertToTetConstraint->initConstraint(ptsA, weightsA, ptsB, weightsB, 0.8, 0.8);
        constraints.addConstraint(vertToTetConstraint);
    }
}

void
PbdConnectiveTissueConstraintGenerator::connectLineToSurfMesh(
    std::shared_ptr<PbdMethod> pbdObj,
    PbdConstraintContainer&    constraints)
{
    auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(pbdObj->getPhysicsGeometry());
    auto lineMesh = std::dynamic_pointer_cast<LineMesh>(m_connectiveStrandObj->getPhysicsGeometry());

    // Find all vertices of the line mesh that are coincident with the surface of mesh A
    int verticesConnected = 0;
    for (int vertId = 0; vertId < lineMesh->getNumVertices(); vertId++)
    {
        const Vec3d vertexPosition = lineMesh->getVertexPosition(vertId);
        double      minSqrDist     = IMSTK_FLOAT_MAX;

        int nearestTriangleId = -1;
        for (int triId = 0; triId < surfMesh->getNumCells(); triId++)
        {
            const Vec3d& x1 = surfMesh->getVertexPosition(surfMesh->getCells()->at(triId)[0]);
            const Vec3d& x2 = surfMesh->getVertexPosition(surfMesh->getCells()->at(triId)[1]);
            const Vec3d& x3 = surfMesh->getVertexPosition(surfMesh->getCells()->at(triId)[2]);

            int         ptOnTriangleCaseType;
            const Vec3d closestPtOnTri = CollisionUtils::closestPointOnTriangle(vertexPosition,
                x1, x2, x3, ptOnTriangleCaseType);

            const double sqrDist = (closestPtOnTri - vertexPosition).squaredNorm();
            if (sqrDist < minSqrDist)
            {
                minSqrDist = sqrDist;
                nearestTriangleId = triId;
            }
        }

        // If the vertex is not on the surface mesh, ignore it.
        if (minSqrDist > m_tolerance)
        {
            continue;
        }

        verticesConnected++;

        const Vec3d weights = surfMesh->computeBarycentricWeights(nearestTriangleId, vertexPosition);
        const int   objId   = pbdObj->getPbdBody()->bodyHandle;

        // Constraint between point on surface triangle to the vertex
        auto                       vertToTriConstraint = std::make_shared<PbdBaryPointToPointConstraint>();
        std::vector<PbdParticleId> ptsA = {
            { objId, (*surfMesh->getCells())[nearestTriangleId][0] },
            { objId, (*surfMesh->getCells())[nearestTriangleId][1] },
            { objId, (*surfMesh->getCells())[nearestTriangleId][2] } };

        std::vector<double> weightsA = { weights[0], weights[1], weights[2] };

        // Ligament vertex end on the gallblader
        std::vector<PbdParticleId> ptsB     = { { m_connectiveStrandObj->getPbdBody()->bodyHandle, vertId } };
        std::vector<double>        weightsB = { 1.0 };
        vertToTriConstraint->initConstraint(ptsA, weightsA, ptsB, weightsB, 0.8, 0.8);
        constraints.addConstraint(vertToTriConstraint);
    }
}

void
PbdConnectiveTissueConstraintGenerator::generateDistanceConstraints()
{
    m_connectiveStrandObj->getPbdSystem()->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, m_distStiffness,
        m_connectiveStrandObj->getPbdBody()->bodyHandle);
}

void
PbdConnectiveTissueConstraintGenerator::operator()(PbdConstraintContainer& constraints)
{
    auto objAPhysMeshSurf = std::dynamic_pointer_cast<SurfaceMesh>(m_objA->getPhysicsGeometry());
    if (objAPhysMeshSurf != nullptr)
    {
        connectLineToSurfMesh(m_objA, constraints);
    }

    auto objAPhysMeshTet = std::dynamic_pointer_cast<TetrahedralMesh>(m_objA->getPhysicsGeometry());
    if (objAPhysMeshTet != nullptr)
    {
        connectLineToTetMesh(m_objA, constraints);
    }

    auto objBPhysMeshSurf = std::dynamic_pointer_cast<SurfaceMesh>(m_objB->getPhysicsGeometry());
    if (objBPhysMeshSurf != nullptr)
    {
        connectLineToSurfMesh(m_objB, constraints);
    }

    auto objBPhysMeshTet = std::dynamic_pointer_cast<TetrahedralMesh>(m_objB->getPhysicsGeometry());
    if (objBPhysMeshTet != nullptr)
    {
        connectLineToTetMesh(m_objB, constraints);
    }
}

std::shared_ptr<Entity>
addConnectiveTissueConstraints(
    std::shared_ptr<LineMesh>  connectiveLineMesh,
    std::shared_ptr<PbdMethod> objA,
    std::shared_ptr<PbdMethod> objB,
    std::shared_ptr<PbdSystem> pbdSystem)
{
    // Check inputs
    CHECK(connectiveLineMesh != nullptr) << "NULL line mesh passes to generateConnectiveTissueConstraints";
    CHECK(objA != nullptr) << "PbdMethod objA is NULL in generateConnectiveTissueConstraints";
    CHECK(objB != nullptr) << "PbdMethod objB is NULL in generateConnectiveTissueConstraints";
    CHECK(pbdSystem != nullptr) << "PbdSystem is NULL in generateConnectiveTissueConstraints";

    // auto connectiveStrands = std::make_shared<PbdMethod>("connectiveTissue");
    auto connectiveStrands = std::make_shared<Entity>("connectiveTissue");

    auto method = std::make_shared<PbdMethod>();
    method->setPhysicsGeometry(connectiveLineMesh);
    method->setPbdSystem(pbdSystem);
    connectiveStrands->addComponent(method);

    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(connectiveLineMesh);
    connectiveStrands->addComponent(visualModel);

    // Setup the Object
    auto collider = connectiveStrands->addComponent<Collider>();
    collider->setGeometry(connectiveLineMesh);

    double mass = 1.0;
    method->getPbdBody()->uniformMassValue = mass / connectiveLineMesh->getNumVertices();

    // Setup constraints between the gallblader and ligaments
    auto attachmentConstraintFunctor = std::make_shared<PbdConnectiveTissueConstraintGenerator>();
    attachmentConstraintFunctor->setConnectiveStrandObj(method);
    attachmentConstraintFunctor->generateDistanceConstraints();
    attachmentConstraintFunctor->setConnectedObjA(objA);
    attachmentConstraintFunctor->setConnectedObjB(objB);
    attachmentConstraintFunctor->setBodyIndex(method->getPbdBody()->bodyHandle);
    pbdSystem->getConfig()->addPbdConstraintFunctor(attachmentConstraintFunctor);

    return connectiveStrands;
}

std::shared_ptr<Entity>
makeConnectiveTissue(
    std::shared_ptr<Entity>                   objA,
    std::shared_ptr<Entity>                   objB,
    std::shared_ptr<PbdSystem>                model,
    double                                    maxDist,
    double                                    strandsPerFace,
    int                                       segmentsPerStrand,
    std::shared_ptr<ProximitySurfaceSelector> proxSelector)
{
    proxSelector = std::make_shared<ProximitySurfaceSelector>();

    // Check inputs
    auto objASurf = std::dynamic_pointer_cast<SurfaceMesh>(Collider::getCollidingGeometryFromEntity(objA.get()));
    CHECK(objASurf != nullptr) << "Object A " << objA->getName() << " Did not contain a surface mesh as colliding geometry in generateConnectiveTissue";

    auto objBSurf = std::dynamic_pointer_cast<SurfaceMesh>(Collider::getCollidingGeometryFromEntity(objB.get()));
    CHECK(objBSurf != nullptr) << "Object B " << objB->getName() << " Did not contain a surface mesh as colliding geometry in generateConnectiveTissue";

    CHECK(model != nullptr) << "PbdSystem in generateConnectiveTissue is NULL";

    Vec3d objACenter = objASurf->getCenter();
    Vec3d objBCenter = objBSurf->getCenter();

    if (fabs(maxDist) < 1.0e-6)
    {
        maxDist = (objACenter - objBCenter).norm();
    }

    proxSelector->setInputMeshes(objASurf, objBSurf);

    proxSelector->setProximity(maxDist);
    proxSelector->update();

    // Create surface connector to generate geometry of connective tissue
    auto surfConnector = std::make_shared<ConnectiveStrandGenerator>();
    surfConnector->setInputMeshes(
        std::dynamic_pointer_cast<SurfaceMesh>(proxSelector->getOutput(0)),
        std::dynamic_pointer_cast<SurfaceMesh>(proxSelector->getOutput(1)));
    surfConnector->setSegmentsPerStrand(segmentsPerStrand);
    surfConnector->setStrandsPerFace(strandsPerFace);
    surfConnector->update();

    // Get mesh for connective strands
    auto connectiveLineMesh = std::dynamic_pointer_cast<LineMesh>(surfConnector->getOutput(0));

    auto methodA = objA->getComponent<PbdMethod>();
    auto methodB = objB->getComponent<PbdMethod>();

    // Create PBD object of connective strands with associated constraints
    auto connectiveStrands = addConnectiveTissueConstraints(
        connectiveLineMesh, methodA, methodB, model);

    return connectiveStrands;
}
} // namespace imstk