#version 440 core

// Attributes
layout(location = 0) in vec3 attr_position;

// Uniforms
layout(location = 0) uniform mat4 uniform_mvp;

out vec3 direction;

void main()
{
  vec4 frag_pos = uniform_mvp * vec4(attr_position, 1.0f);
  gl_Position = vec4(frag_pos.xyww);
  direction = attr_position;
} 