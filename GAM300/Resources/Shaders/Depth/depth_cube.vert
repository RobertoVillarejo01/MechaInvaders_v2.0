#version 440 core

// Attributes
layout(location = 0) in vec3 attr_position;

// Uniforms
layout(location = 0) uniform mat4 uniform_m2w;

void main()
{
  gl_Position = uniform_m2w * vec4(attr_position, 1.0f);
}