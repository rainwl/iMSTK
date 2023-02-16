/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkTrackingDeviceControl.h"

#include <functional>

namespace imstk
{
class VisualModel;

///
/// \class SceneObjectController
///
/// \brief This class implements once tracking controller controlling one scene object
///
class SceneObjectController : public TrackingDeviceControl
{
public:
    SceneObjectController(const std::string& name = "SceneObjectController");
    ~SceneObjectController() override = default;

    ///
    /// \brief Update controlled scene object using latest tracking information
    ///
    void update(const double& dt) override;

    ///
    /// \brief Get/Set controlled scene object
    ///@{
    std::shared_ptr<VisualModel> getControlledObject() const { return m_sceneObject; }
    void setControlledObject(std::shared_ptr<VisualModel> so) { m_sceneObject = so; }
///@}

protected:
    std::shared_ptr<VisualModel> m_sceneObject; ///< SceneObject controlled by the Tracker
};
} // namespace imstk