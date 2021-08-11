#version 330 core

out vec4 FragColor;

in vec3 Pos;
in vec3 Norm;
in vec3 Color;
in vec2 UV;

void main() {
	vec3 light = normalize(vec3(5, 0, -10) - Pos);
	float d = 0.0 + dot(Norm, light) * 1.0;

	d = clamp(d, 0.0, 1.0);
	FragColor = vec4(Color,1) * d * 1.5;
}