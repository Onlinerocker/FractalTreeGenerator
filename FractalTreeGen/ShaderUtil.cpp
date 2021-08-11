//taken from https://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/loading.php

#include "ShaderUtil.h"
#include <fstream>

GLint getFileLength(std::ifstream& file)
{
    if (!file.good()) return 0;

    file.seekg(0, std::ios::end);
    unsigned long len = file.tellg();
    file.seekg(std::ios::beg);

    return (GLint)len;
}

int LoadShader(const char* filename, GLchar*& ShaderSource, GLint& len)
{
    std::ifstream file;
    file.open(filename, std::ios::in); // opens as ASCII!
    if (!file) return -1;

    len = getFileLength(file);

    if (len == 0) return -2;   // Error: Empty File 

    ShaderSource = new char[len + (GLint)1];
    if (ShaderSource == 0) return -3;   // can't reserve memory

     // len isn't always strlen cause some characters are stripped in ascii read...
     // it is important to 0-terminate the real length later, len is just max possible value... 
    ShaderSource[len] = 0;

    unsigned int i = 0;
    while (file.good())
    {
        ShaderSource[i] = file.get();       // get character from file.
        if (!file.eof())
            i++;
    }

    ShaderSource[i] = 0;  // 0-terminate it at the correct position

    file.close();

    return 0; // No Error
}


int UnloadShader(GLubyte** ShaderSource)
{
    if (*ShaderSource != 0)
        delete[] * ShaderSource;
    *ShaderSource = 0;
    return 1;
}