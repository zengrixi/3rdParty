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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>
*/
#ifndef OSGEARTH_ANNO_ORTHO_NODE_H
#define OSGEARTH_ANNO_ORTHO_NODE_H 1

#include <osgEarthAnnotation/AnnotationNode>
#include <osgEarth/GeoTransform>
#include <osgEarth/CullingUtils>
#include <osg/PositionAttitudeTransform>

namespace osgEarth { namespace Annotation
{	
    using namespace osgEarth;

    /**
     * Base class for an annotation node that is anchored to a GeoPoint on the map.
     * Use getGeoTransform() to control the map position, and getPositionAttitudeTransform()
     * to control the offsets in the local tangent plane at that location.
     */
    class OSGEARTHANNO_EXPORT GeoPositionNode : public AnnotationNode
    {
    public:
        META_AnnotationNode( osgEarthAnnotation, GeoPositionNode );

        /**
         * Constructs an positional node
         */
        GeoPositionNode();

        /**
         * The anchor position of this node.
         * If the annotation also has a style that contains an AltitudeSymbol,
         * any clamping properties in the symbol will take precedence over
         * the altitude mode in the GeoPoint you pass into this call.
         * @param pos New geoposition
         */
        virtual void setPosition(const GeoPoint& pos);
        const GeoPoint& getPosition() const { return _geoxform->getPosition(); }

        /** Local XYZ offset */
        virtual void setLocalOffset(const osg::Vec3d& pos) { _paxform->setPosition(pos); dirty(); }
        const osg::Vec3d& getLocalOffset() const           { return _paxform->getPosition(); }

        /** Local rotation */
        virtual void setLocalRotation(const osg::Quat& rot) { _paxform->setAttitude(rot); dirty(); }
        const osg::Quat& getLocalRotation() const           { return _paxform->getAttitude(); }

        /** Scale */
        virtual void setScale(const osg::Vec3f& scale) { _paxform->setScale(scale); dirty(); }
        const osg::Vec3d& getScale() const             { return _paxform->getScale(); }

        /**
         * Access to the GeoTransform that positions this node.
         */
        GeoTransform* getGeoTransform() const { return _geoxform; }

        /**
         * Access to the PositionAttitudeTransform for local offsets and rotation
         */
        osg::PositionAttitudeTransform* getPositionAttitudeTransform() const { return _paxform; }

        /**
         * Enables or disables ray based occlusion culling
         */
        bool getOcclusionCulling() const;
        void setOcclusionCulling( bool value );

        /**
         * Gets or sets the maximum altitude that the occlusion culling takes place.
         */
        double getOcclusionCullingMaxAltitude() const;
        void setOcclusionCullingMaxAltitude( double occlusionCullingMaxAltitude );

        /** Serialize. */
        Config getConfig() const;


    public: // AnnotationNode

        virtual void applyStyle(const Style& style);

    public: // MapNodeObserver

        virtual void setMapNode( MapNode* mapNode );

    protected:
        /** virtual dtor */
        virtual ~GeoPositionNode() { }

        /** called when someone calls one of the set functions */
        virtual void dirty() { }

        virtual void setConfig(const Config&);

        //virtual void init(const osgDB::Options*);

    private:
        GeoTransform*                   _geoxform;
        osg::PositionAttitudeTransform* _paxform;
        bool                            _occlusionCullingRequested;
        optional< double >              _occlusionCullingMaxAltitude;
        bool                            _horizonCullingRequested;

        osg::ref_ptr< HorizonCullCallback >      _horizonCuller;
        osg::ref_ptr< OcclusionCullingCallback > _occlusionCuller;

        void construct();

    protected:
        GeoPositionNode(const Config& conf, const osgDB::Options*);

        // required by META_Node, but this object is not cloneable
        GeoPositionNode(const GeoPositionNode& rhs, const osg::CopyOp& op =osg::CopyOp::DEEP_COPY_ALL) { }
    };

} } // namespace osgEarth::Annotation

#endif // OSGEARTH_ANNO_LOCALIZED_NODE_H
