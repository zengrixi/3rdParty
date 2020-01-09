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
#ifndef OSGEARTH_GEODATA_H
#define OSGEARTH_GEODATA_H 1

#include <osgEarth/Common>
#include <osgEarth/GeoCommon>
#include <osgEarth/Bounds>
#include <osgEarth/SpatialReference>
#include <osgEarth/Units>
#include <osgEarth/ImageUtils>

#include <osg/Referenced>
#include <osg/Image>
#include <osg/Shape>
#include <osg/Polytope>

namespace osgEarth
{
    class TerrainResolver;
    class GeoExtent;

    /**
     * A georeferenced 3D point.
     */
    class OSGEARTH_EXPORT GeoPoint
    {
    public:

        /**
         * Constructs a GeoPoint.
         */
        GeoPoint(
            const SpatialReference* srs,
            double x,
            double y,
            double z,
            const AltitudeMode& mode );

        /**
         * Constructs a GeoPoint with an absolute Z.
         */
        GeoPoint(
            const SpatialReference* srs,
            double x,
            double y,
            double z );

        /**
         * Constructs a GeoPoint with X and Y coordinates. The Z defaults
         * to zero with an ALTMODE_RELATIVE altitude mode (i.e., 0 meters
         * above the terrain).
         */
        GeoPoint(
            const SpatialReference* srs,
            double x,
            double y );

        /**
         * Constructs a GeoPoint from a vec3.
         */
        GeoPoint(
            const SpatialReference* srs,
            const osg::Vec3d&       xyz,
            const AltitudeMode&     mode );

        /**
         * Constructs a GeoPoint from a vec3 with absolute Z.
         */
        GeoPoint(
            const SpatialReference* srs,
            const osg::Vec3d&       xyz);

        /**
         * Constructs a new GeoPoint by transforming an existing GeoPoint into
         * the specified spatial reference.
         */
        GeoPoint(
            const SpatialReference* srs,
            const GeoPoint&         rhs );

        /**
         * Copy constructor
         */
        GeoPoint(const GeoPoint& rhs);

        /**
         * Constructs an empty (and invalid) geopoint.
         */
        GeoPoint();

        /**
         * Constructs a geopoint from serialization
         */
        GeoPoint( const Config& conf, const SpatialReference* srs =0L );

        /** dtor */
        virtual ~GeoPoint() { }

        /**
         * Sets the SRS and coords 
         */
        void set(
            const SpatialReference* srs,
            const osg::Vec3d&       xyz,
            const AltitudeMode&     mode );

        void set(
            const SpatialReference* srs,
            double                  x,
            double                  y,
            double                  z,
            const AltitudeMode&     mode );

        // component getter/setters

        double& x() { return _p.x(); }
        double  x() const { return _p.x(); }

        double& y() { return _p.y(); }
        double  y() const { return _p.y(); }

        double& z() { return _p.z(); }
        double  z() const { return _p.z(); }

        double& alt() { return _p.z(); }
        double  alt() const { return _p.z(); }

        osg::Vec3d& vec3d() { return _p; }
        const osg::Vec3d& vec3d() const { return _p; }

        const SpatialReference* getSRS() const { return _srs.get(); }

        /**
         * AltitudeMode reflects whether the Z coordinate is absolute with
         * respect to MSL or relative to the terrain elevation at that
         * point. When using relative points, GeoPoint usually requires
         * access to a TerrainProvider in order to resolve the altitude.
         */
        AltitudeMode& altitudeMode() { return _altMode; }
        const AltitudeMode& altitudeMode() const { return _altMode; }
        bool isRelative() const { return _altMode == ALTMODE_RELATIVE; }
        bool isAbsolute() const { return _altMode == ALTMODE_ABSOLUTE; }

        /**
         * Returns a copy of this geopoint transformed into another SRS.
         */
        GeoPoint transform(const SpatialReference* outSRS) const;

        /**
         * Transforms this geopoint into another SRS.
         */
        bool transform(const SpatialReference* outSRS, GeoPoint& output) const;

        /**
         * Transforms this point in place to another SRS.
         */
        bool transformInPlace(const SpatialReference* srs);

        /**
         * Transforms this geopoint's Z coordinate (in place)
         */
        bool transformZ(const AltitudeMode& altMode, const TerrainResolver* t );

        /**
         * Transforms and returns the geopoints Z coordinate.
         */
        bool transformZ(const AltitudeMode& altMode, const TerrainResolver* t, double& out_z) const;

        /**
         * Transforms this geopoint's Z to be absolute w.r.t. the vertical datum
         */
        bool makeAbsolute(const TerrainResolver* t) { return transformZ(ALTMODE_ABSOLUTE, t); }

        /**
         * Transforms this geopoint's Z to be terrain-relative.
         */
        bool makeRelative(const TerrainResolver* t) { return transformZ(ALTMODE_RELATIVE, t); }

        /**
         * Transforms this GeoPoint to geographic (lat/long) coords in place.
         */
        bool makeGeographic();

        /**
         * Outputs world coordinates corresponding to this point. If the point
         * is ALTMODE_RELATIVE, this will fail because there's no way to resolve
         * the actual Z coordinate. Use the variant of toWorld that takes a
         * Terrain* instead.
         */
        bool toWorld( osg::Vec3d& out_world ) const;

        /**
         * Outputs world coordinates corresponding to this point, passing in a Terrain
         * object that will be used if the point needs to be converted to absolute
         * altitude
         */
        bool toWorld( osg::Vec3d& out_world, const TerrainResolver* terrain ) const;

        /**
         * Converts world coordinates into a geopoint
         */
        bool fromWorld(const SpatialReference* srs, const osg::Vec3d& world);

        /**
         * geopoint into absolute world coords.
         */
        bool createLocalToWorld( osg::Matrixd& out_local2world ) const;

        /**
         * Outputs a matrix that will transform absolute world coordiantes so they are
         * localized into a local tangent place around this geopoint.
         */
        bool createWorldToLocal( osg::Matrixd& out_world2local ) const;

        /**
         * Outputs an "up" vector corresponding to the given point. The up vector
         * is orthogonal to a local tangent plane at that point on the map.
         */
        bool createWorldUpVector( osg::Vec3d& out_up ) const;

        /**
         * The great-circle distance, in meters, from this point to another.
         * This calculation does not consider altitude.
         */
        double distanceTo(const GeoPoint& rhs) const;

        /**
         * Interpolates a point between this point and another point,
         * along a great circle, using the parameter t [0..1]. It will also
         * interpolate the altitude value.
         */
        GeoPoint interpolate(const GeoPoint& rhs, double t) const;


        bool operator == (const GeoPoint& rhs) const;
        bool operator != (const GeoPoint& rhs) const { return !operator==(rhs); }
        bool isValid() const { return _srs.valid(); }

        Config getConfig() const;

        /**
         * Represent this point as a string
         */
        std::string toString() const;

    public:
        static GeoPoint INVALID;

    protected:
        osg::ref_ptr<const SpatialReference> _srs;
        osg::Vec3d       _p;
        AltitudeMode _altMode;
    };


    /**
     * A simple circular bounding area consiting of a GeoPoint and a linear radius.
     */
     class OSGEARTH_EXPORT GeoCircle
     {
     public:
         /** Construct an INVALID GeoCircle */
        GeoCircle();

        /** Copy another GoeCircle */
        GeoCircle(const GeoCircle& rhs);

        /** Construct a new GeoCircle */
        GeoCircle(
             const GeoPoint& center,
             double          radius );

        virtual ~GeoCircle() { }

        /** The center point of the circle */
        const GeoPoint& getCenter() const { return _center; }
        void setCenter( const GeoPoint& value ) { _center = value; }

        /** Circle's radius, in linear map units (or meters for a geographic SRS) */
        double getRadius() const { return _radius; }
        void setRadius( double value ) { _radius = value; }

        /** SRS of the center point */
        const SpatialReference* getSRS() const { return _center.getSRS(); }

        /** equality test */
        bool operator == ( const GeoCircle& rhs ) const;

        /** inequality test */
        bool operator != ( const GeoCircle& rhs ) const { return !operator==(rhs); }

        /** validity test */
        bool isValid() const { return _center.isValid() && _radius > 0.0; }

        /** transform the GeoCircle to another SRS */
        GeoCircle transform( const SpatialReference* srs ) const;

        /** transform the GeoCircle to another SRS */
        bool transform( const SpatialReference* srs, GeoCircle& out_circle ) const;

        /** does this GeoCircle intersect another? */
        bool intersects( const GeoCircle& rhs ) const;

     public:

         static GeoCircle INVALID;

     protected:
         GeoPoint _center;
         double   _radius;
     };
}
OSGEARTH_SPECIALIZE_CONFIG(osgEarth::GeoPoint);

namespace osgEarth
{
    /**
     * An axis-aligned geospatial extent. A bounding box that is aligned with a
     * spatial reference's coordinate system.
     */
    class OSGEARTH_EXPORT GeoExtent
    {
    public:
        /** Default ctor creates an "invalid" extent */
        GeoExtent(); 

        /** Contructs a valid extent */
        GeoExtent(
            const SpatialReference* srs,
            double west, double south,
            double east, double north );

        /** Contructs an invalid extent that you can grow with the expandToInclude method */
        GeoExtent( const SpatialReference* srs );

        /** Copy ctor */
        GeoExtent( const GeoExtent& rhs );

        /** create from Bounds object */
        GeoExtent( const SpatialReference* srs, const Bounds& bounds );

        /** dtor */
        virtual ~GeoExtent() { }

        //! Set from the SW and NE corners.
        void set(double west, double south, double east, double north);

        bool operator == ( const GeoExtent& rhs ) const;
        bool operator != ( const GeoExtent& rhs ) const;

        /** Gets the spatial reference system underlying this extent. */
        const SpatialReference* getSRS() const { return _srs.get(); }

        //! Coordinates of the bounding edges, normalized for the lat/long frame if necessary
        inline double west() const { return _west; }
        inline double east() const { return normalizeX(_west+_width); }
        inline double south() const { return _south; }
        inline double north() const { return _south+_height; }

        //! Coordinates of the bounds, NOT normalized to the lat/long frame.
        inline double xMin() const { return _west; }
        inline double xMax() const { return _west + _width; }
        inline double yMin() const { return _south; }
        inline double yMax() const { return _south + _height; }

        //! East-to-west span of the extent
        inline double width() const { return _width; }

        //! North-to-south span of the extent
        inline double height() const { return _height; }

        /**
         * Gets the centroid of the bounds
         */
        bool getCentroid( double& out_x, double& out_y ) const;
        osg::Vec3d getCentroid() const { osg::Vec3d r; getCentroid(r.x(), r.y()); return r; }
        bool getCentroid( GeoPoint& output ) const;

        //! True if the extent is geographic and crosses the 180 degree meridian.
        bool crossesAntimeridian() const;

        //! Raw bounds of the extent (unnormalized)
        void getBounds(double& xmin, double& ymin, double& xmax, double& ymax) const;

        /** True if this object defines a real, valid extent with positive area */
        bool isValid() const;
        bool isInvalid() const { return !isValid(); }

        /**
         * If this extent crosses the international date line, populates two extents, one for
         * each side, and returns true. Otherwise returns false and leaves the reference
         * parameters untouched.
         */
        bool splitAcrossAntimeridian( GeoExtent& first, GeoExtent& second ) const;

        /**
         * Returns this extent transformed into another spatial reference. 
         *
         * NOTE! It is possible that the target SRS will not be able to accomadate the
         * extents of the source SRS. (For example, transforming a full WGS84 extent
         * to Mercator will resultin an error since Mercator does not cover the entire
         * globe.) Consider using Profile:clampAndTransformExtent() instead of using
         * this method directly.
         */
        GeoExtent transform( const SpatialReference* to_srs ) const;

        /**
         * Same as transform(srs) but puts the result in the output extent
         */
        bool transform( const SpatialReference* to_srs, GeoExtent& output ) const;

        /**
         * Returns true if the specified point falls within the bounds of the extent.
         *
         * @param x, y
         *      Coordinates to test
         * @param xy_srs
         *      SRS of input x and y coordinates; if null, the method assumes x and y
         *      are in the same SRS as this object.
         */
        bool contains(double x, double y, const SpatialReference* srs =0L) const;

        bool contains(const osg::Vec3d& xy, const SpatialReference* srs =0L) const { return contains(xy.x(),xy.y(),srs); }

        /**
         * Returns true if the point falls within this extent.
         */
        bool contains( const GeoPoint& rhs ) const;

        /**
         * Returns true if this extent fully contains another extent.
         */
        bool contains( const GeoExtent& rhs ) const;

        /**
         * Returns true if this extent fully contains the target bounds.
         */
        bool contains( const Bounds& rhs ) const;

        /**
         * Returns TRUE if this extent intersects another extent.
         * @param[in ] rhs      Extent against which to perform intersect test
         * @param[in ] checkSRS If false, assume the extents have the same SRS (don't check).
         */
        bool intersects( const GeoExtent& rhs, bool checkSRS =true ) const;

        /**
         * Copy of the anonymous bounding box
         */
        Bounds bounds() const;

        /**
         * Gets a geo circle bounding this extent.
         */
        GeoCircle computeBoundingGeoCircle() const;

        /**
         * Grow this extent to include the specified point (which is assumed to be
         * in the extent's SRS.
         */
        void expandToInclude( double x, double y ); 
        void expandToInclude( const osg::Vec3d& v ) { expandToInclude(v.x(), v.y()); }
        //void expandToInclude( const Bounds& bounds );

        /**
         * Expand this extent to include the bounds of another extent.
         */
        bool expandToInclude( const GeoExtent& rhs );
        
        /**
         * Intersect this extent with another extent in the same SRS and return the
         * result.
         */
        GeoExtent intersectionSameSRS( const GeoExtent& rhs ) const; //const Bounds& rhs ) const;

        /**
         * Returns a human-readable string containing the extent data (without the SRS)
         */
        std::string toString() const;

        /**
         *Inflates this GeoExtent by the given ratios
         */
        void scale(double x_scale, double y_scale);

        /**
         * Expands the extent by x and y.
         */
        void expand( double x, double y );

        /**
         *Gets the area of this GeoExtent
         */
        double area() const;

        /**
         * Generate a polytope in world coordinates that bounds the extent.
         * Return false if the extent it invalid.
         */
        bool createPolytope(osg::Polytope& out) const;

        /**
         * Computes a scale/bias matrix that transforms parametric coordinates [0..1]
         * from this extent into the target extent. Return false if the extents are
         * incompatible (different SRS, etc.)
         *
         * For example, if this extent is 100m wide, and the target extent is
         * 200m wide, the output matrix will have an x_scale = 0.5.
         *
         * Note! For the sake of efficiency, this method does NOT check for 
         * validity nor for SRS equivalence -- so be sure to validate those beforehand.
         * It also assumes the output matrix is preset to the identity.
         */
        bool createScaleBias(const GeoExtent& target, osg::Matrixd& output) const;
        bool createScaleBias(const GeoExtent& target, osg::Matrixf& output) const;

        /**
         * Generates a BoundingSphere encompassing the extent and a vertical 
         * volume, in world coordinates.
         */
        osg::BoundingSphered createWorldBoundingSphere(double minElev, double maxElev) const;

    public:
        static GeoExtent INVALID;

    private:
        osg::ref_ptr<const SpatialReference> _srs;
        double _west, _width, _south, _height;

        double normalizeX( double longitude ) const;

        void clamp();

        bool isGeographic() const;

        void setOriginAndSize(double west, double south, double width, double height);
    };


    /**
     * A geospatial area with tile data LOD extents
     */
    class OSGEARTH_EXPORT DataExtent : public GeoExtent
    {
    public:
        DataExtent(const GeoExtent& extent);
        DataExtent(const GeoExtent& extent, const std::string &description);
        DataExtent(const GeoExtent& extent, unsigned minLevel );
        DataExtent(const GeoExtent& extent, unsigned minLevel, const std::string &description);
        DataExtent(const GeoExtent& extent, unsigned minLevel, unsigned maxLevel);
        DataExtent(const GeoExtent& extent, unsigned minLevel, unsigned maxLevel, const std::string &description);

        /** dtor */
        virtual ~DataExtent() { }

        /** The minimum LOD of the extent */
        optional<unsigned>& minLevel() { return _minLevel; }
        const optional<unsigned>& minLevel() const { return _minLevel; }

        /** The maximum LOD of the extent */
        optional<unsigned>& maxLevel() { return _maxLevel; }
        const optional<unsigned>& maxLevel() const { return _maxLevel; }

        /** description for the data extents */
        const osgEarth::optional<std::string>& description() const { return _description; }

    private:
        optional<unsigned> _minLevel;
        optional<unsigned> _maxLevel;
        optional<std::string> _description;
    };

    typedef std::vector< DataExtent > DataExtentList;


    /**
     * A georeferenced image; i.e. an osg::Image and an associated GeoExtent with SRS.
     */
    class OSGEARTH_EXPORT GeoImage
    {
    public:
        /** Construct an empty (invalid) geoimage. */
        GeoImage();

        /** 
         * Constructs a new goereferenced image.
         */
        GeoImage(osg::Image* image, const GeoExtent& extent);

        /** dtor */
        virtual ~GeoImage() { }

        static GeoImage INVALID;

    public:
        /**
         * True if this is a valid geo image. 
         */
        bool valid() const;

        /**
         * Gets a pointer to the underlying OSG image.
         */
        osg::Image* getImage() const;

        /**
         * Gets the geospatial extent of the image.
         */
        const GeoExtent& getExtent() const;

        /**
         * Shortcut to get the spatial reference system describing
         * the projection of the image.
         */
        const SpatialReference* getSRS() const;

        /**
         * Crops the image to a new geospatial extent. 
         *
         * @param extent
         *      New extent to which to crop the image.
         * @param exact
         *      If "exact" is true, the output image will have exactly the extents requested;
         *      this process may require resampling and will therefore be more expensive. If
         *      "exact" is false, we do a simple crop of the image that is rounded to the nearest
         *      pixel. The resulting extent will be close but usually not exactly what was
         *      requested - however, this method is faster.
         * @param width, height
         *      New pixel size for the output image. By default, the method will automatically
         *      calculate a new pixel size.
         */
        GeoImage crop( 
            const GeoExtent& extent,
            bool exact = false,
            unsigned int width = 0,
            unsigned int height = 0,
            bool useBilinearInterpolation = true) const;

        /**
         * Warps the image into a new spatial reference system.
         *
         * @param to_srs
         *      SRS into which to warp the image.
         * @param to_extent
         *      Supply this extent if you wish to warp AND crop the image in one step. This is
         *      faster than calling reproject() and then crop().
         * @param width, height
         *      New pixel size for the output image. Be default, the method will automatically
         *      calculate a new pixel size.
         */
        GeoImage reproject(
            const SpatialReference* to_srs,
            const GeoExtent* to_extent = 0,
            unsigned int width = 0,
            unsigned int height = 0,
            bool useBilinearInterpolation = true) const;

        /**
         * Adds a one-pixel transparent border around an image.
         */
        GeoImage addTransparentBorder(
            bool leftBorder=true, 
            bool rightBorder=true, 
            bool bottomBorder=true, 
            bool topBorder=true);

        /**
         * Sets alpha to Zero for any pixels that do NOT intersect the masking extent.
         */
        void applyAlphaMask(const GeoExtent& maskingExtent);

        /**
         * Returns the underlying OSG image and releases the reference pointer.
         */
        osg::Image* takeImage();

		/**
		 * Gets the units per pixel of this geoimage
		 */
		double getUnitsPerPixel() const;

    private:
        osg::ref_ptr<osg::Image> _image;
        GeoExtent                _extent;
    };

    typedef std::vector<GeoImage> GeoImageVector;


    class OSGEARTH_EXPORT NormalMap : public osg::Image
    {
    public:
        NormalMap(unsigned s, unsigned t);

        void set(unsigned s, unsigned t, const osg::Vec3& normal, float curvature =0.0f);

        osg::Vec3 getNormal(unsigned s, unsigned t) const;

        osg::Vec3 getNormalByUV(double u, double v) const;

        float getCurvature(unsigned s, unsigned t) const;

        virtual ~NormalMap();

    private:
        ImageUtils::PixelWriter* _write;
        ImageUtils::PixelReader* _read;
    };


    /**
     * A georeferenced heightfield and associated normal/curvature map.
     */
    class OSGEARTH_EXPORT GeoHeightField
    {
    public:
        /** Constructs an empty (invalid) heightfield. */
        GeoHeightField();

        /**
         * Constructs a new georeferenced heightfield.
         */
        GeoHeightField(
            osg::HeightField* heightField,
            const GeoExtent&  extent );

        //! Constructs a new GeoHeightField
        //! @param[in] heightField Elevation grid
        //! @param[in] normalMap   Normal/Curvature map
        //! @param[in] extent      Geospatial extent of the data
        GeoHeightField(
            osg::HeightField* heightField,
            NormalMap*        normalMap,
            const GeoExtent&  extent);

        /** dtor */
        virtual ~GeoHeightField() { }

        static GeoHeightField INVALID;

        /**
         * True if this is a valid heightfield. 
         */
        bool valid() const;

        /**
         * Gets the elevation value at a specified point.
         *
         * @param srs
         *      Spatial reference of the query coordinates. (If you pass in NULL, the method
         *      will assume that the SRS is equivalent to that of the GeoHeightField. Be sure
         *      this is case of you will get incorrect results.)
         * @param x, y
         *      Coordinates at which to query the elevation value.
         * @param interp
         *      Interpolation method for the elevation query.
         * @param srsWithOutputVerticalDatum
         *      Transform the output elevation value according to the vertical datum 
         *      associated with this SRS. If the SRS is NULL, assume a geodetic vertical datum
         *      relative to this object's reference ellipsoid.
         * @param out_elevation
         *      Output: the elevation value
         * @return
         *      True if the elevation query was succesful; false if not (e.g. if the query
         *      fell outside the geospatial extent of the heightfield)
         */
        bool getElevation(
            const SpatialReference* inputSRS, 
            double                  x,
            double                  y,
            ElevationInterpolation  interp,
            const SpatialReference* srsWithOutputVerticalDatum,
            float&                  out_elevation ) const;

        bool getElevationAndNormal(
            const SpatialReference* inputSRS, 
            double                  x,
            double                  y,
            ElevationInterpolation  interp,
            const SpatialReference* srsWithOutputVerticalDatum,
            float&                  out_elevation,
            osg::Vec3&              out_normal ) const;

        //! Gets the elevation at a point (must be in the same SRS; bilinear interpolation)
        float getElevation(double x, double y) const;

        //! Gets the normal at a point (must be in the same SRS; bilinear interpolation)
        osg::Vec3 getNormal(double x, double y) const;
        
        /**
         * Subsamples the heightfield, returning a new heightfield corresponding to
         * the destEx extent. The destEx must be a smaller, inset area of sourceEx.
         */
        GeoHeightField createSubSample( const GeoExtent& destEx, unsigned int width, unsigned int height, ElevationInterpolation interpolation) const;

        /**
         * Gets the geospatial extent of the heightfield.
         */
        const GeoExtent& getExtent() const;

        /**
         * The minimum height in the heightfield
         */
        float getMinHeight() const { return _minHeight; }

        /**
         * The maximum height in the heightfield
         */
        float getMaxHeight() const { return _maxHeight; }

        /**
         * Gets a pointer to the underlying OSG heightfield.
         */
        const osg::HeightField* getHeightField() const;

        //! Normal map
        const NormalMap* getNormalMap() const;

        /**
         * Gets the X interval of this GeoHeightField
         */
        double getXInterval() const;

        /**
         * Gets the Y interval of this GeoHeightField
         */
        double getYInterval() const;


        //Sort GeoHeightField's by resolution
        struct SortByResolutionFunctor
        {
            inline bool operator() (const GeoHeightField& lhs, const GeoHeightField& rhs) const
            {                
                return lhs.getXInterval() < rhs.getXInterval();                
            }
        };

    protected:
        osg::ref_ptr<osg::HeightField> _heightField;
        osg::ref_ptr<NormalMap>        _normalMap;
        GeoExtent                      _extent;
        float                          _minHeight, _maxHeight;

        void init();
    };

	typedef std::vector<GeoHeightField> GeoHeightFieldVector;




}

#endif // OSGEARTH_GEODATA_H
