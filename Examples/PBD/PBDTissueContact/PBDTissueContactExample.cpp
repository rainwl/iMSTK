/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkPbdCollisionHandling.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectController.h"
#include "imstkPointwiseMap.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkTextVisualModel.h"
#include "imstkVTKViewer.h"

#ifdef iMSTK_USE_OPENHAPTICS
#include "imstkHapticDeviceManager.h"
#include "imstkHapticDeviceClient.h"
#else
#include "imstkDummyClient.h"
#include "imstkMouseDeviceClient.h"
#endif

using namespace imstk;

///
/// \brief Spherically project the texture coordinates
///
static void
setSphereTexCoords(std::shared_ptr<SurfaceMesh> surfMesh, const double uvScale)
{
    Vec3d min, max;
    surfMesh->computeBoundingBox(min, max);
    const Vec3d size   = max - min;
    const Vec3d center = (max + min) * 0.5;

    const double radius = (size * 0.5).norm();

    auto                    uvCoordsPtr = std::make_shared<VecDataArray<float, 2>>(surfMesh->getNumVertices());
    VecDataArray<float, 2>& uvCoords    = *uvCoordsPtr.get();
    for (int i = 0; i < surfMesh->getNumVertices(); i++)
    {
        Vec3d vertex = surfMesh->getVertexPosition(i) - center;

        // Compute phi and theta on the sphere
        const double theta = asin(vertex[0] / radius);
        const double phi   = atan2(vertex[1], vertex[2]);
        uvCoords[i] = Vec2f(phi / (PI * 2.0) + 0.5, theta / (PI * 2.0) + 0.5) * uvScale;
    }
    surfMesh->setVertexTCoords("tcoords", uvCoordsPtr);
}

///
/// \brief Creates pbd simulated tissue object
/// \param name
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of tissue block
/// \param model dynamical model the tissue should use
///
static std::shared_ptr<PbdObject>
makeTissueObj(const std::string& name,
              const Vec3d& size, const Vec3i& dim, const Vec3d& center,
              std::shared_ptr<PbdModel> model)
{
    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tissueMesh = GeometryUtils::toTetGrid(center, size, dim);
    std::shared_ptr<SurfaceMesh>     surfMesh   = tissueMesh->extractSurfaceMesh();
    setSphereTexCoords(surfMesh, 4.0);

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);
    auto diffuseTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshDiffuse.jpg");
    material->addTexture(std::make_shared<Texture>(diffuseTex, Texture::Type::Diffuse));
    auto normalTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshNormal.jpg");
    material->addTexture(std::make_shared<Texture>(normalTex, Texture::Type::Normal));
    auto ormTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshORM.jpg");
    material->addTexture(std::make_shared<Texture>(ormTex, Texture::Type::ORM));

    // Add a visual model to render the surface of the tet mesh
    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(surfMesh);
    visualModel->setRenderMaterial(material);

    // Add a visual model to render the normals of the surface
    auto normalsVisualModel = std::make_shared<VisualModel>();
    normalsVisualModel->setGeometry(surfMesh);
    normalsVisualModel->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::SurfaceNormals);
    normalsVisualModel->getRenderMaterial()->setPointSize(0.5);

    // Setup the Object
    auto tissueObj = std::make_shared<PbdObject>(name);
    tissueObj->addVisualModel(visualModel);
    tissueObj->addVisualModel(normalsVisualModel);
    tissueObj->setPhysicsGeometry(tissueMesh);
    tissueObj->setCollidingGeometry(surfMesh);
    tissueObj->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tissueMesh, surfMesh));
    tissueObj->setDynamicalModel(model);
    tissueObj->getPbdBody()->uniformMassValue = 0.05;
    // Fix the borders
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                if (x == 0 || /*z == 0 ||*/ x == dim[0] - 1 /*|| z == dim[2] - 1*/)
                {
                    tissueObj->getPbdBody()->fixedNodeIds.push_back(x + dim[0] * (y + dim[1] * z));
                }
            }
        }
    }
    model->getConfig()->m_femParams->m_YoungModulus = 50.0;
    model->getConfig()->m_femParams->m_PoissonRatio = 0.4;
    model->getConfig()->enableFemConstraint(PbdFemConstraint::MaterialType::NeoHookean);
    model->getConfig()->setBodyDamping(tissueObj->getPbdBody()->bodyHandle, 0.1);

    return tissueObj;
}

///
/// \brief Create pbd simulated rigid tool object
/// \param model dynamical model the tool should use
///
static std::shared_ptr<PbdObject>
makeToolObj(std::shared_ptr<PbdModel> model)
{
    auto                    toolGeometry = std::make_shared<LineMesh>();
    VecDataArray<double, 3> vertices     = { Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 2.0, 0.0) };
    VecDataArray<int, 2>    indices      = { Vec2i(0, 1) };
    toolGeometry->initialize(std::make_shared<VecDataArray<double, 3>>(vertices),
        std::make_shared<VecDataArray<int, 2>>(indices));

    auto toolObj = std::make_shared<PbdObject>("Tool");
    toolObj->setVisualGeometry(toolGeometry);
    toolObj->setCollidingGeometry(toolGeometry);
    toolObj->setPhysicsGeometry(toolGeometry);
    toolObj->setDynamicalModel(model);
    toolObj->getVisualModel(0)->getRenderMaterial()->setColor(Color::Blue);
    toolObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    toolObj->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
    toolObj->getVisualModel(0)->getRenderMaterial()->setLineWidth(10.0);

    model->getConfig()->setBodyDamping(toolObj->getPbdBody()->bodyHandle, 0.05, 0.0);

    toolObj->getPbdBody()->setRigid(Vec3d(0.0, 0.8, 0.0), 0.2,
        Quatd::Identity(), Mat3d::Identity() * 10000.0);

    return toolObj;
}

///
/// \brief Creates a text object with text in the top right
///
static std::shared_ptr<SceneObject>
makeTextObj()
{
    auto txtVisualModel = std::make_shared<TextVisualModel>();
    txtVisualModel->setText("Device Force: 0N\nContact Force: 0N");
    txtVisualModel->setPosition(TextVisualModel::DisplayPosition::UpperRight);
    auto obj = std::make_shared<SceneObject>();
    obj->addVisualModel(txtVisualModel);
    return obj;
}

///
/// \brief This example demonstrates two-way collision interaction with a 3d
/// xpbd simulated tool and tissue (tetrahedral). With proper compliance.
///
/// If imstk is built with haptics a force is rendered. Otherwise mouse controls
/// are used. This example is currently not too scale, a force scaling is used.
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    auto scene = std::make_shared<Scene>("PbdTissueContact");
    scene->getActiveCamera()->setPosition(0.12, 4.51, 16.51);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);

    // Setup the Model/System
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->getConfig()->m_doPartitioning = false;
    pbdModel->getConfig()->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdModel->getConfig()->m_dt         = 0.05;
    pbdModel->getConfig()->m_iterations = 5;

    // Setup a tissue
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("Tissue",
        Vec3d(8.0, 2.0, 8.0), Vec3i(6, 5, 6), Vec3d(0.0, -1.0, 0.0), pbdModel);
    scene->addSceneObject(tissueObj);

    // Setup a tool
    std::shared_ptr<PbdObject> toolObj = makeToolObj(pbdModel);
    scene->addSceneObject(toolObj);

    // Setup a text to display forces
    std::shared_ptr<SceneObject> txtObj = makeTextObj();
    scene->addSceneObject(txtObj);

    // With PbdRigidObjectCollision we have Pbd-Rigid coupling
    // The toolObj responds to the tissue (the tool is pushed partly out of the way of the tissue whilst the
    // tissue deforms)
    auto collision = std::make_shared<PbdObjectCollision>(tissueObj, toolObj, "ClosedSurfaceMeshToMeshCD");
    //std::dynamic_pointer_cast<ClosedSurfaceMeshToMeshCD>(collision->getCollisionDetection())->setGenerateEdgeEdgeContacts(true);
    scene->addInteraction(collision);

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("Light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);
        viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

        auto controller = std::make_shared<PbdObjectController>();
#ifdef iMSTK_USE_OPENHAPTICS
        auto hapticManager = std::make_shared<HapticDeviceManager>();
        hapticManager->setSleepDelay(1.0); // Delay for 1ms (haptics thread is limited to max 1000hz)
        std::shared_ptr<HapticDeviceClient> deviceClient = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        controller->setTranslationScaling(0.05);
#else
        auto deviceClient = std::make_shared<DummyClient>();
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                const Vec2d mousePos = viewer->getMouseDevice()->getPos();
                const Vec3d worldPos = Vec3d(mousePos[0] - 0.5, mousePos[1] - 0.5, 0.0) * 10.0;

                deviceClient->setPosition(worldPos);
            });

        controller->setTranslationScaling(1.0);
#endif
        controller->setControlledObject(toolObj);
        controller->setDevice(deviceClient);
        controller->setLinearKs(5000.0);
        controller->setAngularKs(10000000.0);
        controller->setUseCritDamping(true);
        controller->setForceScaling(0.0025);
        controller->setUseForceSmoothening(true);
        scene->addControl(controller);

        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                // Keep the tool moving in real time
                pbdModel->getConfig()->m_dt = sceneManager->getDt();

                // Assume first collision constraint
                double contactForceMag = 0.0;
                auto pbdCH = std::dynamic_pointer_cast<PbdCollisionHandling>(collision->getCollisionHandlingAB());
                const std::vector<PbdConstraint*>& collisionConstraints = pbdCH->getConstraints();
                if (collisionConstraints.size() > 0)
                {
                    // Multiply with gradient for direction
                    contactForceMag  = std::abs(collisionConstraints[0]->getForce(pbdModel->getConfig()->m_dt));
                    contactForceMag *= controller->getForceScaling(); // Scale to bring in device space
                }

                // Display spring and contact force
                const std::string forceTxt = "Device Force: " + std::to_string(controller->getDeviceForce().norm()) + "N\n" +
                                             "Scaled Contact Force: " + std::to_string(contactForceMag) + 'N';
                std::dynamic_pointer_cast<TextVisualModel>(txtObj->getVisualModel(0))->setText(forceTxt);
        });

        // Add mouse and keyboard controls to the viewer
        {
            auto mouseControl = std::make_shared<MouseSceneControl>();
            mouseControl->setDevice(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            scene->addControl(mouseControl);

            auto keyControl = std::make_shared<KeyboardSceneControl>();
            keyControl->setDevice(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            scene->addControl(keyControl);
        }

        driver->start();
    }

    return 0;
}