/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkComponent.h"
#include "imstkPbdBody.h"

namespace imstk
{
class Geometry;
class GeometryMap;
class PbdConstraint;
class PbdSystem;
class PointSet;

class PbdMethod : public SceneBehaviour
{
public:
    IMSTK_TYPE_NAME(PbdMethod) PbdMethod(const std::string& name = "PbdMethod");

    bool initialize();
    ///
    /// \brief Initializes the edges of the Behaviour's computational graph
    ///
    void initGraphEdges();

    ///
    /// \brief Set/Get the geometry used for Physics computations
    ///@{
    std::shared_ptr<Geometry> getPhysicsGeometry() const { return m_physicsGeometry; }
    void setPhysicsGeometry(std::shared_ptr<Geometry> geometry) { m_physicsGeometry = geometry; }
    ///@}

    ///
    /// \brief Set/Get the Physics-to-Collision map
    ///@{
    std::shared_ptr<GeometryMap> getPhysicsToCollidingMap() const { return m_physicsToCollidingGeomMap; }
    void setPhysicsToCollidingMap(std::shared_ptr<GeometryMap> map) { m_physicsToCollidingGeomMap = map; }
    ///@}

    ///
    /// \brief Set/Get the Physics-to-Visual map
    ///@{
    std::shared_ptr<GeometryMap> getPhysicsToVisualMap() const { return m_physicsToVisualGeomMap; }
    void setPhysicsToVisualMap(std::shared_ptr<GeometryMap> map) { m_physicsToVisualGeomMap = map; }
    ///@}

    ///
    /// \brief Set/Get dynamical model
    ///@{
    std::shared_ptr<PbdSystem> getPbdSystem() const { return m_pbdSystem; }
    void setPbdSystem(std::shared_ptr<PbdSystem> pbdSystem); // { m_pbdSystem = pbdSystem; }
    ///@}

    ///
    /// \brief Returns body in the model.
    ///
    std::shared_ptr<PbdBody> getPbdBody() const;

    ///
    /// \brief Returns the computational node for updating
    ///
    std::shared_ptr<TaskNode> getUpdateNode() const { return m_updateNode; }

    ///
    /// \brief Returns the computational node for updating geometry
    ///
    std::shared_ptr<TaskNode> getUpdateGeometryNode() const { return m_updateGeometryNode; }

    const std::vector<std::shared_ptr<PbdConstraint>>& getCellConstraints(int cellId);

    /// \brief Returns the corresponding body handle in the PbdSystem
    /// \return
    const int getBodyHandle() const { return m_pbdBody->bodyHandle; }

    /// \brief Set uniform mass value for each vertex of the body.
    /// \param uniformMass
    void setUniformMass(double uniformMass) { m_pbdBody->uniformMassValue = uniformMass; }

    /// @brief Set the ids of the nodes that will remain fixed.
    /// @param fixedNodes Vector of node ids
    void setFixedNodes(const std::vector<int>& fixedNodes)
    {
        m_pbdBody->fixedNodeIds = fixedNodes;
    }

    /// @brief Set pbdBody as rigid along with the necessary rigid body parameters.
    /// @param pos Position of rigid body center.
    /// @param mass Mass of rigid body.
    /// @param orientation
    /// @param inertia
    void setRigid(const Vec3d& pos,
                  const double mass = 1.0,
                  const Quatd& orientation = Quatd::Identity(),
                  const Mat3d& inertia     = Mat3d::Identity())
    {
        m_pbdBody->setRigid(pos, mass, orientation, inertia);
    }

    const Vec3d& getRigidPosition() const { return m_pbdBody->getRigidPosition(); }

    ///
    /// \brief Reset the behaviour by reseting the respective DynamicalModel and Geometry
    ///
    void reset();

    void setBodyFromGeometry();

    void updateGeometries();;

    void computeCellConstraintMap();

private:
    // void update() { };
    void updatePhysicsGeometry();
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink);

    void setDeformBodyFromGeometry(PbdBody& body, std::shared_ptr<PointSet> geom);
    void setRigidBody(PbdBody& body);

    // Physics related geometry and maps
    std::shared_ptr<Geometry>    m_physicsGeometry;                     ///< Geometry used for Physics
    std::shared_ptr<GeometryMap> m_physicsToCollidingGeomMap = nullptr; ///< Maps from Physics to collision geometry
    std::shared_ptr<GeometryMap> m_physicsToVisualGeomMap    = nullptr; ///< Maps from Physics to visual geometry
    std::shared_ptr<PbdBody>     m_pbdBody;                             ///< Handle to this object in the model/system
    std::shared_ptr<PbdSystem>   m_pbdSystem;                           ///< Pbd mathematical model and solver system

    std::shared_ptr<TaskNode> m_updateNode = nullptr;
    std::shared_ptr<TaskNode> m_updateGeometryNode = nullptr;
};
} // namespace imstk