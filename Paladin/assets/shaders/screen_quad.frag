#version 410 core

in vec2 uv;

uniform sampler2D image;

out vec3 color;

void main()
{
	color = texture(image, uv).rgb;
}