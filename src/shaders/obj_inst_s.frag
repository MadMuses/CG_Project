#version 330 core
out vec3 finalColor;

in vec3 worldPosition;
in vec3 worldNormal;	// normalised in vertex shader

uniform vec3 lightPosition;
uniform vec3 lightIntensity;

// Color related
uniform vec4  baseColorFactor;
uniform float metallicFactor;
uniform float roughnessFactor;

// Texture related
in vec2 textureUV;
uniform sampler2D textureSampler;
uniform float validTexture;			// If valid texture is 0.0f, there is no texture, if not there is one


void main()
{
	vec3 color;
	if (validTexture == 1.0f){
		color = texture(textureSampler,textureUV).rgb;
	}else{
		color = vec3(baseColorFactor[0], baseColorFactor[1], baseColorFactor[2]); 		// Color (calculated from RGBa)
	}

	// Lighting
	vec3  lightDir  = lightPosition - worldPosition;
	float lightDist = dot(lightDir, lightDir);

	float cosTheta  = abs(dot(normalize(lightDir), worldNormal));
	vec3 light = lightIntensity / (4*3.14*lightDist);

	vec3 v = (1.14/3.14)* color * light * cosTheta;

	// Tone mapping
	v = v / (1.0 + v);

	// Gamma correction
	v = pow(v, vec3(1.0 / 2.2));

	// Gamma correction
	finalColor = v ;
}
