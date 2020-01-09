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
#ifndef OSGEARTHUTIL_ATLAS_BUILDER_H
#define OSGEARTHUTIL_ATLAS_BUILDER_H

#include <osgEarthUtil/Common>
#include <osgEarthSymbology/ResourceLibrary>
#include <osg/Vec4f>
#include <vector>

namespace osgDB {
    class Options;
}

namespace osgEarth { namespace Util
{
    /**
     * Compiles a resource library into a texture atlas.
     */
    class OSGEARTHUTIL_EXPORT AtlasBuilder
    {
    public:
        struct Atlas
        {
            // resulting atlas images. The first one is the master atlas image;
            // any following ones correspond to images created based on auxiliary
            // patterns added with addAuxFilePattern.
            std::vector<osg::ref_ptr<osg::Image> >             _images;

            // resulting resource library for the atlas
            osg::ref_ptr<osgEarth::Symbology::ResourceLibrary> _lib;
        };

    public:
        /** Construct a new atlas builder */
        AtlasBuilder(const osgDB::Options* options =0L);

        /** Sets the maximum size of each layer in the output atlas */
        void setSize(unsigned width, unsigned height);

        /** Adds an auxiliary file pattern that should be built, along with the
            default RGBA to use when the aux file doesn't exist. */
        void addAuxFilePattern(const std::string& pattern, const osg::Vec4f& defRGBA);

        /** List of aux file patterns */
        const std::vector<std::string>& auxFilePatterns() const { return _auxPatterns; }
        const std::vector<osg::Vec4f>& auxDefaultValues() const { return _auxDefaults; }

        /**
         * Whether to generate an RGB texture.  By default RGBA textures are created.
         */
        bool getRGB() const { return _rgb; }
        void setRGB( bool rgb ) { _rgb = rgb; }

        /** Builds an atlas. */
        bool build(
            const osgEarth::Symbology::ResourceLibrary* input,
            const std::string&                          newAtlasURI,
            Atlas&                                      output) const;


    protected:
        unsigned _width;
        unsigned _height;
        osg::ref_ptr<const osgDB::Options> _options;
        std::vector<std::string> _auxPatterns;
        std::vector<osg::Vec4f>  _auxDefaults;
        bool _debug;
        bool _rgb;
    };

} } // namespace osgEarth::Util

#endif // OSGEARTHUTIL_ATLAS_BUILDER_H
