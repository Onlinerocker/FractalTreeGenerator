#pragma once
#include <GL\glew.h>
#include <fstream>

int LoadShader(const char* filename, GLchar*& ShaderSource, GLint& len);
int UnloadShader(GLubyte** ShaderSource);