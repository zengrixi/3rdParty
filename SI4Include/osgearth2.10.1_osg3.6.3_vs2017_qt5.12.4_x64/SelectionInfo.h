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
#ifndef OSGEARTH_DRIVERS_REX_TERRAIN_ENGINE_SELECTION_INFO
#define OSGEARTH_DRIVERS_REX_TERRAIN_ENGINE_SELECTION_INFO 1

#include "Common"
#include <osg/NodeVisitor>
#include <osgEarth/Profile>
#include <vector>


namespace osgEarth { namespace Drivers { namespace RexTerrainEngine
{
    /**
     * SelectionInfo is a data structure that holds the LOD distance switching 
     * information for the terrain, to support paging and LOD morphing. 
     * This is calculated once when the terrain is first created.
     */
    class SelectionInfo
    {
    public:
        struct LOD
        {
            double _visibilityRange;
            double _morphStart;
            double _morphEnd;
        };

    public:
        SelectionInfo() : _firstLOD(0) { }

        //! Initialize the selection into LODs
        void initialize(unsigned firstLod, unsigned maxLod, const Profile* profile, double mtrf);

        //! Number of LODs
        unsigned getNumLODs(void) const { return _lods.size(); }

        //! Visibility and morphing information for a specific LOD
        const LOD& getLOD(unsigned lod) const;

    private:
        std::vector<LOD>    _lods;
        unsigned            _firstLOD;
        static const double _morphStartRatio;
    };

} } } // namespace

#endif
