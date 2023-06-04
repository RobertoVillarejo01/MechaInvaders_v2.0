#version 440 core

in vec2 TextureCoord;

out vec4 FragColor;

layout(location = 2) uniform sampler2D  ourTexture;
layout(location = 3) uniform vec4       VtxColor;

void main()
{
  FragColor = VtxColor;
  if(texture(ourTexture, TextureCoord).r == 0.0 || FragColor.a == 0.0)
  	discard;
}