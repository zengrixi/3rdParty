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

#ifndef OSGEARTHFEATURES_BUILD_GEOMETRY_FILTER_H
#define OSGEARTHFEATURES_BUILD_GEOMETRY_FILTER_H 1

#include <osgEarthFeatures/Common>
#include <osgEarthFeatures/Feature>
#include <osgEarthFeatures/Filter>
#include <osgEarthSymbology/Style>
#include <osgEarth/GeoMath>
#include <osg/Geode>

namespace osgEarth { namespace Features 
{
    using namespace osgEarth;
    using namespace osgEarth::Symbology;

    /**
     * Builds geometry from a stream of input features.
     */
    class OSGEARTHFEATURES_EXPORT BuildGeometryFilter : public FeaturesToNodeFilter
    {
    public:
        BuildGeometryFilter( const Style& style =Style() );

        virtual ~BuildGeometryFilter() { }

        /** Pushes a list of features through the filter. */
        osg::Node* push( FeatureList& input, FilterContext& context );

        /** The style to apply to feature geometry */
        const Style& getStyle() { return _style; }
        void setStyle(const Style& s) { _style = s; }

        /**
         * For geocentric data, sets the granularity of edges created by the filter. This
         * is the maximum angle (in degrees) between the vectors representing two geocentric
         * points comprising an edge. Since large polygons won't naturally conform to the
         * ellipsoid very well, this setting lets you specify a minimum edge size so that they
         * will break down and conform better.
         */
        optional<double>& maxGranularity() { return _maxAngle_deg; }
        const optional<double>& maxGranularity() const { return _maxAngle_deg; }

        /**
         * The algorithm to use when interpolating between geodetic locations.
         * The default is GEOINTERP_RHUMBLINE.
         */
        optional<GeoInterpolation>& geoInterp() { return _geoInterp; }
        const optional<GeoInterpolation>& geoInterp() const { return _geoInterp; }

        /**
         * Sets an expression to evaluate for setting the name of a Geometry.
         * Warning: this will disable some performance optimizations since the filter
         * can no longer merge geometries.
         */
        optional<StringExpression>& featureName() { return _featureNameExpr; }
        const optional<StringExpression>& featureName() const { return _featureNameExpr; }

        /**
         * When a geocentric polygon is very large, the filter has to tile it up in order to
         * properly tessellate it. This is the maximum size of each tile (in degrees).
         */        
        optional<float>& maxPolygonTilingAngle() { return _maxPolyTilingAngle_deg; }
        const optional<float>& maxPolygonTilingAngle() const { return _maxPolyTilingAngle_deg; }

        /**
         * Whether to run vertex order optimizations on the resulting geometry.
         * This can speed up draw performance at the expense of increasing build time.
         */
        optional<bool>& optimizeVertexOrdering() { return _optimizeVertexOrdering; }
        const optional<bool>& optimizeVertexOrdering() const { return _optimizeVertexOrdering; }

        /**
         * Maximum angle at which the smoother (for 3D polygon normal generation) should
         * smooth normals across polygon boundaries. Default is 0 degrees. This corresponds
         * to the SmoothingVisitor::setCreaseAngle value.
         */
        optional<Angle>& maxCreaseAngle() { return _maximumCreaseAngle; }
        const optional<Angle>& maxCreaseAngle() const { return _maximumCreaseAngle; }

        /**
         * Shader policy to use when appropriate
         */
        optional<ShaderPolicy>& shaderPolicy() { return _shaderPolicy; }
        const optional<ShaderPolicy>& shaderPolicy() const { return _shaderPolicy; }

    protected:
        Style                      _style;

        optional<double>           _maxAngle_deg;
        optional<GeoInterpolation> _geoInterp;
        optional<StringExpression> _featureNameExpr;
        optional<float>            _maxPolyTilingAngle_deg;
        optional<bool>             _optimizeVertexOrdering;
        optional<Angle>            _maximumCreaseAngle;
        optional<ShaderPolicy>     _shaderPolicy;
        
        void tileAndBuildPolygon(
            Geometry*               input,
            const SpatialReference* featureSRS,
            const SpatialReference* mapSRS,
            bool                    makeECEF,
            bool                    tessellate,
            osg::Geometry*          osgGeom,
            const osg::Matrixd      &world2local);
        
        void buildPolygon(
            Geometry*               input,
            const SpatialReference* featureSRS,
            const SpatialReference* mapSRS,
            bool                    makeECEF,
            osg::Geometry*          osgGeom,
            const osg::Matrixd      &world2local);

        osg::Geode* processPolygons        (FeatureList& input, FilterContext& cx);
        osg::Group* processLines           (FeatureList& input, FilterContext& cx);
        osg::Group* processPolygonizedLines(FeatureList& input, bool twosided, FilterContext& cx);
        osg::Geode* processPoints          (FeatureList& input, FilterContext& cx);
    };

} } // namespace osgEarth::Features

#endif // OSGEARTHFEATURES_BUILD_GEOMETRY_FILTER_H
