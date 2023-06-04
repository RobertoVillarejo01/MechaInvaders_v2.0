#version 440 core

layout(location = 0) in vec3 attr_position;
layout(location = 1) in vec3 attr_normal;
layout(location = 2) in vec2 attr_uvs;
layout(location = 3) in vec3 attr_tangents;
layout(location = 4) in vec3 attr_bitangents;

layout(location = 0) uniform mat4 uniform_m2p;
layout(location = 4) uniform mat4 uniform_m2w;
layout(location = 5) uniform mat4 uniform_m2v;
layout(location = 6) uniform mat4 uniform_normals_m2v;

const int MAX_LIGHTS        = 40;
const int MAX_SHADOW_LIGHTS = 3;
layout (std140, binding = 1) uniform LightTransforms
{
  mat4 light_w2p[MAX_SHADOW_LIGHTS];
  int numLightTransforms;
}; 


// TODO: Probably need a out of a array of vec4 for light space frag pos

out vec2 mUVs;
out vec3 mNormal;
out vec4 mTangent;
out vec4 mBitangent;
out vec3 mFragPos;
out vec3 mFragPosWorld;
out vec4 mLightSpaceFragPos[MAX_SHADOW_LIGHTS];

void main()
{
  mUVs = attr_uvs;
  gl_Position = uniform_m2p * vec4(attr_position, 1.0f);
    
	mNormal     = normalize(mat3(uniform_normals_m2v) * attr_normal);
  mTangent    = normalize(uniform_m2v * vec4(attr_tangents, 0));
  mBitangent  = normalize(uniform_m2v * vec4(attr_bitangents, 0));

	mFragPos = vec3(uniform_m2v * vec4(attr_position, 1));
  mFragPosWorld = vec3(uniform_m2w * vec4(attr_position, 1));

  for (int i = 0; i < numLightTransforms; ++i) {
    mLightSpaceFragPos[i]  = light_w2p[i] * uniform_m2w * vec4(attr_position, 1);
  }
}
