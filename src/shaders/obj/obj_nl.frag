#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;	// normalised in vertex shader

// Texture related
in vec2 textureUV;

out vec3 finalColor;

uniform vec3 lightPosition;
uniform vec3 lightIntensity;

uniform vec4  baseColorFactor;
uniform float metallicFactor;
uniform float roughnessFactor;

uniform sampler2D textureSampler;
uniform float validTexture;			// If valid texture is 0.0f, there is no texture, if not there is one

void main()
{
    vec3 color;
    if (validTexture == 1.0f){
        color = texture(textureSampler,textureUV).rgb;
    }else{
        // Color (calculated from RGBa)
        color = vec3(baseColorFactor[0], baseColorFactor[1], baseColorFactor[2]);
    }

    // Gamma correction
    finalColor = color;
}