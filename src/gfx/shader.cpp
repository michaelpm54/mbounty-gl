#include "gfx/shader.hpp"

#include <spdlog/spdlog.h>

#include <fstream>

bool compile_shader(GLuint shader);
bool link_program(GLuint program);
std::string read_text(const std::string &path);

namespace bty {

GLuint load_shader(const std::string &vs_path, const std::string &fs_path)
{
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    std::string shader_sources[2] =
        {
            read_text(vs_path),
            read_text(fs_path),
        };
    const char *vs_src = shader_sources[0].c_str();
    const char *fs_src = shader_sources[1].c_str();
    glShaderSource(vs, 1, &vs_src, nullptr);
    glShaderSource(fs, 1, &fs_src, nullptr);

    if (!compile_shader(vs)) {
        spdlog::error("Fragment shader compilation failed");
        return GL_NONE;
    }
    if (!compile_shader(fs)) {
        spdlog::error("Vertex shader compilation failed");
        glDeleteShader(vs);
        return GL_NONE;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    if (!link_program(program)) {
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

bool compile_shader(GLuint shader)
{
    glCompileShader(shader);
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint log_size = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
        GLint max_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);
        if (log_size == 0 || max_length == 0) {
            spdlog::error("Failed to get shader info log.\nMake sure your OpenGL functions are working correctly.");
            return false;
        }
        std::vector<GLchar> error_log(log_size);
        glGetShaderInfoLog(shader, max_length, &max_length, &error_log[0]);
        glDeleteShader(shader);
        spdlog::error("{}", std::string(error_log.begin(), error_log.end()));
        return false;
    }

    return true;
}

bool link_program(GLuint program)
{
    glLinkProgram(program);
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint log_size = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);
        GLint max_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);
        if (log_size == 0 || max_length == 0) {
            spdlog::error("Failed to get program info log.\nMake sure your OpenGL functions are working correctly.");
            return false;
        }
        std::vector<GLchar> error_log(max_length);
        glGetProgramInfoLog(program, max_length, &max_length, &error_log[0]);
        glDeleteProgram(program);
        spdlog::error("{}", std::string(error_log.begin(), error_log.end()));
        return false;
    }

    return true;
}

std::string read_text(const std::string &path)
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
