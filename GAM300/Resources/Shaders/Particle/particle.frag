#version 440 core

in vec2 TextureCoord;
in vec4 VtxColor;

out vec4 FragColor;

layout(location = 1) uniform sampler2D ourTexture;

void main()
{
  FragColor = texture(ourTexture, TextureCoord) * VtxColor;
  if(FragColor.a == 0.0)
  	discard;
}