/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
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

#ifndef OSGDB_READERWRITER
#define OSGDB_READERWRITER 1

#include <osg/Image>
#include <osg/Shape>
#include <osg/Node>
#include <osg/ScriptEngine>

#include <osgDB/AuthenticationMap>

#include <deque>
#include <list>
#include <iosfwd>

namespace osgDB {

class Archive;

/** List of directories to search through which searching for files. */
typedef std::deque<std::string> FilePathList;

// forward declare
class Options;

/** Pure virtual base class for reading and writing of non native formats. */
class OSGDB_EXPORT ReaderWriter : public osg::Object
{
    public:


        ReaderWriter():
            osg::Object(true) {}

        ReaderWriter(const ReaderWriter& rw,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
            osg::Object(rw,copyop) {}

        virtual ~ReaderWriter();

        META_Object(osgDB,ReaderWriter);

        typedef std::map<std::string, std::string> FormatDescriptionMap;
        typedef std::list<std::string> FeatureList;

        /** Return which protocols are supported by ReaderWriter. */
        virtual const FormatDescriptionMap& supportedProtocols() const { return _supportedProtocols; }

        /** Return which list of file extensions supported by ReaderWriter. */
        virtual const FormatDescriptionMap& supportedExtensions() const { return _supportedExtensions; }

        /** Return which list of file extensions supported by ReaderWriter. */
        virtual const FormatDescriptionMap& supportedOptions() const { return _supportedOptions; }

        /** Return true if ReaderWriter accepts specified file extension.*/
        virtual bool acceptsExtension(const std::string& /*extension*/) const;

        virtual bool acceptsProtocol(const std::string& protocol) const;

        /// Bit mask for setting up which feature types are available for read and/or write
        enum Features
        {
            FEATURE_NONE               = 0,
            FEATURE_READ_OBJECT        = 1<<0,
            FEATURE_READ_IMAGE         = 1<<1,
            FEATURE_READ_HEIGHT_FIELD  = 1<<2,
            FEATURE_READ_NODE          = 1<<3,
            FEATURE_READ_SHADER        = 1<<4,
            FEATURE_WRITE_OBJECT       = 1<<5,
            FEATURE_WRITE_IMAGE        = 1<<6,
            FEATURE_WRITE_HEIGHT_FIELD = 1<<7,
            FEATURE_WRITE_NODE         = 1<<8,
            FEATURE_WRITE_SHADER       = 1<<9,
            FEATURE_READ_SCRIPT        = 1<<10,
            FEATURE_WRITE_SCRIPT       = 1<<11,
            FEATURE_ALL                = FEATURE_READ_OBJECT        |
                                         FEATURE_READ_IMAGE         |
                                         FEATURE_READ_HEIGHT_FIELD  |
                                         FEATURE_READ_NODE          |
                                         FEATURE_READ_SHADER        |
                                         FEATURE_READ_SCRIPT        |
                                         FEATURE_WRITE_OBJECT       |
                                         FEATURE_WRITE_IMAGE        |
                                         FEATURE_WRITE_HEIGHT_FIELD |
                                         FEATURE_WRITE_NODE         |
                                         FEATURE_WRITE_SHADER       |
                                         FEATURE_WRITE_SCRIPT
        };
        /** Return available features*/
        virtual Features supportedFeatures() const;

        /** Return feature as string */
        static FeatureList featureAsString(Features feature);



        class OSGDB_EXPORT ReadResult
        {
            public:

                enum ReadStatus
                {
                    NOT_IMPLEMENTED, //!< read*() method not implemented in concrete ReaderWriter.
                    FILE_NOT_HANDLED, //!< File is not appropriate for this file reader, due to some incompatibility, but *not* a read error.
                    FILE_NOT_FOUND, //!< File could not be found or could not be read.
                    ERROR_IN_READING_FILE, //!< File found, loaded, but an error was encountered during processing.
                    FILE_LOADED, //!< File successfully found, loaded, and converted into osg.
                    FILE_LOADED_FROM_CACHE, //!< File found in cache and returned.
                    FILE_REQUESTED, //!< Asynchronous file read has been requested, but returning immediately, keep polling plugin until file read has been completed.
                    INSUFFICIENT_MEMORY_TO_LOAD //!< File found but not loaded because estimated required memory surpasses available memory.
                };

                ReadResult(ReadStatus status=FILE_NOT_HANDLED):_status(status) {}
                ReadResult(const std::string& m):_status(ERROR_IN_READING_FILE),_message(m) {}

                ReadResult(osg::Object* obj, ReadStatus status=FILE_LOADED):_status(status),_object(obj) {}

                template<class T>
                ReadResult(const osg::ref_ptr<T>& obj, ReadStatus status=FILE_LOADED):_status(status),_object(obj.get()) {}

                ReadResult(const ReadResult& rr):_status(rr._status),_message(rr._message),_object(rr._object) {}
                ReadResult& operator = (const ReadResult& rr) { if (this==&rr) return *this; _status=rr._status; _message=rr._message;_object=rr._object; return *this; }

                bool operator < (const ReadResult& rhs) const { return _status < rhs._status; }

                osg::Object* getObject();
                osg::Image* getImage();
                osg::HeightField* getHeightField();
                osg::Node* getNode();
                osgDB::Archive* getArchive();
                osg::Shader* getShader();
                osg::Script* getScript();

                bool validObject() { return _object.valid(); }
                bool validImage() { return getImage()!=0; }
                bool validHeightField() { return getHeightField()!=0; }
                bool validNode() { return getNode()!=0; }
                bool validArchive() { return getArchive()!=0; }
                bool validShader() { return getShader()!=0; }
                bool validScript() { return getScript()!=0; }

                osg::Object* takeObject();
                osg::Image* takeImage();
                osg::HeightField* takeHeightField();
                osg::Node* takeNode();
                osgDB::Archive* takeArchive();
                osg::Shader* takeShader();
                osg::Script* takeScript();

                std::string& message() { return _message; }
                const std::string& message() const { return _message; }

                /// report the ReadResult's status, and message (if any). Useful for reporting of errors to users.
                std::string statusMessage() const;

                ReadStatus status() const { return _status; }
                bool success() const { return _status==FILE_LOADED || _status==FILE_LOADED_FROM_CACHE ; }
                bool loadedFromCache() const { return _status==FILE_LOADED_FROM_CACHE; }
                bool error() const { return _status==ERROR_IN_READING_FILE; }
                bool notHandled() const { return _status==FILE_NOT_HANDLED || _status==NOT_IMPLEMENTED; }
                bool notFound() const { return _status==FILE_NOT_FOUND; }
                bool notEnoughMemory() const { return _status==INSUFFICIENT_MEMORY_TO_LOAD; }

            protected:

                ReadStatus                  _status;
                std::string                 _message;
                osg::ref_ptr<osg::Object>   _object;

        };

        class WriteResult
        {
            public:

                enum WriteStatus
                {
                    NOT_IMPLEMENTED, //!< write*() method not implemented in concrete ReaderWriter.
                    FILE_NOT_HANDLED,
                    ERROR_IN_WRITING_FILE,
                    FILE_SAVED
                };

                WriteResult(WriteStatus status=FILE_NOT_HANDLED):_status(status) {}
                WriteResult(const std::string& m):_status(ERROR_IN_WRITING_FILE),_message(m) {}

                WriteResult(const WriteResult& rr):_status(rr._status),_message(rr._message) {}
                WriteResult& operator = (const WriteResult& rr) { if (this==&rr) return *this; _status=rr._status; _message=rr._message; return *this; }

                bool operator < (const WriteResult& rhs) const { return _status < rhs._status; }

                std::string& message() { return _message; }
                const std::string& message() const { return _message; }

                /// Report the WriteResult's status, and message (if any). Useful for reporting of errors to users.
                std::string statusMessage() const;

                WriteStatus status() const { return _status; }
                bool success() const { return _status==FILE_SAVED; }
                bool error() const { return _status==ERROR_IN_WRITING_FILE; }
                bool notHandled() const { return _status==FILE_NOT_HANDLED || _status==NOT_IMPLEMENTED; }

            protected:

                WriteStatus                 _status;
                std::string                 _message;
        };

        enum ArchiveStatus
        {
            READ,
            WRITE,
            CREATE
        };

        typedef osgDB::Options Options;

        /** Determine if a file exists, normally the default implementation will be appropriate for local file access
         *  but with plugins like the libcurl based one it will return true if the file is accessible at the server. */
        virtual bool fileExists(const std::string& filename, const Options* options) const;

        /** Open an archive for reading, writing, or to create an empty archive for writing to.*/
        virtual ReadResult openArchive(const std::string& /*fileName*/,ArchiveStatus, unsigned int =4096, const Options* =NULL) const { return ReadResult(ReadResult::NOT_IMPLEMENTED); }

        /** Open an archive for reading.*/
        virtual ReadResult openArchive(std::istream& /*fin*/,const Options* =NULL) const { return ReadResult(ReadResult::NOT_IMPLEMENTED); }

        virtual ReadResult readObject(const std::string& /*fileName*/,const Options* =NULL) const { return ReadResult(ReadResult::NOT_IMPLEMENTED); }
        virtual ReadResult readImage(const std::string& /*fileName*/,const Options* =NULL) const { return ReadResult(ReadResult::NOT_IMPLEMENTED); }
        virtual ReadResult readHeightField(const std::string& /*fileName*/,const Options* =NULL) const { return ReadResult(ReadResult::NOT_IMPLEMENTED); }
        virtual ReadResult readNode(const std::string& /*fileName*/,const Options* =NULL) const { return ReadResult(ReadResult::NOT_IMPLEMENTED); }
        virtual ReadResult readShader(const std::string& /*fileName*/,const Options* =NULL) const { return ReadResult(ReadResult::NOT_IMPLEMENTED); }
        virtual ReadResult readScript(const std::string& /*fileName*/,const Options* =NULL) const { return ReadResult(ReadResult::NOT_IMPLEMENTED); }

        virtual WriteResult writeObject(const osg::Object& /*obj*/,const std::string& /*fileName*/,const Options* =NULL) const {return WriteResult(WriteResult::NOT_IMPLEMENTED); }
        virtual WriteResult writeImage(const osg::Image& /*image*/,const std::string& /*fileName*/,const Options* =NULL) const {return WriteResult(WriteResult::NOT_IMPLEMENTED); }
        virtual WriteResult writeHeightField(const osg::HeightField& /*heightField*/,const std::string& /*fileName*/,const Options* =NULL) const {return WriteResult(WriteResult::NOT_IMPLEMENTED); }
        virtual WriteResult writeNode(const osg::Node& /*node*/,const std::string& /*fileName*/,const Options* =NULL) const { return WriteResult(WriteResult::NOT_IMPLEMENTED); }
        virtual WriteResult writeShader(const osg::Shader& /*shader*/,const std::string& /*fileName*/,const Options* =NULL) const {return WriteResult(WriteResult::NOT_IMPLEMENTED); }
        virtual WriteResult writeScript(const osg::Script& /*script*/,const std::string& /*fileName*/,const Options* =NULL) const {return WriteResult(WriteResult::NOT_IMPLEMENTED); }

        virtual ReadResult readObject(std::istream& /*fin*/,const Options* =NULL) const { return ReadResult(ReadResult::NOT_IMPLEMENTED); }
        virtual ReadResult readImage(std::istream& /*fin*/,const Options* =NULL) const { return ReadResult(ReadResult::NOT_IMPLEMENTED); }
        virtual ReadResult readHeightField(std::istream& /*fin*/,const Options* =NULL) const { return ReadResult(ReadResult::NOT_IMPLEMENTED); }
        virtual ReadResult readNode(std::istream& /*fin*/,const Options* =NULL) const { return ReadResult(ReadResult::NOT_IMPLEMENTED); }
        virtual ReadResult readShader(std::istream& /*fin*/,const Options* =NULL) const { return ReadResult(ReadResult::NOT_IMPLEMENTED); }
        virtual ReadResult readScript(std::istream& /*fin*/,const Options* =NULL) const { return ReadResult(ReadResult::NOT_IMPLEMENTED); }

        virtual WriteResult writeObject(const osg::Object& /*obj*/,std::ostream& /*fout*/,const Options* =NULL) const { return WriteResult(WriteResult::NOT_IMPLEMENTED); }
        virtual WriteResult writeImage(const osg::Image& /*image*/,std::ostream& /*fout*/,const Options* =NULL) const { return WriteResult(WriteResult::NOT_IMPLEMENTED); }
        virtual WriteResult writeHeightField(const osg::HeightField& /*heightField*/,std::ostream& /*fout*/,const Options* =NULL) const { return WriteResult(WriteResult::NOT_IMPLEMENTED); }
        virtual WriteResult writeNode(const osg::Node& /*node*/,std::ostream& /*fout*/,const Options* =NULL) const { return WriteResult(WriteResult::NOT_IMPLEMENTED); }
        virtual WriteResult writeShader(const osg::Shader& /*shader*/,std::ostream& /*fout*/,const Options* =NULL) const { return WriteResult(WriteResult::NOT_IMPLEMENTED); }
        virtual WriteResult writeScript(const osg::Script& /*script*/,std::ostream& /*fout*/,const Options* =NULL) const { return WriteResult(WriteResult::NOT_IMPLEMENTED); }

        /** Specify fmt string as a supported protocol.
          * Please note, this method should usually only be used internally by subclasses of ReaderWriter, Only in special cases
          * will a ReaderWriter implementation be able to handle a protocol format that it wasn't originally designed for.
          * To know whether it's safe to inject a new protocol format into an existing ReaderWriter you will need to review
          * the source code and dependencies of that ReaderWriter. */
        void supportsProtocol(const std::string& fmt, const std::string& description);

        /** Specify ext string as a supported file extension.
          * Please note, this method should usually only be used internally by subclasses of ReaderWriter. Only in special cases
          * will a ReaderWriter implementation be able to handle a file extension that it wasn't originally designed for.
          * To know whether it's safe to inject a new file extension into an existing ReaderWriter you will need to review the
          * the source code and dependencies of that ReaderWriter. */
        void supportsExtension(const std::string& ext, const std::string& description);

        /** Specify option string as a supported option string.
          * Please note, this should usually only be used internally by subclasses of ReaderWriter. */
        void supportsOption(const std::string& opt, const std::string& description);

    protected:

        FormatDescriptionMap _supportedProtocols;
        FormatDescriptionMap _supportedExtensions;
        FormatDescriptionMap _supportedOptions;
};

}

#endif // OSGDB_READERWRITER
