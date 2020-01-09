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
#ifndef OSGEARTH_FEATURES_GEOMETRY_COMPILER_H
#define OSGEARTH_FEATURES_GEOMETRY_COMPILER_H 1

#include <osgEarthFeatures/Common>
#include <osgEarthFeatures/Feature>
#include <osgEarthFeatures/FeatureCursor>
#include <osgEarthFeatures/ResampleFilter>
#include <osgEarthSymbology/Style>
#include <osgEarth/GeoMath>
#include <osgEarth/ShaderUtils>

namespace osgEarth { namespace Features
{
    using namespace osgEarth;
    using namespace osgEarth::Symbology;

    class OSGEARTHFEATURES_EXPORT GeometryCompilerOptions
    {
    public:
        /**
         * Set the global default values for the options.
         */
        static void setDefaults(const GeometryCompilerOptions& defaults);

    public:
        /**
         * Construct new copiler options, optionally deserializing them
         */
        GeometryCompilerOptions(const ConfigOptions& conf =ConfigOptions());

    public:
        /** Maximum span of a generated edge, in degrees. Applicable to geocentric maps only */
        optional<double>& maxGranularity() { return _maxGranularity_deg; }
        const optional<double>& maxGranularity() const { return _maxGranularity_deg; }

        /** Interpolation type to use for geodetic points */
        optional<GeoInterpolation>& geoInterp() { return _geoInterp; }
        const optional<GeoInterpolation>& geoInterp() const { return _geoInterp; }

        /** Whether to merge geometry from multiple features */
        optional<bool>& mergeGeometry() { return _mergeGeometry; }
        const optional<bool>& mergeGeometry() const { return _mergeGeometry; }

        /** Expression to evaluate to extract a feature's readable name */
        optional<StringExpression>& featureName() { return _featureNameExpr; }
        const optional<StringExpression>& featureName() const { return _featureNameExpr; }

        /** Whether to cluster feature geometries together for speed */
        optional<bool>& clustering() { return _clustering; }
        const optional<bool>& clustering() const { return _clustering; }

        /** Whether to enabled draw-instancing for model substitution */
        optional<bool>& instancing() { return _instancing; }
        const optional<bool>& instancing() const { return _instancing; }

        /** Whether to ignore the altitude filter (e.g. if you plan to do auto-clamping layer) */
        optional<bool>& ignoreAltitudeSymbol() { return _ignoreAlt; }
        const optional<bool>& ignoreAltitudeSymbol() const { return _ignoreAlt; }

        //todo: merge this with geoInterp()
        optional<osgEarth::Features::ResampleFilter::ResampleMode>& resampleMode() { return _resampleMode;}
        const optional<osgEarth::Features::ResampleFilter::ResampleMode>& resampleMode() const { return _resampleMode;}

        optional<double>& resampleMaxLength() { return _resampleMaxLength; }
        const optional<double>& resampleMaxLength() const { return _resampleMaxLength;}

        /** Whether to generate shader components on compiled geometry */
        optional<ShaderPolicy>& shaderPolicy() { return _shaderPolicy; }
        const optional<ShaderPolicy>& shaderPolicy() const { return _shaderPolicy; }

        /** Whether to run consolidate equivalent state attributes for better performance. */
        optional<bool>& optimizeStateSharing() { return _optimizeStateSharing; }
        const optional<bool>& optimizeStateSharing() const { return _optimizeStateSharing; }

        /** Whether to run the optimizer on the resulting group. */
        optional<bool>& optimize() { return _optimize; }
        const optional<bool>& optimize() const { return _optimize; }

        /** Whether to run the vertex order optimizer on geometry. */
        optional<bool>& optimizeVertexOrdering() { return _optimizeVertexOrdering; }
        const optional<bool>& optimizeVertexOrdering() const { return _optimizeVertexOrdering; }

        /** Whether to run a geometry validation pass on the resulting group. This is for debugging
        purposes and will dump issues to the console. */
        optional<bool>& validate() { return _validate; }
        const optional<bool>& validate() const { return _validate; }

        /** Maximum size (angle, degrees) of a polygon tile, when breaking up a large polygon for tessellation;
        only applies to geocentric maps (detault = 5.0) */
        optional<float>& maxPolygonTilingAngle() { return _maxPolyTilingAngle; }
        const optional<float>& maxPolygonTilingAngle() const { return _maxPolyTilingAngle; }

        /** Whether to use GPU-generated geometry for screen-space (pixel) width lines (default=false) */
        optional<bool>& useGPUScreenSpaceLines() { return _useGPULines; }
        const optional<bool>& useGPUScreenSpaceLines() const { return _useGPULines; }

    public:
        Config getConfig() const;

    protected:
        void fromConfig( const Config& conf );

    private:
        optional<double>               _maxGranularity_deg;
        optional<GeoInterpolation>     _geoInterp;
        optional<bool>                 _mergeGeometry;
        optional<StringExpression>     _featureNameExpr;
        optional<bool>                 _clustering;
        optional<bool>                 _instancing;
        optional<ResampleFilter::ResampleMode> _resampleMode;
        optional<double>               _resampleMaxLength;
        optional<bool>                 _ignoreAlt;
        optional<ShaderPolicy>         _shaderPolicy;
        optional<bool>                 _optimizeStateSharing;
        optional<bool>                 _optimize;
        optional<bool>                 _optimizeVertexOrdering;
        optional<bool>                 _validate;
        optional<float>                _maxPolyTilingAngle;
        optional<bool>                 _useGPULines;


        static GeometryCompilerOptions s_defaults;

    public:
       GeometryCompilerOptions(bool); // internal
    };


    /**
     * Compiles a collection of features against a style.
     */
    class OSGEARTHFEATURES_EXPORT GeometryCompiler
    {
    public:

        /** Constructs a new geometry compiler with default options. */
        GeometryCompiler();

        /** Constructs a new compiler with preconfigured options. */
        GeometryCompiler( const GeometryCompilerOptions& options );
        
        virtual ~GeometryCompiler() { }
        
        /** Access the options read-only */
        const GeometryCompilerOptions& options() const { return _options; }

        /** Access the options for editing. */
        GeometryCompilerOptions& options() { return _options; }

    public:

        /** Compiles a collection of features into an OSG scene graph. */
        osg::Node* compile(
            FeatureCursor*        input,
            const Style&          style,
            const FilterContext&  context);

        osg::Node* compile(
            Feature*              input,
            const Style&          style,
            const FilterContext&  context);

        osg::Node* compile(
            Feature*              input,
            const FilterContext&  context);

        osg::Node* compile(
            Geometry*             geom,
            const Style&          style,
            const FilterContext&  context);

        osg::Node* compile(
            Geometry*             geom,
            const Style&          style);

        osg::Node* compile(
            Geometry*             geom,
            const FilterContext&  context);

        osg::Node* compile(
            FeatureList&          mungeableInput,
            const Style&          style,
            const FilterContext&  context);

    protected:
        GeometryCompilerOptions _options;
    };

} } // namespace osgEarth::Features

#endif // OSGEARTH_FEATURES_GEOMETRY_COMPILER_H
