/* -*-c++-*- */
/* osgEarth - Geospatial SDK for OpenSceneGraph
* Copyright 2008-2012 Pelican Mapping
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
#ifndef OSGEARTH_OBJECT_INDEX_H
#define OSGEARTH_OBJECT_INDEX_H

#include <osgEarth/Common>
#include <osgEarth/ThreadingUtils>
#include <osgEarth/ShaderLoader>
#include <osg/Version>
#include <osg/Drawable>
#include <osg/Array>
#include <OpenThreads/Atomic>
#include <algorithm>

#define OSGEARTH_OBJECTID_EMPTY   (ObjectID)0
#define OSGEARTH_OBJECTID_TERRAIN (ObjectID)1

namespace osgEarth
{
    typedef unsigned       ObjectID;
    typedef osg::UIntArray ObjectIDArray;

    /** 
     * Virutal interface class for building an object index.
     */
    template<typename T>
    class ObjectIndexBuilder
    {
    public:
        /**
         * Inserts the object into the index, and tags the drawable with its object id.
         * Returns the ID of the object.
         */
        virtual ObjectID tagDrawable(osg::Drawable* drawable, T* object) =0;

        /**
         * Inserts the object into the index, and tags all the drawalbes under the
         * specified node with the object ID. Returns the Object ID.
         */
        virtual ObjectID tagAllDrawables(osg::Node* node, T* object) =0;

        /**
         * Inserts the object into the index, and tags the Node with a uniform containing
         * the object id. Returns the Object ID.
         */
        virtual ObjectID tagNode(osg::Node* node, T* object) =0;
    };


    /**
     * Index for tracking objects in the scene graph using vertex
     * attributes and uniforms.
     */
    class OSGEARTH_EXPORT ObjectIndex : public osg::Referenced,
                                        public ObjectIndexBuilder<osg::Referenced>
    {
    public:
        /** constructs a new index */
        ObjectIndex();

        /**
         * Adds an object to the index, and returns a new globally unique
         * ID for that object. You can then use that UID to tag scene elements
         * with one of the tag* functions. If the object already exists in the
         * index, this method will return the UID assigned to it.
         */
        ObjectID insert(osg::Referenced* object);

        /**
         * Finds the object corresponding to a unique ID and places it in "output";
         * Returns true if found, false if not.
         */
        template<typename T>
        osg::ref_ptr<T> get(ObjectID id) const {
            Threading::ScopedMutexLock lock(_mutex);
            return dynamic_cast<T*>( getImpl(id) );
        }   

        /**
         * Removes the object corresponding the the unique ID form the index.
         */
        void remove(ObjectID id);

        /**
         * Removes a collection of objects from the index all at once.
         */
        template<typename ForwardIter>
        void remove(ForwardIter i0, ForwardIter i1) {
            _mutex.lock();
            for(ForwardIter i = i0; i != i1; ++i) removeImpl( *i );
            _mutex.unlock();
        }

        /**
         * The vertex attribute binding location to use when indexing geoemtry.
         * Warning: Changing this after tagging objects will cause undefined results.
         */
        void setObjectIDAtrribLocation(int value);
        int getObjectIDAttribLocation() const { return _attribLocation; }

        /**
         * The name of the uniform used to tag nodes when tagNode() is called.
         */
        const std::string& getObjectIDUniformName() const { return _oidUniformName; }

        /**
         * The name of the ObjectID vertex attribute in the ObjectIndex shaders.
         */
        const std::string& getObjectIDAttribName() const { return _attribName; }

        /**
         * Convenience fuction to install shader components that will set the vertex-stage
         * ObjectID variable whenever detected in the geometry. This will automatically use
         * the ObjectID AttribLocation as set in this object.
         *
         * Returns false if the method fails for any reason (e.g., vp is NULL)
         */
        bool loadShaders(VirtualProgram* vp) const;

        /**
         * The ShaderPackage that includes the index initialization shaders. installShaders()
         * calls this internally to get the virtual program components.
         */
        const ShaderPackage& getShaderPackage() const { return _shaders; }

        /**
         * Returns a set of ObjectIDs used in a drawable. Returns false is none are found
         */
        bool getObjectIDs(const osg::Drawable* drawable, std::set<ObjectID>& output) const;

        /**
         * Returns an ObjectID set on a node. Returns false if none is found
         */
        bool getObjectID(osg::Node* node, ObjectID& output) const;


    public: // ObjectIndexBuilder<osg::Referenced>

        /**
         * Inserts the object into the index, and tags the drawable with its object id.
         * Returns the ID of the object.
         */
        ObjectID tagDrawable(osg::Drawable* drawable, osg::Referenced* object);

        /**
         * Inserts the object into the index, and tags all the drawalbes under the
         * specified node with the object ID. Returns the Object ID.
         */
        ObjectID tagAllDrawables(osg::Node* node, osg::Referenced* object);

        /**
         * Inserts the object into the index, and tags the Node with a uniform containing
         * the object id. Returns the Object ID.
         */
        ObjectID tagNode(osg::Node* node, osg::Referenced* object);


    public: // Raw tagging methods.

        /**
         * Tags the vertices in a drawable with the object identifier.
         */
        void tagDrawable(osg::Drawable* drawable, ObjectID id) const;

        /**
         * Tags the vertices in all Drawables until a node with the object identifier.
         */
        void tagAllDrawables(osg::Node* node, ObjectID id) const;

        /**
         * Tags a node with an object identifier. This simply puts a uniform on the
         * node and does NOT tag any actual vertices. This is only useful if you want
         * to tag an entire model and are not planning to merge geometries.
         */
        void tagNode(osg::Node* node, ObjectID id) const;

        /**
         * For each ObjectID found in a drawable, update it with a new Object ID and
         * populate an output table that maps the old ID to the new ID. Internal function
         * used for serialization support.
         */
        bool updateObjectIDs(osg::Drawable* drawable, std::map<ObjectID, ObjectID>& oldNewTable, osg::Referenced* obj);

        /**
         * On a node, replace an existing objectID with a new one and return the mapping.
         * Internal function used for serialization support.
         */
        bool updateObjectID(osg::Node* node, std::map<ObjectID, ObjectID>& oldNewTable, osg::Referenced* obj);

    protected:
        virtual ~ObjectIndex() { }
        
        typedef std::map<ObjectID, osg::observer_ptr<osg::Referenced> > IndexMap;

        IndexMap                 _index;
        int                      _attribLocation;
        std::string              _oidUniformName;
        mutable Threading::Mutex _mutex;
        OpenThreads::Atomic      _idGen;
        ShaderPackage            _shaders;
        std::string              _attribName;

        ObjectID insertImpl(osg::Referenced*);
        void removeImpl(ObjectID id);
        osg::Referenced* getImpl(ObjectID id) const;
    };

} // namespace osgEarth

#endif // OSGEARTH_OBJECT_INDEX_H
