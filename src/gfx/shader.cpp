#include "gfx/shader.hpp"

#include <spdlog/spdlog.h>

#include <fstream>

bool compileShader(GLuint shader);
bool linkProgram(GLuint program);
std::string readText(const std::string &path);

namespace bty {

GLuint loadShader(const std::string &vsPath, const std::string &fsPath)
{
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    std::string shader_sources[2] =
        {
            readText(vsPath),
            readText(fsPath),
        };
    const char *vsSrc = shader_sources[0].c_str();
    const char *fsSrc = shader_sources[1].c_str();
    glShaderSource(vs, 1, &vsSrc, nullptr);
    glShaderSource(fs, 1, &fsSrc, nullptr);

    if (!compileShader(vs)) {
        spdlog::error("Fragment shader compilation failed");
        return GL_NONE;
    }
    if (!compileShader(fs)) {
        spdlog::error("Vertex shader compilation failed");
        glDeleteShader(vs);
        return GL_NONE;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    if (!linkProgram(program)) {
        spdlog::error("Program link failed");
        glDetachShader(program, vs);
        glDetachShader(program, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);
        glDeleteProgram(program);
        return GL_NONE;
    }
    glDetachShader(program, vs);
    glDetachShader(program, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

}    // namespace bty

bool compileShader(GLuint shader)
{
    glCompileShader(shader);
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logSize = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        if (logSize == 0 || maxLength == 0) {
            spdlog::error("Failed to get shader info log.\nMake sure your OpenGL functions are working correctly.");
            return false;
        }
        std::vector<GLchar> errorLog(logSize);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
        glDeleteShader(shader);
        spdlog::error("{}", std::string(errorLog.begin(), errorLog.end()));
        return false;
    }

    return true;
}

bool linkProgram(GLuint program)
{
    glLinkProgram(program);
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logSize = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        if (logSize == 0 || maxLength == 0) {
            spdlog::error("Failed to get program info log.\nMake sure your OpenGL functions are working correctly.");
            return false;
        }
        std::vector<GLchar> errorLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &errorLog[0]);
        glDeleteProgram(program);
        spdlog::error("{}", std::string(errorLog.begin(), errorLog.end()));
        return false;
    }

    return true;
}

std::string readText(const std::string &path)
{
    auto stream {std::ifstream(path)};
    if (!stream.good()) {
        spdlog::error("Problem loading file {}", path);
        return {};
    }
    stream.seekg(0, std::ios::end);
    size_t size = stream.tellg();
    std::string buffer(size, ' ');
    stream.seekg(0);
    stream.read(buffer.data(), size);
    return buffer;
}
