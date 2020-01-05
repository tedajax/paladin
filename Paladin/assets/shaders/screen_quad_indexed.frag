#version 410 core

in vec2 uv;

uniform int mode = 0;
uniform sampler2D intensity;
uniform sampler2D palette;

out vec3 color;

void main()
{
	//color = texture(palette, texture(intensity, uv).rg).rgb;
	if (mode == 0)
	{
		vec2 index = texture(intensity, uv).rg;
		color = texture(palette, index).rgb;
	}
	else if (mode == 1)
	{
		color = texture(intensity, uv).rrr;
	}
	else if (mode == 2)
	{
		color = texture(palette, uv).rgb;
	}
	else
	{
		color = vec3(1, 0, 1);
	}
}