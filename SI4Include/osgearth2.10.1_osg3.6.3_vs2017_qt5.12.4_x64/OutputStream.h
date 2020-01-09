/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2010 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/
// Written by Wang Rui, (C) 2010

#ifndef OSGDB_OUTPUTSTREAM
#define OSGDB_OUTPUTSTREAM

#include <osg/Version>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Quat>
#include <osg/Matrix>
#include <osg/BoundingBox>
#include <osg/BoundingSphere>
#include <osg/Array>
#include <osg/PrimitiveSet>
#include <osgDB/ReaderWriter>
#include <osgDB/StreamOperator>
#include <iostream>
#include <sstream>

namespace osgDB
{

class OutputException : public osg::Referenced
{
public:
    OutputException( const std::vector<std::string>& fields, const std::string& err ) : _error(err)
    {
        for ( unsigned int i=0; i<fields.size(); ++i )
        {
            _field += fields[i];
            _field += " ";
        }
    }

    const std::string& getField() const { return _field; }
    const std::string& getError() const { return _error; }

protected:
    std::string _field;
    std::string _error;
};

class OSGDB_EXPORT OutputStream
{
public:
    typedef std::map<const osg::Array*, unsigned int> ArrayMap;
    typedef std::map<const osg::Object*, unsigned int> ObjectMap;

    enum WriteType
    {
        WRITE_UNKNOWN = 0,
        WRITE_SCENE,
        WRITE_IMAGE,
        WRITE_OBJECT
    };

    enum WriteImageHint
    {
        WRITE_USE_IMAGE_HINT = 0,  /*!< Use image hint, write inline data or use external */
        WRITE_USE_EXTERNAL,        /*!< Use external file on disk and write only the filename */
        WRITE_INLINE_DATA,         /*!< Write Image::data() to stream */
        WRITE_INLINE_FILE,         /*!< Write the image file itself to stream */
        WRITE_EXTERNAL_FILE        /*!< Write Image::data() to disk and use it as external file */
    };

    OutputStream( const osgDB::Options* options );
    virtual ~OutputStream();

    void setFileVersion( const std::string& d, int v );
    int getFileVersion( const std::string& d=std::string() ) const;

    bool isBinary() const { return _out->isBinary(); }
    const std::string& getSchemaName() const { return _schemaName; }
    const osgDB::Options* getOptions() const { return _options.get(); }

    void setWriteImageHint( WriteImageHint hint ) { _writeImageHint = hint; }
    WriteImageHint getWriteImageHint() const { return _writeImageHint; }

    // Serialization related functions
    OutputStream& operator<<( bool b ) { _out->writeBool(b); return *this; }
    OutputStream& operator<<( char c ) { _out->writeChar(c); return *this; }
    OutputStream& operator<<( signed char c) { _out->writeChar(c); return *this; }
    OutputStream& operator<<( unsigned char c ) { _out->writeUChar(c); return *this; }
    OutputStream& operator<<( short s ) { _out->writeShort(s); return *this; }
    OutputStream& operator<<( unsigned short s ) { _out->writeUShort(s); return *this; }
    OutputStream& operator<<( int i ) { _out->writeInt(i); return *this; }
    OutputStream& operator<<( unsigned int i ) { _out->writeUInt(i); return *this; }
    OutputStream& operator<<( long l ) { _out->writeLong(l); return *this; }
    OutputStream& operator<<( unsigned long l ) { _out->writeULong(l); return *this; }
    OutputStream& operator<<( float f ) { _out->writeFloat(f); return *this; }
    OutputStream& operator<<( double d ) { _out->writeDouble(d); return *this; }
    OutputStream& operator<<( long long ll ) { _out->writeInt64(ll); return *this; }
    OutputStream& operator<<( unsigned long long ull ) { _out->writeUInt64(ull); return *this; }
    OutputStream& operator<<( const std::string& s ) { _out->writeString(s); return *this; }
    OutputStream& operator<<( const char* s ) { _out->writeString(s); return *this; }
    OutputStream& operator<<( std::ostream& (*fn)(std::ostream&) ) { _out->writeStream(fn); return *this; }
    OutputStream& operator<<( std::ios_base& (*fn)(std::ios_base&) ) { _out->writeBase(fn); return *this; }

    OutputStream& operator<<( const ObjectGLenum& value ) { _out->writeGLenum(value); return *this; }
    OutputStream& operator<<( const ObjectProperty& prop ) { _out->writeProperty(prop); return *this; }
    OutputStream& operator<<( const ObjectMark& mark ) { _out->writeMark(mark); return *this; }

    OutputStream& operator<<( const osg::Vec2b& v );
    OutputStream& operator<<( const osg::Vec3b& v );
    OutputStream& operator<<( const osg::Vec4b& v );
    OutputStream& operator<<( const osg::Vec2ub& v );
    OutputStream& operator<<( const osg::Vec3ub& v );
    OutputStream& operator<<( const osg::Vec4ub& v );
    OutputStream& operator<<( const osg::Vec2s& v );
    OutputStream& operator<<( const osg::Vec3s& v );
    OutputStream& operator<<( const osg::Vec4s& v );
    OutputStream& operator<<( const osg::Vec2us& v );
    OutputStream& operator<<( const osg::Vec3us& v );
    OutputStream& operator<<( const osg::Vec4us& v );
    OutputStream& operator<<( const osg::Vec2i& v );
    OutputStream& operator<<( const osg::Vec3i& v );
    OutputStream& operator<<( const osg::Vec4i& v );
    OutputStream& operator<<( const osg::Vec2ui& v );
    OutputStream& operator<<( const osg::Vec3ui& v );
    OutputStream& operator<<( const osg::Vec4ui& v );
    OutputStream& operator<<( const osg::Vec2f& v );
    OutputStream& operator<<( const osg::Vec3f& v );
    OutputStream& operator<<( const osg::Vec4f& v );
    OutputStream& operator<<( const osg::Vec2d& v );
    OutputStream& operator<<( const osg::Vec3d& v );
    OutputStream& operator<<( const osg::Vec4d& v );
    OutputStream& operator<<( const osg::Quat& q );
    OutputStream& operator<<( const osg::Plane& p );
    OutputStream& operator<<( const osg::Matrixf& mat );
    OutputStream& operator<<( const osg::Matrixd& mat );
    OutputStream& operator<<( const osg::BoundingBoxf& bb );
    OutputStream& operator<<( const osg::BoundingBoxd& bb );
    OutputStream& operator<<( const osg::BoundingSpheref& bb );
    OutputStream& operator<<( const osg::BoundingSphered& bb );

    OutputStream& operator<<( const osg::Image* img ) { writeImage(img); return *this; }
    OutputStream& operator<<( const osg::Array* a ) { if (_targetFileVersion >= 112) writeObject(a); else writeArray(a); return *this; }
    OutputStream& operator<<( const osg::PrimitiveSet* p ) { if (_targetFileVersion >= 112) writeObject(p); else writePrimitiveSet(p); return *this; }
    OutputStream& operator<<( const osg::Object* obj ) { writeObject(obj); return *this; }

    OutputStream& operator<<( const osg::ref_ptr<osg::Image>& ptr ) { writeImage(ptr.get()); return *this; }
    OutputStream& operator<<( const osg::ref_ptr<osg::Array>& ptr ) { if (_targetFileVersion >= 112) writeObject(ptr.get()); else writeArray(ptr.get()); return *this; }
    OutputStream& operator<<( const osg::ref_ptr<osg::PrimitiveSet>& ptr ) { if (_targetFileVersion >= 112) writeObject(ptr.get()); else writePrimitiveSet(ptr.get()); return *this; }

    template<typename T> OutputStream& operator<<( const osg::ref_ptr<T>& ptr ) { writeObject(ptr.get()); return *this; }

    // Convenient methods for writing
    void writeWrappedString( const std::string& str ) { _out->writeWrappedString(str); }
    void writeCharArray( const char* s, unsigned int size ) { _out->writeCharArray(s, size); }

    // method for converting all data structure sizes to unsigned int to ensure architecture portability.
    template<typename T>
    void writeSize(T size) { *this<<static_cast<unsigned int>(size); }

    // Global writing functions
    void writeArray( const osg::Array* a );
    void writePrimitiveSet( const osg::PrimitiveSet* p );
    void writeImage( const osg::Image* img );
    void writeObject( const osg::Object* obj );
    void writeObjectFields( const osg::Object* obj );
    void writeObjectFields( const osg::Object* obj, const std::string& compoundName );

    /// set an output iterator, used directly when not using OutputStream with a traditional file related stream.
    void setOutputIterator( OutputIterator* oi ) { _out = oi; }

    /// start writing to OutputStream treating it as a traditional file related stream, handles headers and versioning
    void start( OutputIterator* outIterator, WriteType type );

    void compress( std::ostream* ostream );

    // Schema handlers
    void writeSchema( std::ostream& fout );

    // Exception handlers
    inline void throwException( const std::string& msg );
    const OutputException* getException() const { return _exception.get(); }

    // Property & mask variables
    ObjectProperty PROPERTY;
    ObjectMark BEGIN_BRACKET;
    ObjectMark END_BRACKET;

protected:
    template<typename T>
    void writeArrayImplementation( const T*, int write_size, unsigned int numInRow=1 );

    unsigned int findOrCreateArrayID( const osg::Array* array, bool& newID );
    unsigned int findOrCreateObjectID( const osg::Object* obj, bool& newID );

    ArrayMap _arrayMap;
    ObjectMap _objectMap;

    typedef std::map<std::string, int> VersionMap;
    VersionMap _domainVersionMap;
    WriteImageHint _writeImageHint;
    bool _useSchemaData;
    bool _useRobustBinaryFormat;

    typedef std::map<std::string, std::string> SchemaMap;
    SchemaMap _inbuiltSchemaMap;
    std::vector<std::string> _fields;
    std::string _schemaName;
    std::string _compressorName;
    std::stringstream _compressSource;
    osg::ref_ptr<OutputIterator> _out;
    osg::ref_ptr<OutputException> _exception;
    osg::ref_ptr<const osgDB::Options> _options;
	
    int _targetFileVersion;
};

void OutputStream::throwException( const std::string& msg )
{
    _exception = new OutputException(_fields, msg);
}

}

#endif
