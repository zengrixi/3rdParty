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

#ifndef OSGEARTH_IOTYPES_H
#define OSGEARTH_IOTYPES_H 1

#include <osgEarth/Config>
#include <osgEarth/DateTime>

/**
 * A collectin of types used by the various I/O systems in osgEarth. These
 * are extended variations on some of OSG's ReaderWriter types.
 */
namespace osgEarth
{
    /**
     * String wrapped in an osg::Object (for I/O purposes)
     */
    class OSGEARTH_EXPORT StringObject : public osg::Object
    {
    public:
        StringObject();
        StringObject( const StringObject& rhs, const osg::CopyOp& op ) : osg::Object(rhs, op), _str(rhs._str) { }
        StringObject( const std::string& in ) : osg::Object(), _str(in) { }

        /** dtor */
        virtual ~StringObject();
        META_Object( osgEarth, StringObject );

        void setString( const std::string& value );
        const std::string& getString() const;
    private:
        std::string _str;
    };


//--------------------------------------------------------------------

    /**
     * Convenience metadata tags
     */
    struct OSGEARTH_EXPORT IOMetadata
    {
        static const std::string CONTENT_TYPE;
    };

//--------------------------------------------------------------------

    /**
     * Return value from a read* method
     */
    struct OSGEARTH_EXPORT ReadResult
    {
        /** Read result codes. */
        enum Code
        {
            RESULT_OK,
            RESULT_CANCELED,
            RESULT_NOT_FOUND,
            RESULT_EXPIRED,
            RESULT_SERVER_ERROR,
            RESULT_TIMEOUT,
            RESULT_NO_READER,
            RESULT_READER_ERROR,
            RESULT_UNKNOWN_ERROR,
            RESULT_NOT_IMPLEMENTED,
            RESULT_NOT_MODIFIED
        };

        /** Construct a result with no object */
        ReadResult( Code code =RESULT_NOT_FOUND )
            : _code(code), _fromCache(false), _lmt(0), _duration_s(0.0) { }

        /** Construct a result with an error message */
        ReadResult(const std::string& error)
            : _code(RESULT_NOT_FOUND), _fromCache(false), _lmt(0), _duration_s(0.0), _detail(error) { }

        /** Construct a result with code and data */
        ReadResult( Code code, osg::Object* result )
            : _code(code), _result(result), _fromCache(false), _lmt(0), _duration_s(0.0) { }

        /** Construct a result with data, possible with an error code */
        ReadResult( Code code, osg::Object* result, const Config& meta )
            : _code(code), _result(result), _meta(meta), _fromCache(false), _lmt(0), _duration_s(0.0) { }

        /** Construct a successful result (implicit OK code) */
        ReadResult( osg::Object* result )
            : _code(RESULT_OK), _result(result), _fromCache(false), _lmt(0), _duration_s(0.0) { }

        template<typename T>
        ReadResult( const osg::ref_ptr<T>& result )
            : _code(RESULT_OK), _result(result), _fromCache(false), _lmt(0), _duration_s(0.0) { }

        /** Construct a successful result with metadata */
        ReadResult( osg::Object* result, const Config& meta )
            : _code(RESULT_OK), _result(result), _meta(meta), _fromCache(false), _lmt(0), _duration_s(0.0) { }

        template<typename T>
        ReadResult( const osg::ref_ptr<T>& result, const Config& meta )
            : _code(RESULT_OK), _result(result), _meta(meta), _fromCache(false), _lmt(0), _duration_s(0.0) { }

        /** Copy construct */
        ReadResult( const ReadResult& rhs )
            : _code(rhs._code), _result(rhs._result.get()), _meta(rhs._meta), _fromCache(rhs._fromCache), _lmt(rhs._lmt), _duration_s(rhs._duration_s) { }

        /** dtor */
        virtual ~ReadResult() { }

        /** Whether the read operation succeeded */
        bool succeeded() const { return _code == RESULT_OK && _result.valid(); }

        /** Whether the read operation failed */
        bool failed() const { return _code != RESULT_OK; }

        /** Whether the result contains an object */
        bool empty() const { return !_result.valid(); }

        /** Detail message, sometimes set upon error */
        const std::string& errorDetail() const { return _detail; }

        /** The result code */
        const Code& code() const { return _code; }

        /** Last modified timestamp */
        TimeStamp lastModifiedTime() const { return _lmt; }

        /** Duration of request/response in seconds */
        double duration() const { return _duration_s; }

        /** True if the object came from the cache */
        bool isFromCache() const { return _fromCache; }

        /** The result */
        osg::Object* getObject() const { return _result.get(); }
        osg::Image*  getImage()  const { return get<osg::Image>(); }
        osg::Node*   getNode()   const { return get<osg::Node>(); }

        /** The result, transfering ownership to the caller */
        osg::Object* releaseObject() { return _result.release(); }
        osg::Image*  releaseImage()  { return release<osg::Image>(); }
        osg::Node*   releaseNode()   { return release<osg::Node>(); }

        /** The metadata */
        const Config& metadata() const { return _meta; }

        /** The result, cast to a custom type */
        template<typename T>
        T* get() const { return dynamic_cast<T*>(_result.get()); }

        /** The result, cast to a custom type and transfering ownership to the caller*/
        template<typename T>
        T* release() { return dynamic_cast<T*>(_result.get())? static_cast<T*>(_result.release()) : 0L; }

        /** The result as a string */
        const std::string& getString() const { const StringObject* so = dynamic_cast<StringObject*>(_result.get()); return so ? so->getString() : _emptyString; }
        
        /** Gets a string describing the read result */
        static std::string getResultCodeString( unsigned code )
        {
            return
                code == RESULT_OK              ? "OK" :
                code == RESULT_CANCELED        ? "Read canceled" :
                code == RESULT_NOT_FOUND       ? "Target not found" :
                code == RESULT_SERVER_ERROR    ? "Server reported error" :
                code == RESULT_TIMEOUT         ? "Read timed out" :
                code == RESULT_NO_READER       ? "No suitable ReaderWriter found" :
                code == RESULT_READER_ERROR    ? "ReaderWriter error" :
                code == RESULT_NOT_IMPLEMENTED ? "Not implemented" :
                                                 "Unknown error";
        }

        std::string getResultCodeString() const
        {
            return getResultCodeString( _code );
        }

    public:
        void setIsFromCache(bool value) { _fromCache = value; }

        void setLastModifiedTime(TimeStamp t) { _lmt = t; }

        void setDuration(double s) { _duration_s = s; }

        void setMetadata(const Config& meta) { _meta = meta; }

        void setErrorDetail(const std::string& value) { _detail = value; }

    protected:
        Code                      _code;
        osg::ref_ptr<osg::Object> _result;
        Config                    _meta;
        std::string               _emptyString;
        Config                    _emptyConfig;
        bool                      _fromCache;
        TimeStamp                 _lmt;
        double                    _duration_s;
        std::string               _detail;
    };

//--------------------------------------------------------------------

    /**
     * Callback that allows the developer to re-route URI read calls. 
     *
     * If the corresponding callback method returns NOT_IMPLEMENTED, URI will
     * fall back on its default mechanism.
     */
    class OSGEARTH_EXPORT URIReadCallback : public osg::Referenced
    {
    public:
        enum CachingSupport
        {
            CACHE_NONE        = 0,
            CACHE_OBJECTS     = 1 << 0,
            CACHE_NODES       = 1 << 1,
            CACHE_IMAGES      = 1 << 2,
            CACHE_STRINGS     = 1 << 3,
            CACHE_CONFIGS     = 1 << 4,
            CACHE_ALL         = ~0
        };

        /** 
         * Tells the URI class which data types (if any) from this callback should be subjected
         * to osgEarth's caching mechamism. By default, the answer is "none" - URI
         * will not attempt to read or write from its cache when using this callback.
         */
        virtual unsigned cachingSupport() const { return CACHE_NONE; }

    public:

        /** Override the readObject() implementation */
        virtual osgEarth::ReadResult readObject( const std::string& uri, const osgDB::Options* options ) {
            return osgEarth::ReadResult::RESULT_NOT_IMPLEMENTED; }

        /** Override the readNode() implementation */
        virtual osgEarth::ReadResult readNode( const std::string& uri, const osgDB::Options* options ) {
            return osgEarth::ReadResult::RESULT_NOT_IMPLEMENTED; }

        /** Override the readImage() implementation */
        virtual osgEarth::ReadResult readImage( const std::string& uri, const osgDB::Options* options ) {
            return osgEarth::ReadResult::RESULT_NOT_IMPLEMENTED; }

        /** Override the readString() implementation */
        virtual osgEarth::ReadResult readString( const std::string& uri, const osgDB::Options* options ) {
            return osgEarth::ReadResult::RESULT_NOT_IMPLEMENTED; }

        /** Override the readConfig() implementation */
        virtual osgEarth::ReadResult readConfig( const std::string& uri, const osgDB::Options* options ) {
            return osgEarth::ReadResult::RESULT_NOT_IMPLEMENTED; }

    protected:

        URIReadCallback();

        /** dtor */
        virtual ~URIReadCallback();
    };

}

#endif // OSGEARTH_IOTYPES_H
