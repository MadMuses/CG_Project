#version 330 core

// Input
in vec3 color;
in vec2 uv;   // TODO: To add UV input to this fragment shader
in float i;

// TODO: To add the texture sampler
uniform sampler2D textureSampler0;
uniform sampler2D textureSampler1;
uniform sampler2D textureSampler2;
uniform sampler2D textureSampler3;
uniform sampler2D textureSampler4;
uniform sampler2D textureSampler5;

// Output
out vec3 finalColor;

void main()
{
	vec3 color;
	if		(i == 2.0)	{color = texture(textureSampler0, uv).rgb;}		// Front
	else if (i == 3.0) 	{color = texture(textureSampler1, uv).rgb; }	// Back
	else if (i == 0.0) 	{color = texture(textureSampler2, uv).rgb; }	// Right
	else if (i == 1.0) 	{color = texture(textureSampler3, uv).rgb; }	// Left
	else if (i == 5.0) 	{color = texture(textureSampler4, uv).rgb; }	// Down
	else if (i == 4.0) 	{color = texture(textureSampler5, uv).rgb; }	// Up

	finalColor = color;
}
