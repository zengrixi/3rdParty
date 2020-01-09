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

#ifndef OSGEARTHFEATURES_CLAMP_FILTER_H
#define OSGEARTHFEATURES_CLAMP_FILTER_H 1

#include <osgEarthFeatures/Common>
#include <osgEarthFeatures/Feature>
#include <osgEarthFeatures/Filter>

namespace osgEarth { namespace Features
{
    using namespace osgEarth;

    /**
     * Feature filter that will clamp incoming feature geometry to an elevation model.
     */
    class OSGEARTHFEATURES_EXPORT AltitudeFilter : public FeatureFilter
    {
    public:
        /** Constructs a new clamping filter */     
        AltitudeFilter();
        
        virtual ~AltitudeFilter() { }

    public: // properties

        /** Shortcut to set any properties that are represented in a style. */
        void setPropertiesFromStyle( const Style& style );

        /** Maximum terrain resolution to consider when clamping */
        void setMaxResolution( double value ) { _maxRes = value; }
        double getMaxResolution() const { return _maxRes; }

    public:
        virtual FilterContext push( FeatureList& input, FilterContext& cx );

    protected:
        osg::ref_ptr<const AltitudeSymbol> _altitude;
        double                             _maxRes;
        std::string                        _maxZAttr, _minZAttr, _terrainZAttr;

        void pushAndClamp( FeatureList& input, FilterContext& cx );
        void pushAndDontClamp( FeatureList& input, FilterContext& cx );
    };

} } // namespace osgEarth::Features

#endif // OSGEARTHFEATURES_CLAMP_FILTER_H
