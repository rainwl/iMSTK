// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
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
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#ifndef VIRTUAL_TOOLS_TOOLCOUPLER_H
#define VIRTUAL_TOOLS_TOOLCOUPLER_H

#include <memory>
#include <chrono>

#include "Core/Module.h"
#include "Core/Quaternion.h"
#include "Core/Vector.h"

namespace imstk {

class DeviceInterface;
class BaseMesh;

class ToolCoupler : public Module
{
public:
	using TransformType = Eigen::Transform<double, 3, Eigen::Isometry>;

public:
    ///
    /// \brief Constructor/Destructor
    ///
    ToolCoupler() = default;
    ~ToolCoupler();

    ToolCoupler(std::shared_ptr<DeviceInterface> inputDevice,
                std::shared_ptr<BaseMesh> toolMesh);

    ToolCoupler(std::shared_ptr<DeviceInterface> inputDevice);

    ToolCoupler(std::shared_ptr<DeviceInterface> inputDevice,
				std::shared_ptr<DeviceInterface> outputDevice,
                std::shared_ptr<BaseMesh> toolMesh);


    ///
    /// \brief Set the input device for this tool
    /// \param newDevice A pointer to an allocated device
    ///
    void setInputDevice(std::shared_ptr<DeviceInterface> newDevice);

    ///
    /// \brief Set the output device for this tool coupler
    ///
    std::shared_ptr<DeviceInterface> getInputDevice();

    ///
    /// \brief Set the output device for this tool coupler
    /// \param newDevice A pointer to an allocated device
    ///
    void setOutpurDevice(std::shared_ptr<DeviceInterface> newDevice);

    ///
    /// \brief Get the output device for this tool coupler
    ///
    std::shared_ptr<DeviceInterface> getOutputDevice();

    ///
    /// \brief Set the pointer to the mesh to control
    /// \param newMesh A pointer to an allocated mesh
    ///
    void setMesh(std::shared_ptr<BaseMesh> newMesh);

    ///
    /// \brief Get the output device for this tool coupler
    ///
    std::shared_ptr<BaseMesh> getMesh() const;

    ///
    /// \brief Get the current polling delay
    /// \return The currently set polling delay
    ///
    const std::chrono::milliseconds &getPollDelay() const;

    ///
    /// \brief Set the polling delay of the controller to get new data from the device
    /// \param delay The new polling delay to set
    ///
    void setPollDelay(const std::chrono::milliseconds &delay);

    ///
    /// \brief Get the current scaling factor
    /// \return The currently set scaling factor
    ///
    const double &getScalingFactor() const;

    ///
    /// \brief Set how much to scale the physical movement by in 3D space
    /// \param factor The new scaling factor to set
    ///
    void setScalingFactor(const double &factor);

    ///
    /// \brief Get the current orientation
    /// \return The currently set scaling factor
    ///
    const Quaterniond &getOrientation() const;

    ///
    /// \brief Set orientationmuch to scale the physical movement by in 3D space
    /// \param  The new scaling factor to set
    ///
    void setOrientation(const Eigen::Map<Quaterniond> &newOrientation);

    ///
    /// \brief Get the current orientation
    /// \return The currently set scaling factor
    ///
    const Vec3d &getPosition() const;

    ///
    /// \brief Set orientationmuch to scale the physical movement by in 3D space
    /// \param  The new scaling factor to set
    ///
    void setPosition(const Vec3d &newOrientation);

    ///
    /// \brief Set offset orientation much to scale the physical movement by in 3D space
    /// \param  The new scaling factor to set
    ///
    void setOffsetOrientation(const Eigen::Map<Quaterniond> &offsetOrientation);

    ///
    /// \brief Get the offset orientation
    /// \return The orientation
    ///
    const Quaterniond &getOffsetOrientation() const;

    ///
    /// \brief Get the offset position
    /// \return The currently set scaling factor
    ///
    const Vec3d &getOffsetPosition() const;

	///
	/// \brief Return the previous position
	///
    const Vec3d& getPrevPosition() const;

	///
	/// \brief Return the previous orientation
	///
    const Quaterniond& getPrevOrientation() const;

    ///
    /// \brief Set offset orientation much to scale the physical movement by in 3D space
    /// \param  The new scaling factor to set
    ///
    void setOffsetPosition(const Vec3d &offsetPosition);

    ///
    /// \brief Module overrides
    ///
    bool init() override;
    void beginFrame() override;
    void endFrame() override;

    ///
    /// \brief Update tracker and
    ///
    void exec() override;

    ///
    /// \brief Update position an orientation of the model from device data.
    ///
    bool updateTracker();

    ///
    /// \brief Update forces of the model from device data.
    ///
    bool updateForces();

private:
    Quaterniond orientation; //!< Previous rotation quaternion from phantom
    Vec3d position;          //!< Previous position from phantom

    Quaterniond prevOrientation; //!< Previous rotation quaternion from phantom
    Vec3d prevPosition;          //!< Previous position from phantom

    double scalingFactor;          //!< Scaling factor for physical to virtual translation

    Quaterniond offsetOrientation; //!< Previous rotation quaternion from device
    Vec3d offsetPosition;          //!< Previous position from device

    std::chrono::milliseconds pollDelay;  //!< Polling delay
    std::shared_ptr<BaseMesh> mesh; //!< Pointer to controlled mesh
    std::shared_ptr<DeviceInterface> inputDevice;  //!< Pointer to input device
    std::shared_ptr<DeviceInterface> outputDevice; //!< Pointer to output device
    TransformType initialTransform; //!< Transform applied to the position obtained from device
};

}

#endif // TOOLCOUPLER_H
