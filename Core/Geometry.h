// This file is part of the iMSTK project.
//
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

#ifndef CORE_GEOMETRY_H
#define CORE_GEOMETRY_H

// iMSTK includes
#include "Core/Config.h"
#include "Core/Vector.h"
#include "Core/Matrix.h"

#include "Core/Factory.h"
#include "RenderDelegate.h"

namespace imstk {

//forward declaration
struct Sphere;

class VisualArtifact
{
public:
  virtual void setRenderDelegate(RenderDelegate::Ptr delegate)
    {
    this->renderDelegate = delegate;
    if (delegate)
      this->renderDelegate->setSourceGeometry(this);
    }
  virtual void draw() const
    {
    if (this->renderDelegate)
      this->renderDelegate->draw();
    }

    /// \brief Get render delegate
    std::shared_ptr<RenderDelegate> getRenderDelegate() const
    {
        return this->renderDelegate;
    }
  RenderDelegate::Ptr renderDelegate;
};

class AnalyticalGeometry : public VisualArtifact
{
public:
    AnalyticalGeometry(){}
    ~AnalyticalGeometry(){}

    virtual void translate(const Vec3d &t) = 0;
    virtual void rotate(const Matrix33d &rot) = 0;
};

/// \brief  Simple Plane definition with unit normal and spatial location
class Plane : public AnalyticalGeometry
{
public:
    Plane()
      {
      this->width = 100.0;

      this->setRenderDelegate(
        Factory<RenderDelegate>::createSubclassForGroup(
            "RenderDelegate", RenderDelegate::RendererType::VTK));
      }
    ~Plane(){}

    /// \brief create a plane with point and unit normal
    Plane(const Vec3d &p, const Vec3d &n)
    {
        this->point = p;
        this->unitNormal = n.normalized();
        this->width = 100.0;

        this->drawPointsOrig[0] = Vec3d(width, 0, 0);
        this->drawPointsOrig[1] = Vec3d(0, width, 0);
        this->drawPointsOrig[2] = Vec3d(-width, 0, 0);
        this->drawPointsOrig[3] = Vec3d(0, -width, 0);

        this->movedOrRotated = true;

        this->setRenderDelegate(
          Factory<RenderDelegate>::createSubclass(
            "RenderDelegate", "PlaneRenderDelegate"));
    }

    double distance(const Vec3d &p_vector)
    {
        auto m = (p_vector - this->point).dot(this->unitNormal);
        return m;
    };

    Vec3d project(const Vec3d &p_vector)
    {
        return p_vector - ((this->point - p_vector)*this->unitNormal.transpose())*this->unitNormal;
    };

    const Vec3d &getUnitNormal() const
    {
        return this->unitNormal;
    }

    void setModified(bool s)
    {
        this->movedOrRotated = s;
    };

    void setUnitNormal(const Vec3d &normal)
    {
        this->unitNormal = normal;

        this->movedOrRotated = true;
    }

    const Vec3d &getPoint() const
    {
        return this->point;
    }

    void setPoint(const Vec3d &p)
    {
        this->point = p;

        this->movedOrRotated = true;
    }

    void translate(const Vec3d &t)
    {
        this->point += t;

        this->movedOrRotated = true;
    }

    void rotate(const Matrix33d &rot)
    {
        this->unitNormal = rot * this->unitNormal;

        this->movedOrRotated = true;
    }

    void setDrawPoint(const Vec3d &p1, const Vec3d &p2, const Vec3d &p3, const Vec3d &p4)
    {
        this->drawPointsOrig[0] = p1;
        this->drawPointsOrig[1] = p2;
        this->drawPointsOrig[2] = p3;
        this->drawPointsOrig[3] = p4;

        this->movedOrRotated = true;
    }

    double getWidth() const
    {
        return this->width;
    }

    void setWidth(double w)
    {
        this->width = w;
    }

    void updateDrawPoints()
    {
        Vec3d ny = Vec3d(0.0, unitNormal[2], -unitNormal[1]);
        Vec3d nz = ny.cross(unitNormal);
        ny.normalize();
        nz.normalize();

        Matrix33d R;
        R << this->unitNormal[0], ny[1], nz[2],
             this->unitNormal[0], ny[1], nz[2],
             this->unitNormal[0], ny[1], nz[2];

        for (int i = 0; i < 4; i++)
        {
            this->drawPoints[i] = this->point + R*this->drawPointsOrig[i];
        }
        this->movedOrRotated = false;
    }

    /// \brief Get render detail
    std::shared_ptr<RenderDetail> getRenderDetail() const
    {return renderDetail;}

    /// \brief Set the render details (properties affecting visual depiction)
    void setRenderDetail(std::shared_ptr<RenderDetail> newRenderDetail)
    { this->renderDetail = newRenderDetail; }

private:
    /// \brief unit normal of the plane
    Vec3d unitNormal;

    /// \brief any point on the plane
    Vec3d point;

    /// \brief true if the plane is static
    bool movedOrRotated;

    /// \brief width od the plane for rendering
    double width;

    /// \brief four points used to render plane
    Vec3d drawPoints[4];

    /// \brief four points used to render plane
    Vec3d drawPointsOrig[4];

    /// Render details
    std::shared_ptr<RenderDetail> renderDetail;
};


/// \brief sphere with center and radius
class Sphere : public AnalyticalGeometry
{
public:
    /// \brief constructor
    Sphere();

    /// \brief sphere constructor with center and radius
    Sphere(const Vec3d &c, const double &r)
    {
        this->center = c;
        this->radius = r;
    }

    ~Sphere(){}

    void setRadius(const double r)
    {
        this->radius = r;
    }

    void setCenter(const Vec3d& c)
    {
        this->center = c;
    }

    void incrementRadius(const double r)
    {
        this->radius += r;
    }

    void translate(const Vec3d &t)
    {
        center += t;
    }

    void rotate(const Matrix33d &/*rot*/)
    {
        //Its a sphere! nothing to be done.
    }

    double getRadius() const
    {
        return this->radius;
    }

    const Vec3d &getCenter() const
    {
        return this->center;
    }

private:
    /// \brief center of sphere
    Vec3d center;

    /// \brief radius of sshere
    double radius;
};

/// \brief cube
struct Cube
{
    /// \brief cube center
    Vec3d center;

    /// \brief cube length
    double sideLength;

    /// \brief constructor
    Cube();

    /// \brief subdivides the cube in mulitple cube with given number of cubes identified for each axis with p_divisionPerAxis
    void subDivide(int p_divisionPerAxis, Cube *p_cube);

    /// \brief expands the cube. increases the edge length with expansion*edge length
    void expand(double p_expansion);

    /// \brief returns the left most corner
    Vec3d leftMinCorner() const ;

    /// \brief returns right most corner
    Vec3d rightMaxCorner() const;

    /// \brief returns the smallest sphere encapsulates the cube
    Sphere getCircumscribedSphere();

    /// \brief returns the  sphere with half edge of the cube as a radius
    Sphere getInscribedSphere();

    /// \brief get tangent sphere
    Sphere getTangent2EdgeSphere();
};




/// \brief Axis Aligned bounding box declarions
class AABB : public VisualArtifact
{
public:
    /// \brief minimum x,y,z point
    Vec3d aabbMin;

    /// \brief maximum x,y,z point
    Vec3d aabbMax;

    const Vec3d &getMax() const
    {
        return aabbMax;
    }

    const Vec3d &getMin() const
    {
        return aabbMin;
    }

    /// \brief constrcutor. The default is set to origin for aabbMin and aabbMax
    AABB();

    /// \brief center of the AABB
    Vec3d center() const;

    /// \brief check if two AABB overlaps
    static bool checkOverlap(const AABB &p_aabbA, const AABB &p_aabbB);

    /// \brief check if two AABB overlaps
    bool overlaps(const AABB &other) const;

    /// \brief set  p_aabb to the current one
    const AABB &operator=(const AABB &p_aabb);

    /// \brief scale the AABB
    AABB &operator*(const double p_scale);

    /// \brief sub divides p_length will be used to create the slices
    void subDivide(const double p_length, const int p_divison, AABB *p_aabb) const;

    /// \brief divides current AABB in x,y,z axes with specificed divisions. results are placed in p_aabb
    void subDivide(const int p_divisionX, const int p_divisionY, const int p_divisionZ, AABB *p_aabb) const;

    /// \brief divides current AABB in all axes with specificed p_division. results are placed in p_aabb
    void subDivide(const int p_division, AABB *p_aabb) const;

    /// \brief returns half of X edge of AABB
    double halfSizeX() const;

    /// \brief returns half of Y edge of AABB
    double halfSizeY() const;

    /// \brief returns half of Z edge of AABB
    double halfSizeZ() const;

    /// \brief expands aabb with p_factor
    void expand(const double &p_factor);

    void reset()
    {
        this->aabbMin << std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max();
        this->aabbMax << std::numeric_limits<float>::min(),
            std::numeric_limits<float>::min(),
            std::numeric_limits<float>::min();
    }

    void extend(const AABB &other)
    {
        this->aabbMin = this->aabbMin.array().min(other.getMin().array());
        this->aabbMax = this->aabbMax.array().max(other.getMax().array());
    }
};

}

#endif
