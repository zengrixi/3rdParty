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

#ifndef OSGEARTHSYMBOLOGY_MODEL_SYMBOL_H
#define OSGEARTHSYMBOLOGY_MODEL_SYMBOL_H 1

#include <climits>

#include <osgEarth/Common>
#include <osgEarthSymbology/InstanceSymbol>
#include <osg/Vec3f>

namespace osgEarth { namespace Symbology
{
    class InstanceResource;

    /**
     * Represents an external 3D model
     */
    class OSGEARTHSYMBOLOGY_EXPORT ModelSymbol : public InstanceSymbol
    {
    public:
        META_Object(osgEarthSymbology, ModelSymbol);

        ModelSymbol(const ModelSymbol& rhs,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
        ModelSymbol( const Config& conf =Config() );

        /** dtor */
        virtual ~ModelSymbol() { }

        /** heading in degrees */
        optional<NumericExpression>& heading() { return _heading; }
        const optional<NumericExpression>& heading() const { return _heading; }

        /** pitch in degrees */
        optional<NumericExpression>& pitch() { return _pitch; }
        const optional<NumericExpression>& pitch() const { return _pitch; }
        
        /** roll in degrees */
        optional<NumericExpression>& roll() { return _roll; }
        const optional<NumericExpression>& roll() const { return _roll; }

        /** whether to automatically scale the model from meters to pixels */
        optional<bool>& autoScale() { return _autoScale; }
        const optional<bool>& autoScale() const { return _autoScale; }

		/** minimum scale of the model when autoscale is enabled */
        optional<double>& minAutoScale() { return _minAutoScale; }
        const optional<double>& minAutoScale() const { return _minAutoScale; }
		
		/** maximum scale of the model when autoscale is enabled */
        optional<double>& maxAutoScale() { return _maxAutoScale; }
        const optional<double>& maxAutoScale() const { return _maxAutoScale; }
		
        /** Name of a specific model in the catalog */
        optional<StringExpression>& name() { return _name; }
        const optional<StringExpression>& name() const { return _name; }
        
        /** Maximum acceptable model length (size in the X dimension) */
        optional<float>& maxSizeX() { return _maxSizeX; }
        const optional<float>& maxSizeX() const { return _maxSizeX; }
        
        /** Maximum acceptable model width (size in the Y dimension) */
        optional<float>& maxSizeY() { return _maxSizeY; }
        const optional<float>& maxSizeY() const { return _maxSizeY; }

        /** Model instance scale factor */
        optional<NumericExpression>& scaleX() { return _scaleX; }
        const optional<NumericExpression>& scaleX() const { return _scaleX; }

        /** Model instance scale factor */
        optional<NumericExpression>& scaleY() { return _scaleY; }
        const optional<NumericExpression>& scaleY() const { return _scaleY; }

        /** Model instance scale factor */
        optional<NumericExpression>& scaleZ() { return _scaleZ; }
        const optional<NumericExpression>& scaleZ() const { return _scaleZ; }

        
        
    public: // non-serialized properties (for programmatic use only)

        /** Explicit model to use for model placement */
        void setModel( osg::Node* node ) { _node = node; }
        osg::Node* getModel() const { return _node.get(); }

    public:
        virtual Config getConfig() const;
        virtual void mergeConfig( const Config& conf );
        static void parseSLD(const Config& c, class Style& style);
    
    public: // InstanceSymbol
        /** Creates a new (empty) resource appropriate for this symbol */
        virtual InstanceResource* createResource() const;

    protected:
        optional<NumericExpression>  _heading;
        optional<NumericExpression>  _pitch;
        optional<NumericExpression>  _roll;
        optional<bool>               _autoScale;
		optional<double>			 _minAutoScale;
		optional<double>			 _maxAutoScale;
        osg::ref_ptr<osg::Node>      _node;
        optional<StringExpression>   _name;
        optional<float>              _maxSizeX;
        optional<float>              _maxSizeY;
        optional<NumericExpression>  _scaleX;
        optional<NumericExpression>  _scaleY;
        optional<NumericExpression>  _scaleZ;
    };

} } // namespace osgEarth::Symbology

#endif
