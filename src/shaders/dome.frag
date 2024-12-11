#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;	// normalised in vertex shader

out vec3 finalColor;

uniform vec3 lightPosition;
uniform vec3 lightIntensity;

uniform vec4  baseColorFactor;
uniform float metallicFactor;
uniform float roughnessFactor;

void main()
{
	// Color (calculated from RGBa)
	vec3 color = vec3(
			baseColorFactor[0],
			baseColorFactor[1],
			baseColorFactor[2]
	);

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
	finalColor = v;
}
