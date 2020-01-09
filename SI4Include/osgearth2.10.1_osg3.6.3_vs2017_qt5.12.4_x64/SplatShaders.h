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
#ifndef OSGEARTH_SPLAT_SHADERS
#define OSGEARTH_SPLAT_SHADERS 1

#include <osgEarth/ShaderLoader>

namespace osgEarth { namespace Splat
{
    struct SplattingShaders : public osgEarth::ShaderPackage
	{
        SplattingShaders();

        std::string
            Types,
            Noise,
            VertModel,
            VertView,
            Frag,
            FragCommon,
            //FragGetRenderInfo,
            Util;
	};

    struct GroundCoverShaders : public osgEarth::ShaderPackage
	{
        GroundCoverShaders();

        std::string
            GroundCover_TCS,
            GroundCover_TES,
            GroundCover_GS,
            GroundCover_FS;
	};
	
} } // namespace osgEarth::Splat

#endif // OSGEARTH_SPLAT_SHADERS
