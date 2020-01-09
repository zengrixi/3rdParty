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
#ifndef OSGEARTH_VERTICAL_DATUM_H
#define OSGEARTH_VERTICAL_DATUM_H 1

#include <osgEarth/Common>
#include <osgEarth/Geoid>
#include <osgEarth/Units>
#include <osg/Shape>

namespace osgEarth
{
    class OSGEARTH_EXPORT GeoExtent;

    /** 
     * Reference information for vertical (height) information.
     */
    class OSGEARTH_EXPORT VerticalDatum : public osg::Object
    {
    public:
        META_Object(osgEarth, VerticalDatum);

        /**
         * Creates an vertical datum from an initialization string. This method
         * uses an internal cache so that there is only ever one instance or each
         * unique vertical datum.
         */
        static VerticalDatum* get( const std::string& init );


    public: // static transform methods

        /**
         * Transforms a Z coordinate from one vertical datum to another.
         */
        static bool transform(
            const VerticalDatum* from,
            const VerticalDatum* to,
            double               lat_deg,
            double               lon_deg,
            double&              in_out_z );

        /**
         * Transforms a Z coordinate from one vertical datum to another.
         */
        static bool transform(
            const VerticalDatum* from,
            const VerticalDatum* to,
            double               lat_deg,
            double               lon_deg,
            float&               in_out_z );

        /**
         * Transforms the values in a height field from one vertical datum to another.
         */
        static bool transform(
            const VerticalDatum* from,
            const VerticalDatum* to,
            const GeoExtent&     extent,
            osg::HeightField*    hf );


    public: // raw transformations

        /**
         * Converts an MSL value (height relative to a mean sea level model) to the
         * corresponding HAE value (height above the model's reference ellipsoid)
         */
        virtual double msl2hae( double lat_deg, double lon_deg, double msl ) const;

        /**
         * Converts an HAE value (height above the model's reference ellipsoid) to the
         * corresponding MSL value (height relative to a mean sea level model)
         */
        virtual double hae2msl(double lat_deg, double lon_deg, double hae) const;


    public: // properties

        /** Gets the readable name of this SRS. */
        const std::string& getName() const { return _name; }

        /** Gets the linear units of height values */
        const Units& getUnits() const { return _units; }

        /** Gets the string that was used to initialize this SRS */
        const std::string& getInitString() const { return _initString; }

        /** Gets the underlying geoid */
        const Geoid* getGeoid() const { return _geoid.get(); }

        /** Tests this SRS for equivalence with another. */
        virtual bool isEquivalentTo( const VerticalDatum* rhs ) const;
        
    public:

        /** Creates a geoid-based VSRS. */
        VerticalDatum(
            const std::string& name,
            const std::string& initString,
            Geoid*             geoid =0L );

        /** Creates a simple ellipsoidal VSRS. */
        VerticalDatum( const Units& units );

        /** dtor */
        virtual ~VerticalDatum() { }

    protected:
        // required by META_Object, but not used.
        VerticalDatum() { }
        VerticalDatum(const VerticalDatum& rhs, const osg::CopyOp& op) { }

        std::string         _name;
        std::string         _initString;
        osg::ref_ptr<Geoid> _geoid;
        Units               _units;
    };

    //--------------------------------------------------------------------

    /**
     * Creates a geoid instance based on an initialization string.
     */
    class OSGEARTH_EXPORT VerticalDatumFactory
    {   
	public:
        static VerticalDatum* create( const std::string& id );
    };

}

#endif // OSGEARTH_VERTICAL_DATUM_H
