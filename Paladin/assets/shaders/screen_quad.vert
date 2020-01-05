#version 410 core

out vec2 uv;

uniform float bufferWindowRatio = 1;

void main()
{
	float x = float(((uint(gl_VertexID) + 2u) / 3u) % 2u);
	float y = float(((uint(gl_VertexID) + 1u) / 3u) % 2u);
	
	gl_Position = vec4(-bufferWindowRatio + x * bufferWindowRatio * 2.0f, 1.0f - y * 2.0f, 0.0f, 1.0f);
	
	uv = vec2(x, y);
}
