#version 440 core

in vec2 mUVs;
in vec3 mNormal;
in vec4 mTangent;
in vec4 mBitangent;
in vec3 mFragPos;
in vec3 mFragPosWorld;

layout(location = 0) out vec4 out_pos;
layout(location = 1) out vec4 out_normal;
layout(location = 2) out vec4 out_albedo;

layout(location = 1) uniform vec4          uniform_color;
layout(location = 2) uniform bool          use_textures;
layout(location = 3) uniform sampler2D     texture_diffuse;
layout(location = 7) uniform bool          use_normal_textures;
layout(location = 8) uniform sampler2D     texture_normals;
layout(location = 14) uniform int          materialID;

//uniform sampler2D NormalMap;
//uniform sampler2D Metalness;

// Maybe will need also the mFragPosWorld which we can include 
// combining .w of the 3 textures
void main()
{
  // Fragment position in proj space
  out_pos = vec4(mFragPos, 1);

  // Normal mapping
  if (use_normal_textures) 
  {
    vec3 normal = mNormal;
    mat3 TBN = mat3(vec3(mTangent), vec3(mBitangent), normal);
  	normal = texture(texture_normals, mUVs).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(TBN * normal);
    out_normal = vec4(normal, 1);
  }
  else 	
    out_normal = vec4(mNormal, 1);
  out_normal.a = float(materialID) / 256.0;//float(materialID);
  
  // Color / Albedo
  if (use_textures)
	  out_albedo = vec4(uniform_color.rgb, 1) * texture(texture_diffuse, mUVs);
  else 	
	  out_albedo = vec4(uniform_color.rgb,1);
} 
