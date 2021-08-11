#include "Rendering.h"
#include "ShaderUtil.h"
#include <iostream>

void Rendering::createBuffers(GLuint& vao, GLuint& vbo, GLuint& ebo)
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
}

void Rendering::setupVertexBuffer(void* verts, size_t vertSize, void* indices, size_t indSize, GLuint vao, GLuint vbo, GLuint ebo)
{
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertSize * sizeof(Vertex), verts, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize * sizeof(GLuint), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, norm));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

void Rendering::draw(GLuint vao, size_t indSize)
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indSize, GL_UNSIGNED_INT, 0);
}

GLuint Rendering::createShader(const char* fileName, GLenum shaderType)
{
	GLchar* shaderCode = nullptr;
	GLint len = 0;
	LoadShader(fileName, shaderCode, len);

	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderCode, NULL);
	glCompileShader(shader);

	GLint success = GL_FALSE;
	char infoLog[512];

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		GLint logSize = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
		glGetShaderInfoLog(shader, 512, &logSize, infoLog);

		std::cout << "[ERR] Shader failed to compile: " << fileName << std::endl;
		std::cout << "::: " << infoLog << std::endl;
	}

	delete[] shaderCode;

	return shader;
}

void Rendering::createShaderProgram(GLuint& frag, GLuint& vert, GLuint& program)
{
	frag = createShader("Shaders/frag.frag", GL_FRAGMENT_SHADER);
	vert = createShader("Shaders/vert.vert", GL_VERTEX_SHADER);
	program = glCreateProgram();

	glAttachShader(program, frag);
	glAttachShader(program, vert);
	glLinkProgram(program);
}