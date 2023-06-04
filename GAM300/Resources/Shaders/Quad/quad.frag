#version 440 core

layout(location = 0) uniform sampler2D screen_texture;

in  vec2 TextureUV;
out vec4 out_color;

const float gamma = 2.2;
const float exposure = 0.1;

void main()
{
  // Get the color we were going to output and check if the alpha is 0
  out_color = texture(screen_texture, TextureUV);
  //out_color = vec4(1,0,0,1);
  if (out_color.a <= 0.0) 
    discard;
  //out_color.a = 1.0f;
    
  // // If it is a valid color, apply tone mapping and gamma correction
  // vec3 hdr = out_color.rgb;
  // //vec3 tone_mapped = hdr / (hdr + vec3(1.0));
  // vec3 tone_mapped = vec3(1.0) - exp(-hdr * exposure);
  // tone_mapped = pow(tone_mapped, vec3(1.0 / gamma));
  // 
  // // We are done
  // out_color = vec4(tone_mapped, out_color.a);
}
