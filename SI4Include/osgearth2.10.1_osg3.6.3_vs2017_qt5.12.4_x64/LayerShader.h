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
#ifndef OSGEARTH_LAYER_SHADER_H
#define OSGEARTH_LAYER_SHADER_H 1

#include <osgEarth/Config>
#include <osgEarth/URI>
#include <osgEarth/TerrainResources>
#include <osgDB/Options>
#include <vector>
#include <string>

namespace osg {
    class StateSet;
}

namespace osgEarth
{
    class Layer;

    //! Serializable shader that supports samplers and uniforms
    //! in an earth file.
    class ShaderOptions : public ConfigOptions
    {
    public:
        META_ConfigOptions(osgEarth, ShaderOptions, ConfigOptions);

        struct Sampler
        {
            std::string      _name;
            std::vector<URI> _uris;
        };

        struct Uniform
        {
            std::string     _name;
            optional<float> _value;
        };

        void apply(osg::StateSet*, Layer*, TerrainResources*, const osgDB::Options*);

    public:
        std::string& code() { return _code; }
        const std::string& code() const { return _code; }

        std::vector<Sampler>& samplers() { return _samplers; }
        const std::vector<Sampler>& samplers() const { return _samplers; }

        std::vector<Uniform>& uniforms() { return _uniforms; }
        const std::vector<Uniform>& uniforms() const { return _uniforms; }

    public:
        virtual Config getConfig() const;

    private:
        void fromConfig(const Config& conf);

    private:
        std::string _code;
        std::vector<Sampler> _samplers;
        std::vector<Uniform>  _uniforms;
    };

    //! Encapsulates a shader attached to a Layer (or other object).
    /* internal */ class LayerShader : public osg::Referenced
    {
    public:
        LayerShader(const ShaderOptions& options);

        void install(Layer* layer, TerrainResources* res);

    protected:
        virtual ~LayerShader();

    private:
        const ShaderOptions _options;
        std::vector<TextureImageUnitReservation> _reservations;
    };

} // namespace osgEarth
OSGEARTH_SPECIALIZE_CONFIG(osgEarth::ShaderOptions);

#endif // OSGEARTH_LAYER_SHADER_H

