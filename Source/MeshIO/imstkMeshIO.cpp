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

#include "imstkMeshIO.h"
#include "imstkAssimpMeshIO.h"
#include "imstkLogger.h"
#include "imstkMSHMeshIO.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVegaMeshIO.h"
#include "imstkVTKMeshIO.h"

#include <sys/stat.h>
#include <unordered_map>

namespace imstk
{
static std::unordered_map<std::string, MeshFileType> extToType =
{
    { "vtk", MeshFileType::VTK },
    { "vtp", MeshFileType::VTP },
    { "vtu", MeshFileType::VTU },
    { "obj", MeshFileType::OBJ },
    { "stl", MeshFileType::STL },
    { "ply", MeshFileType::PLY },
    { "dae", MeshFileType::DAE },
    { "fbx", MeshFileType::FBX },
    { "3ds", MeshFileType::_3DS },
    { "veg", MeshFileType::VEG },
    { "msh", MeshFileType::MSH },
    { "dcm", MeshFileType::DCM },
    { "nrrd", MeshFileType::NRRD },
    { "nii", MeshFileType::NII },
    { "mhd", MeshFileType::MHD }
};

std::shared_ptr<PointSet>
MeshIO::read(const std::string& filePath)
{
    bool isDir = false;

    if (isDir)
    {
        // Assume that the directory is a collection of DICOM files
        return VTKMeshIO::read(filePath, MeshFileType::DCM);
    }

    MeshFileType meshType = MeshIO::getFileType(filePath);
    switch (meshType)
    {
    case MeshFileType::VTK:
    case MeshFileType::VTU:
    case MeshFileType::VTP:
    case MeshFileType::STL:
    case MeshFileType::PLY:
    case MeshFileType::NRRD:
    case MeshFileType::NII:
    case MeshFileType::DCM:
    case MeshFileType::MHD:
        return VTKMeshIO::read(filePath, meshType);
        break;
    case MeshFileType::OBJ:
    case MeshFileType::DAE:
    case MeshFileType::FBX:
    case MeshFileType::_3DS:
        return AssimpMeshIO::read(filePath, meshType);
        break;
    case MeshFileType::VEG:
        return VegaMeshIO::read(filePath, meshType);
        break;
    case MeshFileType::MSH:
        return MSHMeshIO::read(filePath, meshType);
        break;
    case MeshFileType::UNKNOWN:
    default:
        break;
    }

    LOG(FATAL) << "MeshIO::read error: file type not supported";
    return nullptr;
}

bool
MeshIO::fileExists(const std::string& file, bool& isDirectory)
{
    struct stat buf;
    if (stat(file.c_str(), &buf) == 0)
    {
        if (buf.st_mode & S_IFDIR)
        {
            isDirectory = true;
        }
        else
        {
            isDirectory = false;
        }
        return true;
    }
    else
    {
        return false;
    }
}

const MeshFileType
MeshIO::getFileType(const std::string& filePath)
{
    std::string extString = filePath.substr(filePath.find_last_of(".") + 1);

    CHECK(!extString.empty()) << "MeshIO::getFileType error: invalid file name";

    // To lowercase
    std::transform(extString.begin(), extString.end(), extString.begin(),
        [](unsigned char c) { return std::tolower(static_cast<int>(c)); });

    if (extToType.count(extString) == 0)
    {
        LOG(FATAL) << "MeshIO::getFileType error: unknown file extension";
    }

    return extToType[extString];
}

bool
MeshIO::write(const std::shared_ptr<imstk::PointSet> imstkMesh, const std::string& filePath)
{
    if (imstkMesh == nullptr)
    {
        LOG(WARNING) << "MeshIO::write error: Mesh nullptr";
        return false;
    }

    MeshFileType meshType = MeshIO::getFileType(filePath);
    switch (meshType)
    {
    case MeshFileType::VEG:
        return VegaMeshIO::write(imstkMesh, filePath, meshType);
        break;
    case MeshFileType::NII:
    case MeshFileType::NRRD:
    case MeshFileType::VTU:
    case MeshFileType::VTK:
    case MeshFileType::VTP:
    case MeshFileType::STL:
    case MeshFileType::PLY:
    case MeshFileType::MHD:
        return VTKMeshIO::write(imstkMesh, filePath, meshType);
        break;
    case MeshFileType::UNKNOWN:
    default:
        break;
    }

    LOG(FATAL) << "MeshIO::write error: file type not supported";
    return false;
}
} // imstk
