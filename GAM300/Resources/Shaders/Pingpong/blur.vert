#version 440 core

// Attributes
layout(location = 0) in vec3 attr_position;
layout(location = 2) in vec2 attr_uvs;

out vec2 TexCoords;

void main()
{
  TexCoords = attr_uvs;
  gl_Position = vec4(attr_position, 1.0f);
}