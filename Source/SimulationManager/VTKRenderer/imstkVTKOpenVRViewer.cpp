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

#include "imstkVTKOpenVRViewer.h"
#include "imstkDeviceControl.h"
#include "imstkLogger.h"
#include "imstkOpenVRDeviceClient.h"
#include "imstkScene.h"
#include "imstkVTKInteractorStyle.h"
#include "imstkVTKInteractorStyleVR.h"
#include "imstkVTKRenderer.h"

#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkMatrix4x4.h>
#include <vtkOpenVRRenderer.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRModel.h>

namespace imstk
{
VTKOpenVRViewer::VTKOpenVRViewer(std::string name) : AbstractVTKViewer(name)
{
    // Create the interactor style
    auto vrInteractorStyle = std::make_shared<vtkInteractorStyleVR>();
    m_interactorStyle    = std::dynamic_pointer_cast<InteractorStyle>(vrInteractorStyle);
    m_vtkInteractorStyle = std::dynamic_pointer_cast<vtkInteractorStyle>(m_interactorStyle);

    // Create the interactor
    vtkNew<vtkOpenVRRenderWindowInteractor> iren;
    iren->SetInteractorStyle(m_vtkInteractorStyle.get());

    // Create the RenderWindow
    m_vtkRenderWindow = vtkSmartPointer<vtkOpenVRRenderWindow>::New();
    m_vtkRenderWindow->SetInteractor(iren);
    iren->SetRenderWindow(m_vtkRenderWindow);
    m_vtkRenderWindow->HideCursor();

    m_vrDeviceClients.push_back(vrInteractorStyle->getLeftControllerDeviceClient());
    m_vrDeviceClients.push_back(vrInteractorStyle->getRightControllerDeviceClient());
    m_vrDeviceClients.push_back(vrInteractorStyle->getHmdDeviceClient());
}

void
VTKOpenVRViewer::setActiveScene(std::shared_ptr<Scene> scene)
{
    // If already current scene
    if (scene == m_activeScene)
    {
        LOG(WARNING) << scene->getName() << " already is the viewer current scene.";
        return;
    }

    // If the current scene has a renderer, remove it
    if (m_activeScene)
    {
        auto vtkRenderer = std::dynamic_pointer_cast<VTKRenderer>(this->getActiveRenderer())->getVtkRenderer();
        if (m_vtkRenderWindow->HasRenderer(vtkRenderer))
        {
            m_vtkRenderWindow->RemoveRenderer(vtkRenderer);
        }
    }

    // Update current scene
    m_activeScene = scene;

    // Create renderer if it doesn't exist
    if (!m_rendererMap.count(m_activeScene))
    {
        m_rendererMap[m_activeScene] = std::make_shared<VTKRenderer>(m_activeScene, true);
    }

    // Cast to VTK renderer
    auto vtkRenderer = std::dynamic_pointer_cast<VTKRenderer>(this->getActiveRenderer())->getVtkRenderer();

    // Set renderer to renderWindow
    m_vtkRenderWindow->AddRenderer(vtkRenderer);

    m_vtkInteractorStyle->SetCurrentRenderer(vtkRenderer);
}

void
VTKOpenVRViewer::setPhysicalToWorldTransform(const Mat4d& physicalToWorldMatrix)
{
    vtkSmartPointer<vtkOpenVRRenderWindow> renWin =
        vtkOpenVRRenderWindow::SafeDownCast(m_vtkRenderWindow);
    vtkNew<vtkMatrix4x4> mat;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            mat->SetElement(i, j, physicalToWorldMatrix(i, j));
        }
    }
    renWin->SetPhysicalToWorldMatrix(mat);
}

Mat4d
VTKOpenVRViewer::getPhysicalToWorldTransform()
{
    vtkSmartPointer<vtkOpenVRRenderWindow> renWin =
        vtkOpenVRRenderWindow::SafeDownCast(m_vtkRenderWindow);
    Mat4d                transform;
    vtkNew<vtkMatrix4x4> mat;
    renWin->GetPhysicalToWorldMatrix(mat);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            transform(i, j) = mat->GetElement(i, j);
        }
    }
    return transform;
}

void
VTKOpenVRViewer::setRenderingMode(const Renderer::Mode mode)
{
    if (!m_activeScene)
    {
        LOG(WARNING) << "Missing scene, can not set rendering mode.\n"
                     << "Use Viewer::setCurrentScene to setup scene.";
        return;
    }

    // Setup renderer
    this->getActiveRenderer()->setMode(mode, true);

    // Render to update displayed actors
    m_vtkRenderWindow->Render();
}

bool
VTKOpenVRViewer::initModule()
{
    if (!AbstractVTKViewer::initModule())
    {
        return false;
    }

    // VR interactor doesn't support timers, here we throw timer event every update
    // another option would be to conform VTKs VR interactor
    auto iren = vtkOpenVRRenderWindowInteractor::SafeDownCast(m_vtkRenderWindow->GetInteractor());
    //iren->Start(); // Cannot use
    if (iren->HasObserver(vtkCommand::StartEvent))
    {
        iren->InvokeEvent(vtkCommand::StartEvent, nullptr);
        return true;
    }

    auto renWin = vtkOpenVRRenderWindow::SafeDownCast(m_vtkRenderWindow);
    renWin->Initialize();

    iren->Initialize();

    // Hide the device overlays
    // \todo: Display devices in debug mode
    renWin->Render(); // Must do one render to initialize vtkOpenVRModel's to then hide the devices

    /*iren->AddAction("/actions/vtk/in/LeftGripMovement", true,
        [&](vtkEventData* ed)
        {
            vtkEventDataDevice3D* edd = static_cast<vtkEventDataDevice3D*>(ed);
            const double* pos = edd->GetTrackPadPosition();
            printf("left movement %f, %f\n", pos[0], pos[1]);
        });
    iren->AddAction("/actions/vtk/in/RightGripMovement", true,
        [&](vtkEventData* ed)
        {
            vtkEventDataDevice3D* edd = static_cast<vtkEventDataDevice3D*>(ed);
            const double* pos = edd->GetTrackPadPosition();
            printf("right movement %f, %f\n", pos[0], pos[1]);
        });*/
    iren->AddAction("/actions/vtk/in/ButtonPressed", true,
        [&](vtkEventData* ed)
        {
            printf("button press\n");
        });
    iren->AddAction("/actions/vtk/in/RightGripPressed", false,
        [&](vtkEventData* ed)
        {
            printf("right grip press\n");
        });
    iren->AddAction("/actions/vtk/in/LeftGripPressed", false,
        [&](vtkEventData* ed)
        {
            printf("left grip press\n");
        });
    iren->AddAction("/actions/vtk/in/LeftTriggerPressed", false,
        [&](vtkEventData* ed)
        {
            printf("left trigger press\n");
        });
    iren->AddAction("/actions/vtk/in/RightTriggerPressed", false,
        [&](vtkEventData* ed)
        {
            printf("right trigger press\n");
        });

    // Hide all controllers
    for (uint32_t i = 0; i < vr::k_unMaxTrackedDeviceCount; i++)
    {
        vtkVRModel* trackedDeviceModel = renWin->GetTrackedDeviceModel(i);
        if (trackedDeviceModel != nullptr)
        {
            trackedDeviceModel->SetVisibility(false);
        }
    }

    return true;
}

void
VTKOpenVRViewer::updateModule()
{
    std::shared_ptr<imstk::VTKRenderer> ren = std::dynamic_pointer_cast<imstk::VTKRenderer>(getActiveRenderer());
    if (ren == nullptr)
    {
        return;
    }

    // Update Camera
    // \todo: No programmatic control over VR camera currently
    //renderer->updateSceneCamera(getActiveScene()->getCamera());

    // Call visual update on every scene object
    getActiveScene()->updateVisuals();
    // Update all the rendering delegates
    ren->updateRenderDelegates();

    // Render
    //m_vtkRenderWindow->GetInteractor()->Render();
    m_vtkRenderWindow->Render();
}

std::shared_ptr<OpenVRDeviceClient>
VTKOpenVRViewer::getVRDeviceClient(int deviceType)
{
    auto iter = std::find_if(m_vrDeviceClients.begin(), m_vrDeviceClients.end(),
        [&](const std::shared_ptr<OpenVRDeviceClient>& deviceClient)
        {
            return static_cast<int>(deviceClient->getDeviceType()) == deviceType;
        });
    return (iter == m_vrDeviceClients.end()) ? nullptr : *iter;
}
}
