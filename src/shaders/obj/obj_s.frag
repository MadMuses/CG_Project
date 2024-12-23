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

// Shadow related
in vec4 projectedPosition;
uniform sampler2D depthTextureSampler;


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

	// Shadow calulations
	float shadow = 1.0f;
	if ((abs(projectedPosition.x) < projectedPosition.w) && (abs(projectedPosition.y) < projectedPosition.w)) {

		// We put the coordinates in the [-1,1] range (xyz/w) then pass it in the [0,1] range (*0.5+0.5)
		vec3 uv = (projectedPosition.xyz/projectedPosition.w)*0.5 + 0.5;

		// Calculate depth
		float depth = uv.z;

		// Use the xy coordinates from the light space position
		float existingDepth = texture(depthTextureSampler, uv.xy).r;
		shadow = (depth >= existingDepth + 7e-3) ? 0.2 : 1.0;
	}


	// Gamma correction
	finalColor = v * shadow;
}
