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

#ifndef OSGDB__SERIALIZER
#define OSGDB__SERIALIZER

#include <osg/ref_ptr>
#include <osg/Notify>
#include <osg/Object>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#include <string>
#include <sstream>
#include <limits.h>

namespace osgDB
{

typedef std::vector<std::string> StringList;
extern OSGDB_EXPORT void split( const std::string& src, StringList& list, char separator=' ' );


#ifndef OBJECT_CAST
    #define OBJECT_CAST static_cast
#endif

class IntLookup
{
public:
    typedef int Value;
    typedef std::map<std::string, Value> StringToValue;
    typedef std::map<Value, std::string> ValueToString;

    IntLookup() {}
    unsigned int size() const { return static_cast<unsigned int>(_stringToValue.size()); }

    void add( const char* str, Value value )
    {
        if ( _valueToString.find(value)!=_valueToString.end() )
        {
            osg::notify(osg::INFO) << "Duplicate enum value " << value
                                   << " with old string: " << _valueToString[value]
                                   << " and new string: " << str << std::endl;
        }
        _valueToString[value] = str;
        _stringToValue[str] = value;
    }

    void add2(const char* str, const char* newStr, Value value) {
        if (_valueToString.find(value) != _valueToString.end())
        {
            osg::notify(osg::INFO) << "Duplicate enum value " << value
                << " with old string: " << _valueToString[value]
                << " and new strings: " << str  << " and " << newStr << std::endl;
        }
        _valueToString[value] = newStr;
        _stringToValue[newStr] = value;
        _stringToValue[str] = value;
    }

    Value getValue( const char* str )
    {
        StringToValue::iterator itr = _stringToValue.find(str);
        if ( itr==_stringToValue.end() )
        {
            Value value;
            std::stringstream stream;
            stream << str; stream >> value;
            _stringToValue[str] = value;
            return value;
        }
        return itr->second;
    }

    const std::string& getString( Value value )
    {
        ValueToString::iterator itr = _valueToString.find(value);
        if ( itr==_valueToString.end() )
        {
            std::string str;
            std::stringstream stream;
            stream << value; stream >> str;
            _valueToString[value] = str;
            return _valueToString[value];
        }
        return itr->second;
    }

    StringToValue& getStringToValue() { return _stringToValue; }
    const StringToValue& getStringToValue() const { return _stringToValue; }

    ValueToString& getValueToString() { return _valueToString; }
    const ValueToString& getValueToString() const { return _valueToString; }

protected:
    StringToValue _stringToValue;
    ValueToString _valueToString;
};

class UserLookupTableProxy
{
public:
    typedef void (*AddValueFunc)( IntLookup* lookup );
    UserLookupTableProxy( AddValueFunc func ) { if ( func ) (*func)(&_lookup); }

    IntLookup _lookup;
};

#define BEGIN_USER_TABLE(NAME, CLASS) \
    static void add_user_value_func_##NAME(osgDB::IntLookup*); \
    static osgDB::UserLookupTableProxy s_user_lookup_table_##NAME(&add_user_value_func_##NAME); \
    static void add_user_value_func_##NAME(osgDB::IntLookup* lookup) { typedef CLASS MyClass
#define ADD_USER_VALUE(VALUE) lookup->add(#VALUE, MyClass::VALUE)
#define END_USER_TABLE() }

#define USER_READ_FUNC(NAME, FUNCNAME) \
    static int FUNCNAME(osgDB::InputStream& is) { \
        int value; if (is.isBinary()) is >> value; \
        else { std::string str; is >> str; \
        value = (s_user_lookup_table_##NAME)._lookup.getValue(str.c_str()); } \
        return value; }

#define USER_WRITE_FUNC(NAME, FUNCNAME) \
    static void FUNCNAME(osgDB::OutputStream& os, int value) { \
        if (os.isBinary()) os << value; \
        else os << (s_user_lookup_table_##NAME)._lookup.getString(value); } \

class BaseSerializer : public osg::Referenced
{
    friend class ObjectWrapper;
public:
    enum Type
    {
        RW_UNDEFINED = 0, RW_USER, RW_OBJECT, RW_IMAGE, RW_LIST,
        RW_BOOL, RW_CHAR, RW_UCHAR, RW_SHORT, RW_USHORT, RW_INT, RW_UINT, RW_FLOAT, RW_DOUBLE,
        RW_VEC2F, RW_VEC2D, RW_VEC3F, RW_VEC3D, RW_VEC4F, RW_VEC4D, RW_QUAT, RW_PLANE,
        RW_MATRIXF, RW_MATRIXD, RW_MATRIX, RW_GLENUM, RW_STRING, RW_ENUM,
        RW_VEC2B, RW_VEC2UB, RW_VEC2S, RW_VEC2US, RW_VEC2I, RW_VEC2UI,
        RW_VEC3B, RW_VEC3UB, RW_VEC3S, RW_VEC3US, RW_VEC3I, RW_VEC3UI,
        RW_VEC4B, RW_VEC4UB, RW_VEC4S, RW_VEC4US, RW_VEC4I, RW_VEC4UI,
        RW_BOUNDINGBOXF, RW_BOUNDINGBOXD,
        RW_BOUNDINGSPHEREF, RW_BOUNDINGSPHERED,
        RW_VECTOR, RW_MAP
    };

    enum Usage
    {
        READ_WRITE_PROPERTY = 1,
        GET_PROPERTY = 2,
        SET_PROPERTY = 4,
        GET_SET_PROPERTY = GET_PROPERTY | SET_PROPERTY
    };


    BaseSerializer(int usage) : _firstVersion(0), _lastVersion(INT_MAX), _usage(usage) {}

    virtual bool set(osg::Object& /*object*/, void* /*value*/) { return false; }
    virtual bool get(const osg::Object& /*object*/, void* /*value*/) { return false; }

    virtual bool read( InputStream&, osg::Object& ) = 0;
    virtual bool write( OutputStream&, const osg::Object& ) = 0;
    virtual const std::string& getName() const = 0;

    virtual IntLookup* getIntLookup() { return 0; }

    void setUsage(int usage) { _usage = usage; }
    int getUsage() const { return _usage; }

    void setUsage(bool hasGetter, bool hasSetter)
    {
        setUsage( ((hasGetter && hasSetter) ? BaseSerializer::READ_WRITE_PROPERTY : 0) |
                  ((hasGetter) ? BaseSerializer::GET_PROPERTY : 0) |
                  ((hasSetter) ? BaseSerializer::SET_PROPERTY : 0) );
    }

    bool supportsReadWrite() const { return (_usage & READ_WRITE_PROPERTY)!=0; }
    bool supportsGetSet() const { return (_usage & GET_SET_PROPERTY)!=0; }
    bool supportsGet() const { return (_usage & GET_PROPERTY)!=0; }
    bool supportsSet() const { return (_usage & SET_PROPERTY)!=0; }

protected:
    int _firstVersion;  // Library version when the serializer is first introduced
    int _lastVersion;   // Library version when the serializer is last required.
    int _usage;         // How the Serializer can be used.
};

template<typename C>
class UserSerializer : public BaseSerializer
{
public:
    typedef bool (*Checker)( const C& );
    typedef bool (*Reader)( InputStream&, C& );
    typedef bool (*Writer)( OutputStream&, const C& );

    UserSerializer( const char* name, Checker cf, Reader rf, Writer wf )
    : BaseSerializer(READ_WRITE_PROPERTY), _name(name), _checker(cf), _reader(rf), _writer(wf) {}

    virtual bool read( InputStream& is, osg::Object& obj )
    {
        C& object = OBJECT_CAST<C&>(obj);
        if ( is.isBinary() )
        {
            bool ok = false; is >> ok;
            if ( !ok ) return true;
        }
        else
        {
            if ( !is.matchString(_name) )
                return true;
        }
        return (*_reader)(is, object);
    }

    virtual bool write( OutputStream& os, const osg::Object& obj )
    {
        const C& object = OBJECT_CAST<const C&>(obj);
        bool ok = (*_checker)(object);
        if ( os.isBinary() )
        {
            os << ok;
            if ( !ok ) return true;
        }
        else
        {
            if ( !ok ) return true;
            os << os.PROPERTY(_name.c_str());
        }
        return (*_writer)(os, object);
    }

    virtual const std::string& getName() const { return _name; }

protected:
    std::string _name;
    Checker _checker;

public:
    Reader _reader;
    Writer _writer;
};

template<typename P>
class TemplateSerializer : public BaseSerializer
{
public:

    TemplateSerializer( const char* name, P def)
        : BaseSerializer(READ_WRITE_PROPERTY), _name(name), _defaultValue(def) {}

    virtual bool read( InputStream& is, osg::Object& obj ) = 0;
    virtual bool write( OutputStream& os, const osg::Object& obj ) = 0;
    virtual const std::string& getName() const { return _name; }

protected:
    std::string _name;
    P _defaultValue;
};

template<typename C, typename P>
class PropByValSerializer : public TemplateSerializer<P>
{
public:
    typedef TemplateSerializer<P> ParentType;
    typedef P (C::*Getter)() const;
    typedef void (C::*Setter)( P );

    PropByValSerializer( const char* name, P def, Getter gf, Setter sf, bool useHex=false ) : ParentType(name, def), _getter(gf), _setter(sf), _useHex(useHex)
    {
        ParentType::setUsage( _getter!=0, _setter!=0);
    }

    virtual bool read( InputStream& is, osg::Object& obj )
    {
        C& object = OBJECT_CAST<C&>(obj);
        P value;
        if ( is.isBinary() )
        {
            is >> value;
            (object.*_setter)( value );
        }
        else if ( is.matchString(ParentType::_name) )
        {
            if ( _useHex ) is >> std::hex;
            is >> value;
            if ( _useHex ) is >> std::dec;
            (object.*_setter)( value );
        }
        return true;
    }

    virtual bool write( OutputStream& os, const osg::Object& obj )
    {
        const C& object = OBJECT_CAST<const C&>(obj);
        P value = (object.*_getter)();
        if ( os.isBinary() )
        {
            os << value;
        }
        else if ( ParentType::_defaultValue!=value )
        {
            os << os.PROPERTY((ParentType::_name).c_str());
            if ( _useHex ) { os << std::hex << std::showbase; }
            os << value;
            if ( _useHex ) os << std::dec << std::noshowbase;
            os << std::endl;
        }
        return true;
    }

public:
    Getter _getter;
    Setter _setter;

protected:
    bool _useHex;
};

template<typename C, typename P>
class PropByRefSerializer : public TemplateSerializer<P>
{
public:
    typedef TemplateSerializer<P> ParentType;
    typedef const P& CP;
    typedef CP (C::*Getter)() const;
    typedef void (C::*Setter)( CP );

    PropByRefSerializer( const char* name, CP def, Getter gf, Setter sf ) : ParentType(name, def), _getter(gf), _setter(sf)
    {
        ParentType::setUsage( _getter!=0, _setter!=0);
    }

    virtual bool read( InputStream& is, osg::Object& obj )
    {
        C& object = OBJECT_CAST<C&>(obj);
        P value;
        if ( is.isBinary() )
        {
            is >> value;
            (object.*_setter)( value );
        }
        else if ( is.matchString(ParentType::_name) )
        {
            is >> value;
            (object.*_setter)( value );
        }
        return true;
    }

    virtual bool write( OutputStream& os, const osg::Object& obj )
    {
        const C& object = OBJECT_CAST<const C&>(obj);
        CP value = (object.*_getter)();
        if ( os.isBinary() )
        {
            os << value;
        }
        else if ( ParentType::_defaultValue!=value )
        {
            os << os.PROPERTY((ParentType::_name).c_str()) << value << std::endl;
        }
        return true;
    }

public:
    Getter _getter;
    Setter _setter;
};

template<typename C>
class MatrixSerializer : public TemplateSerializer<osg::Matrix>
{
public:
    typedef TemplateSerializer<osg::Matrix> ParentType;
    typedef const osg::Matrix& (C::*Getter)() const;
    typedef void (C::*Setter)( const osg::Matrix& );

    MatrixSerializer( const char* name, const osg::Matrix& def, Getter gf, Setter sf ) : ParentType(name, def), _getter(gf), _setter(sf)
    {
        ParentType::setUsage( _getter!=0, _setter!=0);
    }

    virtual bool read( InputStream& is, osg::Object& obj )
    {
        C& object = OBJECT_CAST<C&>(obj);
        osg::Matrix value;
        if ( is.isBinary() )
        {
            readMatrixImplementation( is, value );
            (object.*_setter)( value );
        }
        else if ( is.matchString(ParentType::_name) )
        {
            readMatrixImplementation( is, value );
            (object.*_setter)( value );
        }
        return true;
    }

    virtual bool write( OutputStream& os, const osg::Object& obj )
    {

        const C& object = OBJECT_CAST<const C&>(obj);
        const osg::Matrix& value = (object.*_getter)();
        if ( os.isBinary() )
        {
            os << value;
        }
        else if ( ParentType::_defaultValue!=value )
        {
            os << os.PROPERTY((ParentType::_name).c_str()) << value << std::endl;
        }
        return true;
    }

protected:
    void readMatrixImplementation( InputStream& is, osg::Matrix& matrix )
    {
#if 1
        is >> matrix;
#else
        if ( is.getUseFloatMatrix() )
        {
            osg::Matrixf realValue; is >> realValue;
            matrix = realValue;
        }
        else
        {
            osg::Matrixd realValue; is >> realValue;
            matrix = realValue;
        }
#endif
    }

public:
    Getter _getter;
    Setter _setter;
};

template<typename C, typename P>
class GLenumSerializer : public TemplateSerializer<P>
{
public:
    typedef TemplateSerializer<P> ParentType;
    typedef P (C::*Getter)() const;
    typedef void (C::*Setter)( P );

    GLenumSerializer( const char* name, P def, Getter gf, Setter sf ) : ParentType(name, def), _getter(gf), _setter(sf)
    {
        ParentType::setUsage( _getter!=0, _setter!=0);
    }

    virtual bool read( InputStream& is, osg::Object& obj )
    {
        C& object = OBJECT_CAST<C&>(obj);
        if ( is.isBinary() )
        {
            GLenum value; is >> value;
            (object.*_setter)( static_cast<P>(value) );
        }
        else if ( is.matchString(ParentType::_name) )
        {
            DEF_GLENUM(value); is >> value;
            (object.*_setter)( static_cast<P>(value.get()) );
        }
        return true;
    }

    virtual bool write( OutputStream& os, const osg::Object& obj )
    {
        const C& object = OBJECT_CAST<const C&>(obj);
        const P value = (object.*_getter)();
        if ( os.isBinary() )
        {
            os << static_cast<GLenum>(value);
        }
        else if ( ParentType::_defaultValue!=value )
        {
            os << os.PROPERTY((ParentType::_name).c_str()) << GLENUM(value) << std::endl;
        }
        return true;
    }

public:
    Getter _getter;
    Setter _setter;
};

template<typename C>
class StringSerializer : public TemplateSerializer<std::string>
{
public:
    typedef TemplateSerializer<std::string> ParentType;
    typedef const std::string& (C::*Getter)() const;
    typedef void (C::*Setter)( const std::string& );

    StringSerializer( const char* name, const std::string& def, Getter gf, Setter sf ) : ParentType(name, def), _getter(gf), _setter(sf)
    {
        ParentType::setUsage( _getter!=0, _setter!=0);
    }

    virtual bool read( InputStream& is, osg::Object& obj )
    {
        C& object = OBJECT_CAST<C&>(obj);
        std::string value;
        if ( is.isBinary() )
        {
            is >> value;
            (object.*_setter)( value );
        }
        else if ( is.matchString(ParentType::_name) )
        {
            is.readWrappedString( value );
            if ( !value.empty() && (_setter!=0) )
                (object.*_setter)( value );
        }
        return true;
    }

    virtual bool write( OutputStream& os, const osg::Object& obj )
    {
        const C& object = OBJECT_CAST<const C&>(obj);
        const std::string& value = (object.*_getter)();
        if ( os.isBinary() )
        {
            os << value;
        }
        else if ( ParentType::_defaultValue!=value )
        {
            os << os.PROPERTY((ParentType::_name).c_str());
            os.writeWrappedString( value );
            os << std::endl;
        }
        return true;
    }

public:
    Getter _getter;
    Setter _setter;
};

template<typename C, typename P>
class ObjectSerializer : public BaseSerializer
{
public:
    typedef const P* (C::*Getter)() const;
    typedef void (C::*Setter)( P* );

    ObjectSerializer( const char* name, P* def, Getter gf, Setter sf ) :
        BaseSerializer(READ_WRITE_PROPERTY),
        _name(name), _defaultValue(def), _getter(gf), _setter(sf)
    {
        setUsage( _getter!=0, _setter!=0);
    }

    virtual bool set(osg::Object& obj, void* value) { C& object = OBJECT_CAST<C&>(obj); (object.*_setter)( dynamic_cast<P*>(*(reinterpret_cast<osg::Object**>(value))) ); return true; }
    virtual bool get(const osg::Object& obj, void* value) { const C& object = OBJECT_CAST<const C&>(obj);*(reinterpret_cast<const osg::Object**>(value )) = dynamic_cast<const osg::Object*>((object.*_getter)()); return true; }

    virtual const std::string& getName() const { return _name; }

    virtual bool read( InputStream& is, osg::Object& obj )
    {
        C& object = OBJECT_CAST<C&>(obj);
        bool hasObject = false;
        if ( is.isBinary() )
        {
            is >> hasObject;
            if ( hasObject )
            {
                osg::ref_ptr<P> value = is.readObjectOfType<P>();
                (object.*_setter)( value.get() );
            }
        }
        else if ( is.matchString(_name) )
        {
            is >> hasObject;
            if ( hasObject )
            {
                is >> is.BEGIN_BRACKET;
                osg::ref_ptr<P> value = is.readObjectOfType<P>();
                (object.*_setter)( value.get() );
                is >> is.END_BRACKET;
            }
        }
        return true;
    }

    virtual bool write( OutputStream& os, const osg::Object& obj )
    {
        const C& object = OBJECT_CAST<const C&>(obj);
        const P* value = (object.*_getter)();
        bool hasObject = (value!=NULL);
        if ( os.isBinary() )
        {
            os << hasObject;
            if (hasObject)
            {
                os.writeObject( value );
            }
        }
        else if ( _defaultValue!=value )
        {
            os << os.PROPERTY(_name.c_str()) << hasObject;
            if ( hasObject )
            {
                os << os.BEGIN_BRACKET << std::endl;
                os.writeObject( value );
                os << os.END_BRACKET;
            }
            os << std::endl;
        }
        return true;
    }

public:
    std::string     _name;
    osg::ref_ptr<P> _defaultValue;
    Getter          _getter;
    Setter          _setter;
};

template<typename C, typename P>
class ImageSerializer : public TemplateSerializer<P*>
{
public:
    typedef TemplateSerializer<P*> ParentType;
    typedef const P* (C::*Getter)() const;
    typedef void (C::*Setter)( P* );

    ImageSerializer( const char* name, P* def, Getter gf, Setter sf ):
        ParentType(name, def), _getter(gf), _setter(sf)
    {
        ParentType::setUsage( _getter!=0, _setter!=0);
    }

    virtual bool set(osg::Object& obj, void* value) { C& object = OBJECT_CAST<C&>(obj); (object.*_setter)( *(reinterpret_cast<P**>(value)) ); return true; }
    virtual bool get(const osg::Object& obj, void* value) { const C& object = OBJECT_CAST<const C&>(obj);*(reinterpret_cast<const P**>(value )) = (object.*_getter)(); return true; }

    virtual bool read( InputStream& is, osg::Object& obj )
    {
        C& object = OBJECT_CAST<C&>(obj);
        bool hasObject = false;
        if ( is.isBinary() )
        {
            is >> hasObject;
            if ( hasObject )
            {
                osg::ref_ptr<osg::Image> image = is.readImage();
                P* value = dynamic_cast<P*>( image.get() );
                (object.*_setter)( value );
            }
        }
        else if ( is.matchString(ParentType::_name) )
        {
            is >> hasObject;
            if ( hasObject )
            {
                is >> is.BEGIN_BRACKET;
                osg::ref_ptr<osg::Image> image = is.readImage();
                P* value = dynamic_cast<P*>( image.get() );
                (object.*_setter)( value );
                is >> is.END_BRACKET;
            }
        }
        return true;
    }

    virtual bool write( OutputStream& os, const osg::Object& obj )
    {
        const C& object = OBJECT_CAST<const C&>(obj);
        const P* value = (object.*_getter)();
        bool hasObject = (value!=NULL);
        if ( os.isBinary() )
        {
            os << hasObject;
            os.writeImage( value );
        }
        else if ( ParentType::_defaultValue!=value )
        {
            os << os.PROPERTY((ParentType::_name).c_str()) << hasObject;
            if ( hasObject )
            {
                os << os.BEGIN_BRACKET << std::endl;
                os.writeImage( value );
                os << os.END_BRACKET;
            }
            os << std::endl;
        }
        return true;
    }

public:
    Getter _getter;
    Setter _setter;
};

template<typename C, typename P, typename B>
class EnumSerializer : public TemplateSerializer<P>
{
public:
    typedef TemplateSerializer<P> ParentType;
    typedef P (C::*Getter)() const;
    typedef B (C::*Setter)( P );

    EnumSerializer( const char* name, P def, Getter gf, Setter sf ) : ParentType(name, def), _getter(gf), _setter(sf)
    {
        ParentType::setUsage( _getter!=0, _setter!=0);
    }

    virtual IntLookup* getIntLookup() { return &_lookup; }

    void add( const char* str, P value )
    { _lookup.add(str, static_cast<IntLookup::Value>(value)); }

    P getValue( const char* str )
    { return static_cast<P>(_lookup.getValue(str)); }

    const std::string& getString( P value )
    { return _lookup.getString(static_cast<IntLookup::Value>(value)); }

    virtual bool read( InputStream& is, osg::Object& obj )
    {
        C& object = OBJECT_CAST<C&>(obj);
        IntLookup::Value value;
        if ( is.isBinary() )
        {
            is >> value;
            (object.*_setter)( static_cast<P>(value) );
        }
        else if ( is.matchString(ParentType::_name) )
        {
            std::string str; is >> str;
            (object.*_setter)( getValue(str.c_str()) );
        }
        return true;
    }

    virtual bool write( osgDB::OutputStream& os, const osg::Object& obj )
    {
        const C& object = OBJECT_CAST<const C&>(obj);
        const P value = (object.*_getter)();
        if ( os.isBinary() )
        {
            os << (IntLookup::Value)value;
        }
        else if ( ParentType::_defaultValue!=value )
        {
            os << os.PROPERTY((ParentType::_name).c_str()) << getString(value) << std::endl;
        }
        return true;
    }

public:
    Getter _getter;
    Setter _setter;

protected:
    IntLookup _lookup;
};


template<typename C, typename P>
class ListSerializer : public BaseSerializer
{
public:
    typedef typename P::value_type ValueType;
    typedef typename P::const_iterator ConstIterator;
    typedef const P& (C::*Getter)() const;
    typedef void (C::*Setter)( const P& );

    ListSerializer( const char* name, Getter gf, Setter sf ):
        BaseSerializer(READ_WRITE_PROPERTY),
        _name(name), _getter(gf), _setter(sf) {}

    virtual const std::string& getName() const { return _name; }

    virtual bool read( InputStream& is, osg::Object& obj )
    {
        C& object = OBJECT_CAST<C&>(obj);
        unsigned int size = 0;
        P list;
        if ( is.isBinary() )
        {
            is >> size;
            for ( unsigned int i=0; i<size; ++i )
            {
                ValueType value;
                is >> value;
                list.push_back( value );
            }
            if ( size>0 ) (object.*_setter)( list );
        }
        else if ( is.matchString(_name) )
        {
            is >> size;
            if ( size>0 ) is >> is.BEGIN_BRACKET;
            for ( unsigned int i=0; i<size; ++i )
            {
                ValueType value;
                is >> value;
                list.push_back( value );
            }
            if ( size>0 )
            {
                is >> is.END_BRACKET;
                (object.*_setter)( list );
            }
        }
        return true;
    }

    virtual bool write( OutputStream& os, const osg::Object& obj )
    {
        const C& object = OBJECT_CAST<const C&>(obj);
        const P& list = (object.*_getter)();
        unsigned int size = (unsigned int)list.size();
        if ( os.isBinary() )
        {
            os << size;
            for ( ConstIterator itr=list.begin();
                  itr!=list.end(); ++itr )
            {
                os << (*itr);
            }
        }
        else if ( size>0 )
        {
            os << os.PROPERTY((_name).c_str()) << size << os.BEGIN_BRACKET << std::endl;
            for ( ConstIterator itr=list.begin();
                  itr!=list.end(); ++itr )
            {
                os << (*itr);
            }
            os << std::endl;
            os << os.END_BRACKET << std::endl;
        }
        return true;
    }

public:
    std::string _name;
    Getter _getter;
    Setter _setter;
};

class VectorBaseSerializer : public BaseSerializer
{
public:

    VectorBaseSerializer(BaseSerializer::Type elementType, unsigned int elementSize):
        BaseSerializer(READ_WRITE_PROPERTY|GET_SET_PROPERTY),
        _elementType(elementType),_elementSize(elementSize) {}

    Type getElementType() const { return _elementType; }
    unsigned int getElementSize() const { return _elementSize; }

    virtual unsigned int size(const osg::Object& /*obj*/) const { return 0; }
    virtual void resize(osg::Object& /*obj*/, unsigned int /*numElements*/) const {}
    virtual void reserve(osg::Object& /*obj*/, unsigned int /*numElements*/) const {}
    virtual void clear(osg::Object& /*obj*/) const {}
    virtual void addElement(osg::Object& /*obj*/, void* /*ptr*/) const {}
    virtual void insertElement(osg::Object& /*obj*/, unsigned int /*index*/, void* /*ptr*/) const {}
    virtual void setElement(osg::Object& /*obj*/, unsigned int /*index*/, void* /*ptr*/) const {}
    virtual void* getElement(osg::Object& /*obj*/, unsigned int /*index*/) const { return 0; }
    virtual const void* getElement(const osg::Object& /*obj*/, unsigned int /*index*/) const { return 0; }

protected:
    Type         _elementType;
    unsigned int _elementSize;
};


template<typename C, typename P>
class VectorSerializer : public VectorBaseSerializer
{
public:
    typedef typename P::value_type ValueType;
    typedef typename P::const_iterator ConstIterator;
    typedef P& (C::*Getter)();
    typedef const P& (C::*ConstGetter)() const;
    typedef void (C::*Setter)( const P& );

    VectorSerializer( const char* name, ConstGetter cgf, Getter gf, Setter sf, BaseSerializer::Type elementType, unsigned int numElementsOnRow):
        VectorBaseSerializer(elementType, sizeof(ValueType)),
        _name(name),
        _constgetter(cgf), _getter(gf), _setter(sf),
        _numElementsOnRow(numElementsOnRow) {}

    virtual const std::string& getName() const { return _name; }

    virtual unsigned int size(const osg::Object& obj) const
    {
        const C& object = OBJECT_CAST<const C&>(obj);
        const P& list = (object.*_constgetter)();
        return static_cast<unsigned int>(list.size());
    }
    virtual void resize(osg::Object& obj, unsigned int numElements) const
    {
        C& object = OBJECT_CAST<C&>(obj);
        P& list = (object.*_getter)();
        list.resize(numElements);
    }
    virtual void reserve(osg::Object& obj, unsigned int numElements) const
    {
        C& object = OBJECT_CAST<C&>(obj);
        P& list = (object.*_getter)();
        list.reserve(numElements);
    }
    virtual void clear(osg::Object& obj) const
    {
        C& object = OBJECT_CAST<C&>(obj);
        P& list = (object.*_getter)();
        list.clear();
    }
    virtual void addElement(osg::Object& obj, void* ptr) const
    {
        C& object = OBJECT_CAST<C&>(obj);
        P& list = (object.*_getter)();
        list.push_back(*reinterpret_cast<ValueType*>(ptr));
    }
    virtual void insertElement(osg::Object& obj, unsigned int index, void* ptr) const
    {
        C& object = OBJECT_CAST<C&>(obj);
        P& list = (object.*_getter)();
        if (index>=list.size()) list.resize(index+1);
        list.insert(list.begin()+index, *reinterpret_cast<ValueType*>(ptr));
    }
    virtual void setElement(osg::Object& obj, unsigned int index, void* ptr) const
    {
        C& object = OBJECT_CAST<C&>(obj);
        P& list = (object.*_getter)();
        if (index>=list.size()) list.resize(index+1);
        list[index] = *reinterpret_cast<ValueType*>(ptr);
    }
    virtual void* getElement(osg::Object& obj, unsigned int index) const
    {
        C& object = OBJECT_CAST<C&>(obj);
        P& list = (object.*_getter)();
        if (index>=list.size()) return 0;
        else return &list[index];
    }
    virtual const void* getElement(const osg::Object& obj, unsigned int index) const
    {
        const C& object = OBJECT_CAST<const C&>(obj);
        const P& list = (object.*_constgetter)();
        if (index>=list.size()) return 0;
        else return &list[index];
    }

    virtual bool read( InputStream& is, osg::Object& obj )
    {
        C& object = OBJECT_CAST<C&>(obj);
        unsigned int size = 0;
        P list;
        if ( is.isBinary() )
        {
            is >> size;
            list.reserve(size);
            for ( unsigned int i=0; i<size; ++i )
            {
                ValueType value;
                is >> value;
                list.push_back( value );
            }
            if ( size>0 ) (object.*_setter)( list );
        }
        else if ( is.matchString(_name) )
        {
            is >> size;
            list.reserve(size);
            if ( size>0 ) is >> is.BEGIN_BRACKET;
            for ( unsigned int i=0; i<size; ++i )
            {
                ValueType value;
                is >> value;
                list.push_back( value );
            }
            if ( size>0 )
            {
                is >> is.END_BRACKET;
                (object.*_setter)( list );
            }
        }
        return true;
    }

    virtual bool write( OutputStream& os, const osg::Object& obj )
    {
        const C& object = OBJECT_CAST<const C&>(obj);
        const P& list = (object.*_constgetter)();
        unsigned int size = (unsigned int)list.size();
        if ( os.isBinary() )
        {
            os << size;
            for ( ConstIterator itr=list.begin();
                  itr!=list.end(); ++itr )
            {
                os << (*itr);
            }
        }
        else if ( size>0 )
        {
            os << os.PROPERTY((_name).c_str()) << size << os.BEGIN_BRACKET << std::endl;
            if (_numElementsOnRow==0)
            {
                for ( ConstIterator itr=list.begin(); itr!=list.end(); ++itr )
                {
                    os << (*itr);
                }
            }
            else if (_numElementsOnRow==1)
            {
                for ( ConstIterator itr=list.begin(); itr!=list.end(); ++itr )
                {
                    os << (*itr); os << std::endl;
                }
            }
            else
            {
                unsigned int i = _numElementsOnRow-1;
                for (ConstIterator itr=list.begin(); itr!=list.end(); ++itr)
                {
                    os << (*itr);
                    if (i==0) { os << std::endl; i = _numElementsOnRow-1; }
                    else --i;
                }
                if (i!=_numElementsOnRow) os << std::endl;
            }
            os << os.END_BRACKET << std::endl;
        }
        return true;
    }

public:
    std::string         _name;
    ConstGetter         _constgetter;
    Getter              _getter;
    Setter              _setter;
    unsigned int        _numElementsOnRow;
};


template<typename C>
class IsAVectorSerializer : public VectorBaseSerializer
{
public:
    typedef typename C::value_type ValueType;
    typedef typename C::const_iterator ConstIterator;

    IsAVectorSerializer( const char* name, BaseSerializer::Type elementType, unsigned int numElementsOnRow) :
        VectorBaseSerializer(elementType, sizeof(ValueType)),
        _name(name),
        _numElementsOnRow(numElementsOnRow) {}

    virtual const std::string& getName() const { return _name; }

    virtual unsigned int size(const osg::Object& obj) const
    {
        const C& list = OBJECT_CAST<const C&>(obj);
        return static_cast<unsigned int>(list.size());
    }
    virtual void resize(osg::Object& obj, unsigned int numElements) const
    {
        C& list = OBJECT_CAST<C&>(obj);
        list.resize(numElements);
    }
    virtual void reserve(osg::Object& obj, unsigned int numElements) const
    {
        C& list = OBJECT_CAST<C&>(obj);
        list.reserve(numElements);
    }
    virtual void clear(osg::Object& obj) const
    {
        C& list = OBJECT_CAST<C&>(obj);
        list.clear();
    }
    virtual void addElement(osg::Object& obj, void* ptr) const
    {
        C& list = OBJECT_CAST<C&>(obj);
        list.push_back(*reinterpret_cast<ValueType*>(ptr));
    }
    virtual void insertElement(osg::Object& obj, unsigned int index, void* ptr) const
    {
        C& list = OBJECT_CAST<C&>(obj);
        if (index>=list.size()) list.resize(index+1);
        list.insert(list.begin()+index, *reinterpret_cast<ValueType*>(ptr));
    }
    virtual void setElement(osg::Object& obj, unsigned int index, void* ptr) const
    {
        C& list = OBJECT_CAST<C&>(obj);
        if (index>=list.size()) list.resize(index+1);
        list[index] = *reinterpret_cast<ValueType*>(ptr);
    }
    virtual void* getElement(osg::Object& obj, unsigned int index) const
    {
        C& list = OBJECT_CAST<C&>(obj);
        if (index>=list.size()) return 0;
        else return &list[index];
    }

    virtual const void* getElement(const osg::Object& obj, unsigned int index) const
    {
        const C& list = OBJECT_CAST<const C&>(obj);
        if (index>=list.size()) return 0;
        else return &list[index];
    }

    virtual bool read( InputStream& is, osg::Object& obj )
    {
        C& list = OBJECT_CAST<C&>(obj);
        unsigned int size = 0;
        if ( is.isBinary() )
        {
            is >> size;
            list.reserve(size);
            for ( unsigned int i=0; i<size; ++i )
            {
                ValueType value;
                is >> value;
                list.push_back( value );
            }
        }
        else if ( is.matchString(_name) )
        {
            is >> size;
            list.reserve(size);
            if ( size>0 ) is >> is.BEGIN_BRACKET;
            for ( unsigned int i=0; i<size; ++i )
            {
                ValueType value;
                is >> value;
                list.push_back( value );
            }
            if ( size>0 )
            {
                is >> is.END_BRACKET;
            }
        }
        return true;
    }

    virtual bool write( OutputStream& os, const osg::Object& obj )
    {
        const C& list = OBJECT_CAST<const C&>(obj);
        unsigned int size = (unsigned int)list.size();
        if ( os.isBinary() )
        {
            os << size;
            for ( ConstIterator itr=list.begin();
                  itr!=list.end(); ++itr )
            {
                os << (*itr);
            }
        }
        else if ( size>0 )
        {
            os << os.PROPERTY((_name).c_str()) << size << os.BEGIN_BRACKET << std::endl;
            if (_numElementsOnRow==0)
            {
                for ( ConstIterator itr=list.begin(); itr!=list.end(); ++itr )
                {
                    os << (*itr);
                }
            }
            else if (_numElementsOnRow==1)
            {
                for ( ConstIterator itr=list.begin(); itr!=list.end(); ++itr )
                {
                    os << (*itr); os << std::endl;
                }
            }
            else
            {
                unsigned int i = _numElementsOnRow-1;
                for (ConstIterator itr=list.begin(); itr!=list.end(); ++itr)
                {
                    os << (*itr);
                    if (i==0) { os << std::endl; i = _numElementsOnRow-1; }
                    else --i;
                }
                if (i!=_numElementsOnRow) os << std::endl;
            }
            os << os.END_BRACKET << std::endl;
        }
        return true;
    }

public:
    std::string         _name;
    unsigned int        _numElementsOnRow;
};

class MapIteratorObject : public osg::Object
{
public:
    MapIteratorObject():
        _keyType(BaseSerializer::RW_UNDEFINED), _keySize(0),
        _elementType(BaseSerializer::RW_UNDEFINED),_elementSize(0) {}

    MapIteratorObject(BaseSerializer::Type keyType, unsigned int keySize, BaseSerializer::Type elementType, unsigned int elementSize):
        _keyType(keyType), _keySize(keySize),
        _elementType(elementType),_elementSize(elementSize) {}

    MapIteratorObject(const MapIteratorObject& rhs, const osg::CopyOp copyop=osg::CopyOp::SHALLOW_COPY):
        osg::Object(rhs, copyop),
        _keyType(rhs._keyType), _keySize(rhs._keySize),
        _elementType(rhs._elementType),_elementSize(rhs._elementSize) {}

    META_Object(osgDB, MapIteratorObject);

    BaseSerializer::Type getKeyType() const { return _keyType; }
    unsigned int getKeySize() const { return _keySize; }

    BaseSerializer::Type getElementType() const { return _elementType; }
    unsigned int getElementSize() const { return _elementSize; }

    virtual bool advance() { return false; }
    virtual bool valid() const { return false; }
    virtual const void* getKey() const { return 0; }
    virtual void* getElement() const { return 0; }
    virtual void setElement(void* /*ptr*/) const {}

protected:
    BaseSerializer::Type _keyType;
    unsigned int _keySize;

    BaseSerializer::Type _elementType;
    unsigned int _elementSize;
};

class MapBaseSerializer : public BaseSerializer
{
public:

    MapBaseSerializer(BaseSerializer::Type keyType, unsigned int keySize, BaseSerializer::Type elementType, unsigned int elementSize):
        BaseSerializer(READ_WRITE_PROPERTY|GET_SET_PROPERTY),
        _keyType(keyType), _keySize(keySize),
        _elementType(elementType),_elementSize(elementSize) {}

    Type getKeyType() const { return _keyType; }
    unsigned int getKeySize() const { return _keySize; }

    Type getElementType() const { return _elementType; }
    unsigned int getElementSize() const { return _elementSize; }

    virtual void clear(osg::Object& /*obj*/) const {}
    virtual void setElement(osg::Object& /*obj*/, void* /*ptrKey*/, void* /*ptrValue*/) const {}
    virtual void* getElement(osg::Object& /*obj*/, void* /*ptrKey*/) const { return 0; }
    virtual const void* getElement(const osg::Object& /*obj*/, void* /*ptrKey*/) const { return 0; }
    virtual unsigned int size(const osg::Object& /*obj*/) const { return 0; }

    virtual MapIteratorObject* createIterator(osg::Object& /*obj*/) const { return 0; }
    virtual MapIteratorObject* createReverseIterator(osg::Object& /*obj*/) const { return 0; }

protected:
    Type _keyType;
    unsigned int _keySize;

    Type _elementType;
    unsigned int _elementSize;
};


template<typename C, typename P>
class MapSerializer : public MapBaseSerializer
{
public:
    typedef typename P::value_type ValueType;
    typedef typename P::key_type  KeyType;
    typedef typename P::mapped_type  ElementType;
    typedef typename P::iterator Iterator;
    typedef typename P::reverse_iterator ReverseIterator;
    typedef typename P::const_iterator ConstIterator;
    typedef P& (C::*Getter)();
    typedef const P& (C::*ConstGetter)() const;
    typedef void (C::*Setter)( const P& );

    MapSerializer( const char* name, ConstGetter cgf, Getter gf, Setter sf, BaseSerializer::Type keyType, BaseSerializer::Type elementType):
        MapBaseSerializer(keyType, sizeof(KeyType), elementType, sizeof(ElementType)),
        _name(name),
        _constgetter(cgf), _getter(gf), _setter(sf) {}

    virtual const std::string& getName() const { return _name; }

    virtual void clear(osg::Object& obj) const
    {
        C& object = OBJECT_CAST<C&>(obj);
        P& map = (object.*_getter)();
        map.clear();
    }

    virtual void setElement(osg::Object& obj, void* ptrKey, void* ptrValue) const
    {
        C& object = OBJECT_CAST<C&>(obj);
        P& map = (object.*_getter)();
        map[*reinterpret_cast<KeyType*>(ptrKey)] = *reinterpret_cast<ElementType*>(ptrValue);
    }

    virtual void* getElement(osg::Object& obj, void* ptrKey) const
    {
        C& object = OBJECT_CAST<C&>(obj);
        P& map = (object.*_getter)();
        return &(map[*reinterpret_cast<KeyType*>(ptrKey)]);
    }

    virtual const void* getElement(const osg::Object& obj, void* ptrKey) const
    {
        const C& object = OBJECT_CAST<const C&>(obj);
        const P& map = (object.*_constgetter)();
        ConstIterator itr = map.find(*reinterpret_cast<KeyType*>(ptrKey));
        if (itr==map.end()) return 0;
        else return &(itr->second);
    }

    virtual unsigned int size(const osg::Object& obj) const
    {
        const C& object = OBJECT_CAST<const C&>(obj);
        const P& map = (object.*_constgetter)();
        return map.size();
    }

    struct MapIterator : public MapIteratorObject
    {
        MapIterator(BaseSerializer::Type keyType, unsigned int keySize, BaseSerializer::Type elementType, unsigned int elementSize,
                    Iterator itr, Iterator endItr):
            MapIteratorObject(keyType, keySize, elementType, elementSize),
            _itr(itr),_endItr(endItr) {}

        Iterator _itr;
        Iterator _endItr;

        virtual bool advance() { if (valid()) ++_itr; return valid(); }
        virtual bool valid() const { return _itr!=_endItr; }
        virtual const void* getKey() const { return valid() ? &(_itr->first) : 0; }
        virtual void* getElement() const { return valid() ? &(_itr->second) : 0; }
        virtual void setElement(void* ptr) const { if (valid()) _itr->second = *reinterpret_cast<ElementType*>(ptr); }
    };

    struct ReverseMapIterator : public MapIteratorObject
    {
        ReverseMapIterator(BaseSerializer::Type keyType, unsigned int keySize, BaseSerializer::Type elementType, unsigned int elementSize,
                    ReverseIterator itr, ReverseIterator endItr):
            MapIteratorObject(keyType, keySize, elementType, elementSize),
            _itr(itr),_endItr(endItr) {}

        ReverseIterator _itr;
        ReverseIterator _endItr;

        virtual bool advance() { if (valid()) ++_itr; return valid(); }
        virtual bool valid() const { return _itr!=_endItr; }
        virtual const void* getKey() const { return valid() ? &(_itr->first) : 0; }
        virtual void* getElement() const { return valid() ? &(_itr->second) : 0; }
        virtual void setElement(void* ptr) const { if (valid()) _itr->second = *reinterpret_cast<ElementType*>(ptr); }
    };

    virtual MapIteratorObject* createIterator(osg::Object& obj) const
    {
        C& object = OBJECT_CAST<C&>(obj);
        P& map = (object.*_getter)();
        return new MapIterator(_keyType, _keySize, _elementType, _elementSize, map.begin(), map.end());
    }

    virtual MapIteratorObject* createReverseIterator(osg::Object& obj) const
    {
        C& object = OBJECT_CAST<C&>(obj);
        P& map = (object.*_getter)();
        return new ReverseMapIterator(_keyType, _keySize, _elementType, _elementSize, map.rbegin(), map.rend());
    }

    virtual bool read( InputStream& is, osg::Object& obj )
    {
        C& object = OBJECT_CAST<C&>(obj);
        unsigned int size = 0;
        P map;
        if ( is.isBinary() )
        {
            is >> size;
            for ( unsigned int i=0; i<size; ++i )
            {
                KeyType key;
                ElementType value;
                is >> key >> value;
                map[key] = value;
            }
            (object.*_setter)( map );
        }
        else if ( is.matchString(_name) )
        {
            is >> size;
            if ( size>0 )
            {
                is >> is.BEGIN_BRACKET;
                for ( unsigned int i=0; i<size; ++i )
                {
                    KeyType key;
                    ElementType value;
                    is >> key >> value;
                    map[key] = value;
                }
                is >> is.END_BRACKET;
            }
            (object.*_setter)( map );
        }
        return true;
    }

    virtual bool write( OutputStream& os, const osg::Object& obj )
    {
        const C& object = OBJECT_CAST<const C&>(obj);
        const P& map = (object.*_constgetter)();
        unsigned int size = (unsigned int)map.size();
        if ( os.isBinary() )
        {
            os << size;
            for ( ConstIterator itr=map.begin();
                  itr!=map.end(); ++itr )
            {
                os << itr->first << itr->second;
            }
        }
        else if ( size>0 )
        {
            os << os.PROPERTY((_name).c_str()) << size << os.BEGIN_BRACKET << std::endl;
            for ( ConstIterator itr=map.begin(); itr!=map.end(); ++itr )
            {
                os << itr->first << itr->second; os << std::endl;
            }
            os << os.END_BRACKET << std::endl;
        }
        return true;
    }

public:
    std::string         _name;
    ConstGetter         _constgetter;
    Getter              _getter;
    Setter              _setter;
};

template<typename C, typename P=unsigned int>
class BitFlagsSerializer : public osgDB::TemplateSerializer<P>
{
public:
    typedef TemplateSerializer<P> ParentType;
    typedef P (C::*Getter)() const;
    typedef void (C::*Setter)( P );

    BitFlagsSerializer( const char* name, P def, Getter gf, Setter sf )
    : ParentType(name, def), _getter(gf), _setter(sf) {}

    void add( const char* str, P value )
    {
        _lookup.add(str, static_cast<osgDB::IntLookup::Value>(value));
    }

    P getValue( const char* str )
    { return static_cast<P>(_lookup.getValue(str)); }

    const std::string& getString( P value )
    { return _lookup.getString(static_cast<osgDB::IntLookup::Value>(value)); }

    virtual bool read( osgDB::InputStream& is, osg::Object& obj )
    {
        C& object = OBJECT_CAST<C&>(obj);
        if ( is.isBinary() )
        {
            if (is.getFileVersion()<123)
            {
                bool ok = false; is >> ok; //code from user serialized ensuring backwards-compatibility
                if ( !ok ) return true;
            }

            P mask;
            is >> mask;
            (object.*_setter)( mask );
        }
        else
        {
            if ( !is.matchString(ParentType::_name) ) //code from user serialized ensuring backwards-compatibility
                return true;
            P mask=P();
            std::string maskSetString;
            is >> maskSetString;
            osgDB::StringList maskList;
            osgDB::split( maskSetString, maskList, '|' );
            for ( unsigned int i=0; i<maskList.size(); ++i )
                mask |= _lookup.getValue( maskList[i].c_str());
            (object.*_setter)( mask );
        }
        return true;
    }

    virtual bool write( osgDB::OutputStream& os, const osg::Object& obj )
    {
        const C& object = OBJECT_CAST<const C&>(obj);
        const P mask = (object.*_getter)();
        bool ok = ParentType::_defaultValue!=static_cast<P>(mask);
        if ( os.isBinary() )
        {
            if (os.getFileVersion()<123)
            {
                os << ok;
                if ( !ok )
                    return true;
            }
            os << (int)mask; //just write int value in binary case
        }
        else
        {
            if ( !ok )
                return true;
            os << os.PROPERTY(ParentType::_name.c_str());

            std::string maskString;
            const osgDB::IntLookup::ValueToString& v2sm = _lookup.getValueToString();
            for( osgDB::IntLookup::ValueToString::const_iterator itr = v2sm.begin() ; itr != v2sm.end() ; itr++)
                if( (mask & itr->first) != 0 )
                    maskString += std::string(itr->second + "|");

            if ( !maskString.size() )
                maskString = std::string("NONE|");
            maskString.erase(maskString.size()-1,1);
            os << maskString << std::endl; //remove last "|"
        }
        return true;
    }

public:
    Getter _getter;
    Setter _setter;

protected:
    osgDB::IntLookup _lookup;
};

// ADDING MANIPULATORS
#define ADD_SERIALIZER(S) \
    wrapper->addSerializer( (S) )

#define ADD_USER_SERIALIZER(PROP) \
    wrapper->addSerializer( new osgDB::UserSerializer<MyClass>( \
        #PROP, &check##PROP, &read##PROP, &write##PROP), osgDB::BaseSerializer::RW_USER )

#define ADD_BOOL_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, bool >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_BOOL )

#define ADD_CHAR_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, char >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_CHAR )

#define ADD_UCHAR_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, unsigned char >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_UCHAR )

#define ADD_SHORT_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, short >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_SHORT )

#define ADD_USHORT_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, unsigned short >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_USHORT )

#define ADD_HEXSHORT_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, unsigned short >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP, true), osgDB::BaseSerializer::RW_USHORT )

#define ADD_INT_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, int >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_INT )

#define ADD_INT_SERIALIZER_NO_SET(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, int >( \
        #PROP, DEF, &MyClass::get##PROP, 0), osgDB::BaseSerializer::RW_INT )

#define ADD_UINT_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, unsigned int >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_UINT )

#define ADD_UINT_SERIALIZER_NO_SET(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, unsigned int >( \
        #PROP, DEF, &MyClass::get##PROP, 0), osgDB::BaseSerializer::RW_UINT )

#define ADD_GLINT_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, GLint >( \
        #PROP, ((int)(DEF)), &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_INT )

#define ADD_HEXINT_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, unsigned int >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP, true), osgDB::BaseSerializer::RW_UINT )

#define ADD_FLOAT_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, float >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_FLOAT )

#define ADD_DOUBLE_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, double >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_DOUBLE )

#define ADD_REF_BOOL_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, bool >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_BOOL )

#define ADD_REF_CHAR_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, char >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_CHAR )

#define ADD_REF_UCHAR_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, unsigned char >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_UCHAR )

#define ADD_REF_SHORT_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, short >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_SHORT )

#define ADD_REF_USHORT_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, unsigned short >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_USHORT )

#define ADD_REF_HEXSHORT_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, unsigned short >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP, true), osgDB::BaseSerializer::RW_USHORT )

#define ADD_REF_INT_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, int >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_INT )

#define ADD_REF_UINT_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, unsigned int >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_UINT )

#define ADD_REF_GLINT_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, GLint >( \
        #PROP, ((int)(DEF)), &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_INT )

#define ADD_REF_HEXINT_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, unsigned int >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP, true), osgDB::BaseSerializer::RW_UINT )

#define ADD_REF_FLOAT_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, float >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_FLOAT )

#define ADD_REF_DOUBLE_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, double >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_DOUBLE )


#define ADD_VEC2B_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec2b >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC2B )

#define ADD_VEC2UB_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec2ub >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC2UB )

#define ADD_VEC2S_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec2s >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC2S )

#define ADD_VEC2US_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec2us >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC2US )

#define ADD_VEC2I_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec2i >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC2I )

#define ADD_VEC2UI_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec2ui >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC2UI )

#define ADD_VEC2F_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec2f >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC2F )

#define ADD_VEC2D_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec2d >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC2D )

#define ADD_VEC2_SERIALIZER(PROP, DEF) ADD_VEC2F_SERIALIZER(PROP, DEF)


#define ADD_VEC3B_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec3b >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC3B )

#define ADD_VEC3UB_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec3ub >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC3UB )

#define ADD_VEC3S_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec3s >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC3S )

#define ADD_VEC3US_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec3us >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC3US )

#define ADD_VEC3I_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec3i >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC3I )

#define ADD_VEC3UI_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec3ui >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC3UI )

#define ADD_VEC3F_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec3f >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC3F )

#define ADD_VEC3D_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec3d >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC3D )

#define ADD_VEC3_SERIALIZER(PROP, DEF) ADD_VEC3F_SERIALIZER(PROP, DEF)

#define ADD_VEC4B_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec4b >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC4B )

#define ADD_VEC4UB_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec4ub >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC4UB )

#define ADD_VEC4S_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec4s >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC4S )

#define ADD_VEC4US_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec4us >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC4US )

#define ADD_VEC4I_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec4i >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC4I )

#define ADD_VEC4UI_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec4ui >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC4UI )

#define ADD_VEC4F_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec4f >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC4F )

#define ADD_VEC4D_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Vec4d >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_VEC4D )

#define ADD_VEC4_SERIALIZER(PROP, DEF) ADD_VEC4F_SERIALIZER(PROP, DEF)

#define ADD_QUAT_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Quat >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_QUAT )

#define ADD_PLANE_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Plane >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_PLANE )

#define ADD_MATRIXF_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Matrixf >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_MATRIXF )

#define ADD_MATRIXD_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::Matrixd >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_MATRIXD )

#define ADD_MATRIX_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::MatrixSerializer< MyClass >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_MATRIX )


#define ADD_BOUNDINGBOXF_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::BoundingBoxf >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_BOUNDINGBOXF )

#define ADD_BOUNDINGBOXD_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::BoundingBoxd >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_BOUNDINGBOXD )

#define ADD_BOUNDINGSPHEREF_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::BoundingSpheref >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_BOUNDINGSPHEREF )

#define ADD_BOUNDINGSPHERED_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, osg::BoundingSphered >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_BOUNDINGSPHERED )


#define ADD_GLENUM_SERIALIZER(PROP, TYPE, DEF) \
    wrapper->addSerializer( new osgDB::GLenumSerializer< MyClass, TYPE >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_GLENUM )

#define ADD_GLENUM_SERIALIZER_NO_SET(PROP, TYPE, DEF) \
    wrapper->addSerializer( new osgDB::GLenumSerializer< MyClass, TYPE >( \
        #PROP, DEF, &MyClass::get##PROP, 0), osgDB::BaseSerializer::RW_GLENUM )

#define ADD_STRING_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::StringSerializer< MyClass >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_STRING )

#define ADD_OBJECT_SERIALIZER(PROP, TYPE, DEF) \
    wrapper->addSerializer( new osgDB::ObjectSerializer< MyClass, TYPE >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_OBJECT )

#define ADD_OBJECT_SERIALIZER_NO_SET(PROP, TYPE, DEF) \
    wrapper->addSerializer( new osgDB::ObjectSerializer< MyClass, TYPE >( \
        #PROP, DEF, &MyClass::get##PROP, 0), osgDB::BaseSerializer::RW_OBJECT )

#define ADD_IMAGE_SERIALIZER(PROP, TYPE, DEF) \
    wrapper->addSerializer( new osgDB::ImageSerializer< MyClass, TYPE >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_IMAGE )

#define ADD_LIST_SERIALIZER(PROP, TYPE) \
    wrapper->addSerializer( new osgDB::ListSerializer< MyClass, TYPE >( \
        #PROP, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_LIST )


#define ADD_VECTOR_SERIALIZER(PROP, TYPE, ELEMENTTYPE, NUMELEMENTSONROW) \
    wrapper->addSerializer( new osgDB::VectorSerializer< MyClass, TYPE >( \
        #PROP, &MyClass::get##PROP, &MyClass::get##PROP, &MyClass::set##PROP, ELEMENTTYPE, NUMELEMENTSONROW), osgDB::BaseSerializer::RW_VECTOR )

#define ADD_ISAVECTOR_SERIALIZER(PROP, ELEMENTTYPE, NUMELEMENTSONROW) \
    wrapper->addSerializer( new osgDB::IsAVectorSerializer< MyClass >( #PROP, ELEMENTTYPE, NUMELEMENTSONROW), osgDB::BaseSerializer::RW_VECTOR )

#define BEGIN_ENUM_SERIALIZER(PROP, DEF) \
    { typedef osgDB::EnumSerializer<MyClass, MyClass::PROP, void> MySerializer; \
    osg::ref_ptr<MySerializer> serializer = new MySerializer( \
        #PROP, MyClass::DEF, &MyClass::get##PROP, &MyClass::set##PROP)

#define BEGIN_ENUM_SERIALIZER2(PROP, TYPE, DEF) \
    { typedef osgDB::EnumSerializer<MyClass, TYPE, void> MySerializer; \
    osg::ref_ptr<MySerializer> serializer = new MySerializer( \
        #PROP, MyClass::DEF, &MyClass::get##PROP, &MyClass::set##PROP)

#define BEGIN_ENUM_SERIALIZER3(PROP, DEF) \
    { typedef osgDB::EnumSerializer<MyClass, MyClass::PROP, bool> MySerializer; \
    osg::ref_ptr<MySerializer> serializer = new MySerializer( \
        #PROP, MyClass::DEF, &MyClass::get##PROP, &MyClass::set##PROP)

#define BEGIN_ENUM_SERIALIZER4(PROPERTIES_CLASS, PROP, DEF) \
    { typedef osgDB::EnumSerializer<MyClass, PROPERTIES_CLASS::PROP, void> MySerializer; \
    osg::ref_ptr<MySerializer> serializer = new MySerializer( \
        #PROP, PROPERTIES_CLASS::DEF, &MyClass::get##PROP, &MyClass::set##PROP)

#define BEGIN_ENUM_SERIALIZER_NO_SET(PROP, DEF) \
    { typedef osgDB::EnumSerializer<MyClass, MyClass::PROP, void> MySerializer; \
    osg::ref_ptr<MySerializer> serializer = new MySerializer( \
        #PROP, MyClass::DEF, &MyClass::get##PROP, 0)

#define ADD_ENUM_VALUE(VALUE) \
    serializer->add(#VALUE, MyClass::VALUE)

#define ADD_ENUM_CLASS_VALUE(CLASS, VALUE) \
    serializer->add(#VALUE, CLASS::VALUE)

#define END_ENUM_SERIALIZER() \
    wrapper->addSerializer(serializer.get(), osgDB::BaseSerializer::RW_ENUM); }

/** defaults to uint bitfield type.*/
#define BEGIN_BITFLAGS_SERIALIZER(PROP, DEF) \
    { typedef osgDB::BitFlagsSerializer<MyClass> MySerializer; \
    osg::ref_ptr<MySerializer> serializer = new MySerializer( \
    #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP)

#define BEGIN_UINT_BITFLAGS_SERIALIZER(PROP, DEF) \
    { typedef osgDB::BitFlagsSerializer<MyClass, unsigned int> MySerializer; \
    osg::ref_ptr<MySerializer> serializer = new MySerializer( \
    #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP)

#define BEGIN_INT_BITFLAGS_SERIALIZER(PROP, DEF) \
    { typedef osgDB::BitFlagsSerializer<MyClass, int> MySerializer; \
    osg::ref_ptr<MySerializer> serializer = new MySerializer( \
    #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP)

#define ADD_BITFLAG_VALUE(VALUE_NAME, VALUE) \
    serializer->add(#VALUE_NAME, VALUE)

#define END_BITFLAGS_SERIALIZER() \
    wrapper->addSerializer(serializer.get(), osgDB::BaseSerializer::RW_INT); }

// VERSION CONTROL OPERATORS
#define UPDATE_TO_VERSION(VER) \
    wrapper->setUpdatedVersion( (VER) );

#define UPDATE_TO_VERSION_SCOPED(VER) \
    osgDB::UpdateWrapperVersionProxy uwvp(wrapper, (VER));

#define ADDED_ASSOCIATE(STR) \
    wrapper->markAssociateAsAdded( STR );

#define REMOVED_ASSOCIATE(STR) \
    wrapper->markAssociateAsRemoved( STR );

#define REMOVE_SERIALIZER(PROP) \
    wrapper->markSerializerAsRemoved( #PROP );

#define ADD_MAP_SERIALIZER(PROP, TYPE, KEYTYPE, ELEMENTTYPE) \
    wrapper->addSerializer( new osgDB::MapSerializer< MyClass, TYPE >( \
        #PROP, &MyClass::get##PROP, &MyClass::get##PROP, &MyClass::set##PROP, KEYTYPE, ELEMENTTYPE), osgDB::BaseSerializer::RW_MAP )

#define ADD_METHOD_OBJECT( METHODNAME, METHODOBJECTCLASS ) wrapper->addMethodObject(METHODNAME, new METHODOBJECTCLASS());

#define ADD_METHOD(METHODNAME) \
    { \
        struct MethodCaller : public osgDB::MethodObject \
        { \
            virtual bool run(void* objectPtr, osg::Parameters&, osg::Parameters&) const \
            { \
                MyClass* obj = reinterpret_cast<MyClass*>(objectPtr); \
                obj->METHODNAME(); \
                return true; \
            } \
        }; \
        wrapper->addMethodObject(#METHODNAME, new MethodCaller()); \
    }


#define SET_USAGE(VALUE) wrapper->getLastSerializer()->setUsage(VALUE)

}

#endif
