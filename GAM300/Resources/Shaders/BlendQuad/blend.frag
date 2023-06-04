#version 440 core

out vec4 FragColor;

in vec2 TextureCoords;

layout(location = 0) uniform sampler2D scene;
layout(location = 1) uniform sampler2D bloom;

const float gamma = 2.2;
const float exposure = 0.1;

void main()
{
  // Get the color we were going to output and check if the alpha is 0
  FragColor = texture(scene, TextureCoords);
  vec4 hdr = FragColor + texture(bloom, TextureCoords);
    
  // If it is a valid color, apply tone mapping and gamma correction
  if (hdr.a == 0)
    discard;

  //vec3 tone_mapped = hdr / (hdr + vec3(1.0));
  vec3 tone_mapped = vec3(1.0) - exp(-hdr.rgb * exposure);
  tone_mapped = pow(tone_mapped, vec3(1.0 / gamma));
  
  // We are done
  FragColor = vec4(tone_mapped, hdr.a);
}