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

#ifndef imstkTexture_h
#define imstkTexture_h

#include <string>
#include <memory>

namespace imstk
{
///
/// \class Texture
///
class Texture
{
public:
    ///
    /// \brief Texture type - determines filtering
    ///
    enum Type
    {
        DIFFUSE = 0, // Also used for albedo
        NORMAL,
        SPECULAR,
        ROUGHNESS,
        METALNESS,
        SUBSURFACE_SCATTERING,
        AMBIENT_OCCLUSION,
        CAVITY,
        CUBEMAP,
        IRRADIANCE_CUBEMAP,
        RADIANCE_CUBEMAP,
        NONE
    };

    ///
    /// \brief Constructor
    /// \param path Path to the texture source file
    /// \param type Type of texture
    ///
    Texture(std::string path = "", Type type = DIFFUSE);

    ///
    /// \brief Destructor
    ///
    virtual ~Texture() {}

    ///
    /// \brief Get type
    ///
    Type getType() const;

    ///
    /// \brief Get path
    ///
    const std::string getPath() const;

    ///
    /// \brief Get type
    ///
    bool getMipmapsEnabled();

protected:
    Type m_type;            ///< Texture type
    std::string m_path;     ///< Texture file path

    // Helps with texture aliasing (and a little with performance)
    bool m_mipmapsEnabled = true;
};
}

// This method is defined to allow for the map to be properly indexed by Texture objects
namespace std
{
template<> struct less<std::shared_ptr<imstk::Texture>>
{
    bool operator() (const std::shared_ptr<imstk::Texture>& texture1,
                     const std::shared_ptr<imstk::Texture>& texture2) const
    {
        if (texture1->getType() != texture2->getType())
        {
            return (texture1->getType() < texture2->getType());
        }

        if (texture1->getPath() != texture2->getPath())
        {
            return (texture1->getPath() < texture2->getPath());
        }

        return false;
    }
};
}

#endif
