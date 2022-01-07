#include "ShaderSource.h"
#include <SDL.h>

ShaderSource::ShaderSource(const char* buf): src(buf)
{
    // don't delete the buf, let it bee
    queryCaps();
}

const char* ShaderSource::type()
{
    return "SHADER_SOURCE";
}

void ShaderSource::debugPrint()
{
    SDL_Log("SHADER_SOURCE[%d] caps:\n\t", id);
    std::string strCaps;
    if (caps & HAS_UNLIT) strCaps += "HAS_UNLIT ";
    if (caps & HAS_AMBIENT) strCaps += "HAS_AMBIENT ";
    if (caps & HAS_DIRECTIONAL_LIGHT) strCaps += "HAS_DIRECTIONAL_LIGHT ";
    if (caps & HAS_POINT_LIGHT) strCaps += "HAS_POINT_LIGHT ";
    if (caps & HAS_SPOT_LIGHT) strCaps += "HAS_SPOT_LIGHT ";
    if (caps & HAS_EMISSION) strCaps += "HAS_EMISSION ";
    if (caps & HAS_ALPHA_CLIP) strCaps += "HAS_ALPHA_CLIP ";
    if (caps & HAS_ALPHA_BLEND) strCaps += "HAS_ALPHA_BLEND ";
    if (caps & HAS_SKINNED) strCaps += "HAS_SKINNED ";
    if (caps & HAS_BILLBOARD) strCaps += "HAS_BILLBOARD ";
    strCaps += "\n";
    SDL_Log("%s", strCaps.c_str());
}

void ShaderSource::queryCaps()
{
    // from the source, check every possible capabilities
    caps = 0;   // no variant

    if (src.find("HAS_UNLIT") != std::string::npos) caps |= HAS_UNLIT;
    if (src.find("HAS_AMBIENT") != std::string::npos) caps |= HAS_AMBIENT;
    if (src.find("HAS_DIRECTIONAL_LIGHT") != std::string::npos) caps |= HAS_DIRECTIONAL_LIGHT;
    if (src.find("HAS_POINT_LIGHT") != std::string::npos) caps |= HAS_POINT_LIGHT;
    if (src.find("HAS_SPOT_LIGHT") != std::string::npos) caps |= HAS_SPOT_LIGHT;
    if (src.find("HAS_EMISSION") != std::string::npos) caps |= HAS_EMISSION;
    if (src.find("HAS_ALPHA_CLIP") != std::string::npos) caps |= HAS_ALPHA_CLIP;
    if (src.find("HAS_ALPHA_BLEND") != std::string::npos) caps |= HAS_ALPHA_BLEND;
    if (src.find("HAS_SKINNED") != std::string::npos) caps |= HAS_SKINNED;
    if (src.find("HAS_BILLBOARD") != std::string::npos) caps |= HAS_BILLBOARD;
}
//---------------------------------------------------------------------------------------------
size_t ShaderKey::computeHash() const
{
    // compute the hash, using a specified algorithm
    // by matching the shader source caps with our key flags
    const int light_caps_bits[] = {
        HAS_UNLIT,
        HAS_AMBIENT,
        HAS_DIRECTIONAL_LIGHT,
        HAS_POINT_LIGHT,
        HAS_SPOT_LIGHT,
        HAS_EMISSION
    };
    const int light_caps_shifts[] = {
        0, 1, 2, 3, 4, 5
    };

    const int opacity_caps_bits[] = {
        0,
        HAS_ALPHA_CLIP,
        HAS_ALPHA_BLEND
    };
    const int opacity_caps_shifts[] = {
        0, 6, 7
    };

    const int geom_caps_bits[] = {
        0,
        HAS_SKINNED,
        HAS_BILLBOARD
    };
    const int geom_caps_shifts[] = {
        0, 8, 9
    };

    int light_type_val = light_type * ( (source->caps & light_caps_bits[light_type]) >> light_caps_shifts[light_type]);
    int opacity_type_val = opacity_type * ( (source->caps & opacity_caps_bits[opacity_type]) >> opacity_caps_shifts[opacity_type]);
    int geom_type_val = geom_type * ( (source->caps & geom_caps_bits[geom_type]) >> geom_caps_shifts[geom_type]);

    return size_t( (source->id << 7) | (light_type_val << 4) | (opacity_type_val << 2) | (geom_type_val) );
}
