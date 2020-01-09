/* -*-c++-*- */
/* osgEarth - Geospatial SDK for OpenSceneGraph
* Copyright 2008-2014 Pelican Mapping
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
#ifndef OSGEARTH_DRIVERS_REX_TERRAIN_ENGINE_TILE_DRAWABLE
#define OSGEARTH_DRIVERS_REX_TERRAIN_ENGINE_TILE_DRAWABLE 1

#include "Common"
#include "TileRenderModel"
#include "GeometryPool"
#include <osg/Geometry>
#include <osg/Image>
#include <osg/Matrixf>
#include <osgEarth/TileKey>
#include <osgEarth/Map>

using namespace osgEarth;

namespace osgEarth { namespace Drivers { namespace RexTerrainEngine
{
    class EngineContext;

    struct ModifyBoundingBoxCallback : public osg::Referenced
    {
        ModifyBoundingBoxCallback(EngineContext* engine);
        void operator()(const TileKey& key, osg::BoundingBox& bbox);
        EngineContext* _engine;
    };

    /**
     * TileDrawable is an osg::Drawable that represents an individual terrain tile
     * for the purposes of scene graph operations (like intersections, bounds
     * computation, statistics, etc.)
     * 
     * NOTE: TileDrawable does not actually render anything!
     * The TerrainRenderData object does all the rendering of tiles.
     *
     * Instead, it exposes various osg::Drawable Functors for traversing
     * the terrain's geometry. It also hold a pointer to the tile's elevation
     * raster so it can properly reflect the elevation data in the texture.
     */
    class TileDrawable : public osg::Drawable
    {
    public:
        // underlying geometry, possibly shared between this tile and other.
        osg::ref_ptr<SharedGeometry> _geom;

        // tile dimensions
        int _tileSize;

        const TileKey _key;

        osg::ref_ptr<const osg::Image> _elevationRaster;
        osg::Matrixf                   _elevationScaleBias;

        // cached 3D mesh of the terrain tile (derived from the elevation raster)
        osg::Vec3f* _mesh;
        GLuint* _meshIndices;

        osg::BoundingBox _bboxOffsets;
        ModifyBoundingBoxCallback* _bboxCB;
        mutable float _bboxRadius;

    public:
        
        // construct a new TileDrawable that fronts an osg::Geometry
        TileDrawable(
            const TileKey& key,
            SharedGeometry* geometry,
            int            tileSize);

    public:

        // Sets the elevation raster for this tile
        void setElevationRaster(const osg::Image* image, const osg::Matrixf& scaleBias);

        const osg::Image* getElevationRaster() const {
            return _elevationRaster.get();
        }

        const osg::Matrixf& getElevationMatrix() const {
            return _elevationScaleBias;
        }

        // Set the render model so we can properly calculate bounding boxes
        void setModifyBBoxCallback(ModifyBoundingBoxCallback* bboxCB) { _bboxCB = bboxCB; }

        float getRadius() const { return _bboxRadius; }

    public: // osg::Drawable overrides

        // These methods defer functors (like stats collection) to the underlying
        // (possibly shared) geometry instance.
        bool supports(const osg::Drawable::AttributeFunctor& f) const { return true; }
        void accept(osg::Drawable::AttributeFunctor& f) { if ( _geom.valid() ) _geom->accept(f); }

        bool supports(const osg::Drawable::ConstAttributeFunctor& f) const { return true; }
        void accept(osg::Drawable::ConstAttributeFunctor& f) const { if ( _geom.valid() ) _geom->accept(f); }

        bool supports(const osg::PrimitiveFunctor& f) const { return true; }
        void accept(osg::PrimitiveFunctor& f) const;
        
        /** indexed functor is NOT supported since we need to apply elevation dynamically */
        bool supports(const osg::PrimitiveIndexFunctor& f) const { return false; }

        osg::BoundingSphere computeBound() const;
        osg::BoundingBox computeBoundingBox() const;

    public:
        META_Object(osgEarth, TileDrawable);
        TileDrawable() : osg::Drawable(){}
        TileDrawable(const TileDrawable& rhs, const osg::CopyOp& cop) : osg::Drawable(rhs, cop) {}

        virtual ~TileDrawable();
    };

} } } // namespace osgEarth::Drivers::RexTerrainEngine

#endif // OSGEARTH_DRIVERS_REX_TERRAIN_ENGINE_RexGEOMETRY

