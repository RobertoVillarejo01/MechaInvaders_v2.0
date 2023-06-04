#version 440 core

// Attributes
layout(location = 0) in vec3 attr_position;

// Uniforms
layout(location = 0) uniform mat4 uniform_mvp;

void main()
{
  gl_Position = uniform_mvp * vec4(attr_position, 1.0f);
}