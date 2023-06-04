#version 440 core

layout(location = 1) uniform samplerCube cubemap;

in vec3 direction;

// Fragment color (the actual pixel)
out vec4 out_color;

void main()
{
  out_color = texture(cubemap, direction);
}
