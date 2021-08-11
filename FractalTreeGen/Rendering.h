#pragma once

#include "glm/glm/glm.hpp"
#include "GL/glew.h"

namespace Rendering
{

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 norm;
		glm::vec3 color;
	};

	void createBuffers(GLuint& vao, GLuint& vbo, GLuint& ebo);
	void setupVertexBuffer(void* verts, size_t vertSize, void* indices, size_t indSize, GLuint vao, GLuint vbo, GLuint ebo);
	void draw(GLuint vao, size_t indSize);

	GLuint createShader(const char* fileName, GLenum shaderType);
	void createShaderProgram(GLuint& frag, GLuint& vert, GLuint& program);

};

