/* -*-c++-*- */
/* osgEarth - Geospatial SDK for OpenSceneGraph
 * Copyright 2008-2011 Pelican Mapping
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

#ifndef OSGEARTH_SHADER_GENERATOR_H
#define OSGEARTH_SHADER_GENERATOR_H 1

#include <osgEarth/Common>
#include <osgEarth/StateSetCache>
#include <osgEarth/VirtualProgram>
#include <osg/NodeVisitor>
#include <osg/State>
#include <osg/Version>
#include <osg/Drawable>
#include <sstream>
#include <set>

// forward declarations
namespace osg
{
    class TexEnv;
    class TexGen;
    class TexMat;
    class Texture1D;
    class Texture2D;
    class Texture3D;
    class TextureRectangle;
    class Texture2DArray;
    class Texture2DMultisample;
    class TextureCubeMap;
    class PointSprite;
    class LightSource;
}

namespace osgSim
{
    class LightPointNode;
}

namespace osgEarth
{
    /**
     * Traverses a scene graph and generates VirtualProgram attributes to
     * render the geometry using GLSL shaders.
     *
     * You can use this class directly, but the osgEarth Registry holds
     * a system-wide implementation that the user can replace. So the best
     * way to use this class is:
     *
     *   osgEarth::Registry::shaderGenerator().run(graph);
     *
     * After generating shaders, the scene graph will have MANY additional
     * StateSets. For performance reasons you should run a StateSet sharing
     * pass afterwards. You can do this by running the StateSetCache 
     * optimization function:
     *
     *   osgEarth::StateSetCache::optimize(graph)
     *
     * Or you can pass a StateSetCache instance into the ShaderGenerator::run()
     * method and it will perform state sharing internally.
     *
     * Implementation Notes:
     *
     * ShaderGenerator WILL NOT modify existing StateSets. Instead, when 
     * a state change is necessary (to inject uniforms or virtual programs)
     * it will clone the existing StateSet and replace it with a
     * modified version. We do this to avoid altering StateSets that might
     * be shared or in the live scene graph.
     */
    class OSGEARTH_EXPORT ShaderGenerator : public osg::NodeVisitor
    {
    public:
        /** Constructs a new shader generator */
        ShaderGenerator();

        /** Copy constructor */
        ShaderGenerator(const ShaderGenerator& rhs, const osg::CopyOp& copy);

    public: // ShaderGeneratorInterface

        /**
         * Runs the shader generator on a graph.
         * @param graph Graph for which to generate shader components.
         * @param name Name to give to the top level Virtual Program.
         * @param cache StateSet cache to use for sharing state when finished.
         */
        void run(osg::Node* graph, const std::string& name, StateSetCache* cache);


    public: // statics

        /**
         * Marks a node with a hint that the shader generator should ignore it in
         * the future.
         */
        static void setIgnoreHint(osg::Object* object, bool ignore);

        /**
         * Whether an object has been marked for ignore 
         */
        static bool ignore(const osg::Object* object);

    public:

        /**
         * Whether to automatically duplicate (by cloning) subgraphs with
         * more than one parent during the traversal process. Since a
         * shader program is unique based on its traversed NodePath, graphs
         * with multi-parenting can run into problems.
         * Default is false.
         * @untested
         */
        void setDuplicateSharedSubgraphs(bool value);
        bool getDuplicateSharedSubgraphs() const { return _duplicateSharedSubgraphs; }

    public:
        /**
         * User callback that lets you selectly reject shader generation for
         * specific state attributes.
         */
        struct OSGEARTH_EXPORT AcceptCallback : public osg::Referenced
        {
            /** Return true to generate shader code for the SA; false to ignore and skip it */
            virtual bool accept(const osg::StateAttribute* sa) const =0;
            virtual ~AcceptCallback() { }
        };

        /**
         * Adds an acceptor callback that the generator will use to decide
         * whether to ignore certain state attributes.
         */
        void addAcceptCallback(AcceptCallback* cb);


    public:

        /** dtor. */
        virtual ~ShaderGenerator() { }


    public: // osg::NodeVisitor

        virtual void apply( osg::Node& );
        virtual void apply( osg::Group& );
        virtual void apply( osg::Geode& );
        virtual void apply( osg::PagedLOD& );
        virtual void apply( osg::ProxyNode& );
        virtual void apply( osg::ClipNode& );
        virtual void apply( osg::Drawable& );

    public: // types not in osg::NodeVisitor

        virtual void applyNonCoreNodeIfNecessary( osg::Node& );
        
        virtual void apply( osgSim::LightPointNode& );


    protected: // high-level entry points:

        virtual void optimizeStateSharing(osg::Node* graph, StateSetCache* cache);

        virtual void apply( osg::Drawable* );

        virtual bool processGeometry(const osg::StateSet* stateSet, osg::ref_ptr<osg::StateSet>& replacement);

        virtual bool processText(const osg::StateSet* stateSet, osg::ref_ptr<osg::StateSet>& replacement);



    protected: // overridable texture handlers:

        struct OSGEARTH_EXPORT GenBuffers
        {
            std::stringstream _modelHead, _modelBody;
            std::stringstream _viewHead, _viewBody;
            std::stringstream _fragHead, _fragBody;
            osg::StateSet*    _stateSet;
            unsigned          _version;

            bool requireVersion(unsigned glslVersion, unsigned glesVersion=100u);
            GenBuffers();
        };

        virtual bool apply(osg::Texture* tex, osg::TexGen* texgen, osg::TexEnv* texenv, osg::TexMat* texmat, osg::PointSprite* sprite, int unit, GenBuffers& buf);

        virtual bool apply(osg::TexEnv* texenv, int unit, GenBuffers& buf);

        virtual bool apply(osg::TexGen* texgen, int unit, GenBuffers& buf);

        virtual bool apply(osg::TexMat* texmat, int unit, GenBuffers& buf);

        virtual bool apply(osg::Texture1D* tex, int unit, GenBuffers& buf);

        virtual bool apply(osg::Texture2D* tex, int unit, GenBuffers& buf);

        virtual bool apply(osg::Texture3D* tex, int unit, GenBuffers& buf);

        virtual bool apply(osg::TextureRectangle* tex, int unit, GenBuffers& buf);

        virtual bool apply(osg::Texture2DArray* tex, int unit, GenBuffers& buf);

        virtual bool apply(osg::TextureCubeMap* tex, int unit, GenBuffers& buf);

        virtual bool apply(osg::PointSprite* sprite, int unit, GenBuffers& buf);
        
        virtual bool apply(osg::StateSet::AttributeList& attrs, GenBuffers& buf);

        virtual bool apply(osg::StateAttribute* attr, GenBuffers& buf);
        
        // This method will check whether setDuplicateSharedNodes has been set,
        // and if so, it will clone a node that has multiple parents such that
        // each parent has a complete separate copy of the child.
        virtual void duplicateSharedNode(osg::Node& child);

        // disables (or removes) attributes that won't work in teh current configuration
        virtual void disableUnsupportedAttributes(osg::StateSet* stateset);

    protected:

        osg::ref_ptr<osg::State> _state;

        bool _active;

        std::string _name;
        bool _duplicateSharedSubgraphs;

        typedef std::vector<osg::ref_ptr<AcceptCallback> > AcceptCallbackVector;
        AcceptCallbackVector _acceptCallbacks;

        std::set<osg::Drawable*> _drawablesVisited;

        bool accept(const osg::StateAttribute* sa) const;
    };

    
    /** Proxy interface for a ShaderGenerator - used by the registry. */
    class ShaderGeneratorProxy //header only
    {
    public:
        void run(osg::Node* graph, const std::string& name, StateSetCache* cache) {
            _instance->run(graph, name, cache);
        }
        void run(osg::Node* graph) {
            run(graph, "ShaderGenerator", 0L);
        }
        void run(osg::Node* graph, StateSetCache* cache) {
            run(graph, "ShaderGenerator", cache);
        }
        void run(osg::Node* graph, const std::string& name) {
            run(graph, name, 0L);
        }

    public:
        ShaderGeneratorProxy(const ShaderGenerator* temp)
            : _instance( new ShaderGenerator(*temp, osg::CopyOp::SHALLOW_COPY) ) { }

    private:
        osg::ref_ptr<ShaderGenerator> _instance;
    };

} // namespace osgEarth

#endif // OSGEARTH_SHADER_GENERATOR_H
