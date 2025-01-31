/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCollidingObject.h"
#include "imstkCDObjectFactory.h"
#include "imstkGeometry.h"
#include "imstkGeometryMap.h"

namespace imstk
{
bool
CollidingObject::initialize()
{
    if (!SceneObject::initialize())
    {
        return false;
    }

    if (m_collidingToVisualMap)
    {
        m_collidingToVisualMap->compute();
    }

    return true;
}

std::shared_ptr<Geometry>
CollidingObject::getCollidingGeometry() const
{
    return m_collidingGeometry;
}

void
CollidingObject::setCollidingGeometry(std::shared_ptr<Geometry> geometry)
{
    m_collidingGeometry = geometry;
}

std::shared_ptr<GeometryMap>
CollidingObject::getCollidingToVisualMap() const
{
    return m_collidingToVisualMap;
}

void
CollidingObject::setCollidingToVisualMap(std::shared_ptr<GeometryMap> map)
{
    m_collidingToVisualMap = map;
}

void
CollidingObject::updateGeometries()
{
    if (m_collidingToVisualMap)
    {
        m_collidingToVisualMap->update();
        m_collidingToVisualMap->getChildGeometry()->postModified();
    }
    SceneObject::updateGeometries();
}

std::string
getCDType(const CollidingObject& obj1, const CollidingObject& obj2)
{
    std::string cdType = CDObjectFactory::getCDType(*obj1.getCollidingGeometry(), *obj2.getCollidingGeometry());
    return cdType;
}

void
CollidingObject::postModifiedAll()
{
    if (m_collidingGeometry != nullptr)
    {
        m_collidingGeometry->postModified();
    }
    SceneObject::postModifiedAll();
}
} // namespace imstk