#version 330 core

layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNorm;
layout (location=2) in vec3 aColor;
layout (location=3) in vec2 aUV;

out vec2 UV;
out vec3 Norm;
out vec3 Color;
out vec3 Pos;

uniform mat4 transform;
uniform mat4 rot;

void main() {
	gl_Position = transform * vec4(aPos.xyz, 1.0);
	vec4 n = rot * vec4(aNorm, 0.0);
	
	UV = aUV;
	Norm = n.xyz;
	Color = aColor;
	Pos = vec3(gl_Position.xyz);
}