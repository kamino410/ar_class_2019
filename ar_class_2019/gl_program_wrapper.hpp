#pragma once

#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

enum class GLSLShaderType {
  VERTEX = GL_VERTEX_SHADER,
  FRAGMENT = GL_FRAGMENT_SHADER,
  GEOMETRY = GL_GEOMETRY_SHADER,
  TESS_CONTROL = GL_TESS_CONTROL_SHADER,
  TESS_EVALUATION = GL_TESS_EVALUATION_SHADER,
  COMPUTE = GL_COMPUTE_SHADER
};

class GLSLProgramWrapper {
 private:
  GLuint handle;

  std::string loadFileAsStr(const char *);

 public:
  GLSLProgramWrapper();
  ~GLSLProgramWrapper();

  /// compile and attach the shader to the program
  void compileShader(const char *, GLSLShaderType);

  /// link program
  void link();

  void bindAttribLocation(GLuint, const char *);
  GLint getAttribLocation(const char *);

  void setUniform(const char *, float, float, float);
  void setUniform(const char *, const glm::vec2 &);
  void setUniform(const char *, const glm::vec3 &);
  void setUniform(const char *, const glm::vec4 &);
  void setUniform(const char *, const glm::mat4 &);
  void setUniform(const char *, const glm::mat3 &);
  void setUniform(const char *, float);
  void setUniform(const char *, int);
  void setUniform(const char *, bool);
  void setUniform(const char *, GLuint);

  void enable();
};
