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

#ifndef OSGEARTHFEATURES_FEATURE_SOURCE_INDEX_NODE_H
#define OSGEARTHFEATURES_FEATURE_SOURCE_INDEX_NODE_H 1

#include <osgEarthFeatures/Common>
#include <osgEarthFeatures/Feature>
#include <osgEarthFeatures/FeatureIndex>
#include <osgEarthFeatures/FeatureSource>
#include <osgEarth/ObjectIndex>
#include <osg/Config>
#include <osg/Group>
#include <osg/Drawable>
#include <map>
#include <set>

namespace osgEarth { namespace Features
{
    using namespace osgEarth;

    /**
     * Options for a feature index
     */
    class OSGEARTHFEATURES_EXPORT FeatureSourceIndexOptions
    {
    public:
        FeatureSourceIndexOptions(const Config& conf =Config());

        /** Whether indexing is enabled. */
        optional<bool>& enabled() { return _enabled; }
        const optional<bool>& enabled() const { return _enabled; }

        /** Whether to embed the actual Feature objects in the index (instead of
         *  just the FeatureID). This is useful for feature sources that cannot
         *  be queried by ID (e.g., streaming data like TFS) */
        optional<bool>& embedFeatures() { return _embedFeatures; }
        const optional<bool>& embedFeatures() const { return _embedFeatures; }

    public:
        Config getConfig() const;

    private:
        optional<bool> _enabled;
        optional<bool> _embedFeatures;
    };

    struct RefIDPair : public osg::Referenced
    {
        RefIDPair(FeatureID fid, ObjectID oid) : _fid(fid), _oid(oid) { }
        FeatureID _fid;
        ObjectID  _oid;
    };

    /**
     * Internal class that maintains a feature index for a single feature source.
     * Internal - not exported!
     */
    class OSGEARTHFEATURES_EXPORT FeatureSourceIndex : public FeatureIndex
    {
    public:
        FeatureSourceIndex(FeatureSource* source,
                           ObjectIndex*   masterIndex,
                           const FeatureSourceIndexOptions& options);

        /** FeatureSource behind this index */
        FeatureSource* getFeatureSource() { return _featureSource.get(); }

    public: // FeatureIndex

        Feature* getFeature(ObjectID oid) const;

        ObjectID getObjectID(FeatureID fid) const;

        int size() const { return _fids.size(); }

    public: // Functions called by FeatureSourceIndexNode

        RefIDPair* tagDrawable    (osg::Drawable* drawable, Feature* feature);
        RefIDPair* tagAllDrawables(osg::Node*     node,     Feature* feature);
        RefIDPair* tagNode        (osg::Node*     node,     Feature* feature);

        // removes a collection of FIDs from the index. If the refcount goes to zero,
        // remove it from the master index as well.
        template<typename InputIter>
        void removeFIDs(InputIter first, InputIter last)
        {
            Threading::ScopedMutexLock lock(_mutex);
            for(InputIter fid = first; fid != last; ++fid )
            {
                FIDMap::iterator f = _fids.find( *fid );
                if ( f != _fids.end() && f->second->referenceCount() == 1 )
                {
                    ObjectID oid = f->second->_oid;
                    _oids.erase( oid );
                    _fids.erase( f );
                    _embeddedFeatures.erase( *fid );
                    if ( _masterIndex.valid() )
                        _masterIndex->remove( oid );
                }
            }
        }
        
    public: // types

        typedef std::map<ObjectID,  FeatureID>                OIDMap;
        typedef std::map<FeatureID, osg::ref_ptr<RefIDPair> > FIDMap;
        typedef std::map<FeatureID, osg::ref_ptr<Feature> >   FeatureMap;

    protected:
        virtual ~FeatureSourceIndex();

    private:
        osg::ref_ptr<FeatureSource> _featureSource;
        osg::ref_ptr<ObjectIndex>   _masterIndex;
        FeatureSourceIndexOptions   _options;        
        bool                        _embed;
        
        mutable Threading::Mutex _mutex;

        OIDMap     _oids;
        FIDMap     _fids;
        FeatureMap _embeddedFeatures;

        void update(osg::Drawable*, std::map<ObjectID,ObjectID>&, const FIDMap&, FIDMap&);
        void update(osg::Node*,     std::map<ObjectID,ObjectID>&, const FIDMap&, FIDMap&);

        friend class FeatureSourceIndexNode;
    };


    /**
     * Node that houses a FeatureSourceIndex, so that it can un-register index
     * entries when it pages out.
     */
    class OSGEARTHFEATURES_EXPORT FeatureSourceIndexNode : public osg::Group,
                                                           public FeatureIndexBuilder
    {
    public:
        META_Node(osgEarth::Features, FeatureSourceIndexNode);
        typedef std::map<FeatureID, osg::ref_ptr<RefIDPair> > FIDMap;

        /** default ctor */
        FeatureSourceIndexNode();

        /** Construct with an index */
        FeatureSourceIndexNode(FeatureSourceIndex* index);

        /** Copy */
        FeatureSourceIndexNode(const FeatureSourceIndexNode& rhs, const osg::CopyOp& copy);

        /** The index referenced by this node. */
        void setIndex(FeatureSourceIndex* index) { _index = index; }
        FeatureSourceIndex* getIndex() { return _index.get(); }

        /** Fetches the entire set of FIDs registered with the index by this node. */
        bool getAllFIDs(std::vector<FeatureID>& output) const;

        /** Finds a FeatureSourceIndexNode in a scene graph. */
        static FeatureSourceIndexNode* get(osg::Node* graph);

    public: // FeatureIndexBuilder

        ObjectID tagDrawable    (osg::Drawable* drawable, Feature* feature);
        ObjectID tagAllDrawables(osg::Node*     node,     Feature* feature);
        ObjectID tagNode        (osg::Node*     node,     Feature* feature);

    public: // To support serialization only - do not use directly

        const FIDMap& getFIDMap() const { return _fids; }
        void setFIDMap(const FIDMap& fids);

        void reIndex(std::map<ObjectID,ObjectID>&);
        void reIndexDrawable(osg::Drawable* drawable, std::map<ObjectID,ObjectID>& oldNew, FIDMap& newFIDMap);
        void reIndexNode(osg::Node* node, std::map<ObjectID,ObjectID>& oldNew, FIDMap& newFIDMap);

        /**
         * Call this after deserializing a scene graph that may contain FeatureSourceIndexNodes.
         * It will locate them, assign the index, and reconsistute the object IDs in the index.
         */
        static void reconstitute(osg::Node* graph, FeatureSourceIndex* index);

    protected:
        
        /** dtor - unregisters any FIDs added by this node. */
        virtual ~FeatureSourceIndexNode();

    private: // serializable
        FIDMap _fids;

    private: // transient
        osg::ref_ptr<FeatureSourceIndex> _index;
    };

} } // namespace osgEarth::Features

OSGEARTH_SPECIALIZE_CONFIG(osgEarth::Features::FeatureSourceIndexOptions);

#endif // OSGEARTHFEATURES_FEATURE_SOURCE_INDEX_NODE_H
