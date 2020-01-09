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

#ifndef OSGEARTHSYMBOLOGY_INSTANCE_RESOURCE_H
#define OSGEARTHSYMBOLOGY_INSTANCE_RESOURCE_H 1

#include <osgEarthSymbology/Common>
#include <osgEarthSymbology/Resource>
#include <osgEarthSymbology/InstanceSymbol>
#include <osgEarth/URI>
#include <map>

namespace osgEarth { namespace Symbology
{
    using namespace osgEarth;

    /**
     * A resource that materializes an InstanceSymbol, which is a single-point object
     * that resolves to an osg::Node. Instances are usually used for point-model
     * substitution.
     */
    class OSGEARTHSYMBOLOGY_EXPORT InstanceResource : public Resource
    {
    public:

        /** dtor */
        virtual ~InstanceResource() { }

        /**
         * Creates a new Node representing the instance.
         */
        osg::Node* createNode(const osgDB::Options* readOptions) const;

        /** Whether this instance type is 2D (orthographic screen space) */
        virtual bool is2D() const =0;

    public:
        /** Source location of the actual data to load.  */
        optional<URI>& uri() { return _uri; }
        const optional<URI>& uri() const { return _uri; }

    public: // serialization methods

        virtual Config getConfig() const;
        void mergeConfig( const Config& conf );

    protected:
        /** Constructs a new resource. */
        InstanceResource( const Config& conf =Config() );

        optional<URI>  _uri;

        virtual osg::Node* createNodeFromURI( const URI& uri, const osgDB::Options* dbOptions ) const =0;
    };


} } // namespace osgEarth::Symbology

#endif // OSGEARTHSYMBOLOGY_MARKER_RESOURCE_H
