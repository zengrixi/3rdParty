/* -*-c++-*- */
/* osgEarth - Geospatial SDK for OpenSceneGraph
 * Copyright 2019 Pelican Mapping
 * http://osgearth.org
 *
 * osgEarth is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#ifndef OSGEARTHSYMBOLOGY_GEOMETRY_FACTORY
#define OSGEARTHSYMBOLOGY_GEOMETRY_FACTORY

#include <osgEarthSymbology/Common>
#include <osgEarthSymbology/Geometry>
#include <osgEarth/SpatialReference>

/**
 * A collection of utilities for creating geometric features
 */
namespace osgEarth { namespace Symbology
{
    using namespace osgEarth;

    /**
     * Convenience class for building simple tessellated Geometry shapes.
     */
    class OSGEARTHSYMBOLOGY_EXPORT GeometryFactory
    {
    public:
        /**
         * Constructs the factory.
         * @param srs The spatial reference in which to create the geometry, or NULL to
         *            create simple localized geometry.
         */
        GeometryFactory( const SpatialReference* srs =0L );

        /** dtor */
        virtual ~GeometryFactory() { }

        /**
         * Creates a circle geometry.
         * @param center Center point (must be in map SRS if a map was provided in ctor)
         * @param radius Circle radius
         * @param numSegments Number of circumference segments, or zero to automatically calculate it
         * @param geomToUse Use this geometry instead of creating a Polygon.
         */
        Geometry* createCircle( 
            const osg::Vec3d& center,
            const Distance&   radius,
            unsigned          numSegments =0,
            Geometry*         geomToUse   =0L) const;

        /**
         * Creates an arc geometry.
         * @param center Center point (must be in map SRS if a map was provided in ctor)
         * @param radius Arc radius
         * @param start  Starting angle of the arc
         * @param end    Ending angle of the arc
         * @param numSegments Number of circumference segments, or zero to automatically calculate it
         * @param pie    Indicates to create a pie shape rather than an arc.
         */
        Geometry* createArc(
            const osg::Vec3d& center,
            const Distance&   radius,
            const Angle&      startAngle,
            const Angle&      endAngle,
            unsigned          numSegments =0,
            Geometry*         geomToUse   =0L,
            bool              pie = false) const;

        /**
         * Creates a ellipse geometry.
         * @param center Center point (must be in map SRS if a map was provided in ctor)
         * @param radiusMajor Major radius (X-axis) length
         * @param radiusMinor Minor radius (Y-axis) length
         * @param rotationAngle with respect to the X-axis
         * @param numSegments Number of circumference segments, or zero to automatically calculate it
         * @param geomToUse Use this geometry instead of creating a Polygon.
         */
        Geometry* createEllipse(
            const osg::Vec3d& center,
            const Distance&   radiusMajor,
            const Distance&   radiusMinor,
            const Angle&      rotationAngle,
            unsigned          numSegments =0,
            Geometry*         geomToUse   =0L) const;

        /**
         * Creates a ellipse geometry.
         * @param center Center point (must be in map SRS if a map was provided in ctor)
         * @param radiusMajor Major radius (X-axis) length
         * @param radiusMinor Minor radius (Y-axis) length
         * @param rotationAngle with respect to the X-axis
         * @param start  Starting angle of the arc
         * @param end    Ending angle of the arc
         * @param numSegments Number of circumference segments, or zero to automatically calculate it
         * @param geomToUse Use this geometry instead of creating a Polygon.
         * @param pie    Indicates to create a pie shape rather than an arc.
         */
        Geometry* createEllipticalArc(
            const osg::Vec3d& center,
            const Distance&   radiusMajor,
            const Distance&   radiusMinor,
            const Angle&      rotationAngle,
            const Angle&      startAngle,
            const Angle&      endAngle,
            unsigned          numSegments =0,
            Geometry*         geomToUse   =0L,
            bool              pie = false) const;

        /**
         * Creates a rectangle geometry
         * @param center Center point (must be in map SRS if the map was provided in ctor)
         * @param width The width of the rectangle
         * @param height The height of the rectangle
         */
        Geometry* createRectangle(
            const osg::Vec3d& center,
            const Distance&   width,
            const Distance&   height ) const;

    protected:

        osg::ref_ptr<const SpatialReference> _srs;
    };

} } // namespace osgEarth::Symbology

#endif // OSGEARTHSYMBOLOGY_GEOMETRY_FACTORY
