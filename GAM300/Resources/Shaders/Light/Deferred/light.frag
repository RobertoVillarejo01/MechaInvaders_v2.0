#version 440 core

// Structs
struct MaterialClass
{
  vec3  ambient;
  float shininess;

  vec3  diffuse;
  vec3  specular;
};
struct Light
{
  // Basic properties 
  int   light_type;
  vec3  position;
  int   texture_unit;
  vec3  direction; 
  float far_plane;
  
  // Light intensities
  vec3  ambient;
  vec3  diffuse;
  vec3  specular;
  
  // Spot Light / Flashlight
  float spotExponent;
  float spotOuterAngle;
  float spotInnerAngle;
  
  // Attenuation / Point Light
  float constAttenuation;
  float linearAttenuation;
  vec3  world_position;
  float quadrAttenuation;
};

// DATA
const float PI = 3.141592;
const vec3  CamPos = vec3(0, 0, 0);
const float bias = 0.02;
const float brightness = 1.0;
float ShadowFactor = 1.0;
int   LightsInRange = 0;

const int MAX_LIGHTS        = 40;
const int MAX_MATERIALS     = 256;
const int MAX_SHADOW_LIGHTS = 3;
const int POINT_LIGHT       = 0;
const int DIRECTIONAL_LIGHT = 1;
const int SPOT_LIGHT        = 2;

// GBUFFER
layout(location = 0) uniform sampler2D gbuffer_positions;
layout(location = 1) uniform sampler2D gbuffer_normals;
layout(location = 2) uniform sampler2D gbuffer_albedo;

// Lights
layout (std140, binding = 0) uniform Lights
{
  Light mLights[MAX_LIGHTS];
  int numLights;
}; 
layout (std140, binding = 2) uniform Materials
{
  MaterialClass mMaterials[MAX_MATERIALS];
  int numMaterials;
}; 

// Data from the quad
in vec2 TextureUV;
in vec3 Normal;



// FUNCTIONS
vec3 PointLight(Light _light, MaterialClass _material)
{
  // Data from GBuffer
  vec3 mNormal    = texture(gbuffer_normals, TextureUV).rgb;
  vec3 mFragPos   = texture(gbuffer_positions, TextureUV).rgb;

  // Diffuse
  vec3 N = normalize(mNormal);
  vec3 L = normalize(_light.position - mFragPos);
	
  // Specular
  vec3 V = normalize(CamPos - mFragPos);
  vec3 R = 2 * dot(N, L) * N - L; 
  float specFactor = pow ( max( dot(R, V), 0 ), _material.shininess);

  // Different light totals
  vec3 ambientTotal = _material.ambient * _light.ambient;
  vec3 diffuseTotal = _material.diffuse * _light.diffuse * brightness * max( dot(N, L), 0 );
  vec3 specularTotal = _material.specular * _light.specular * specFactor;
	
	// Point Light's (Attenuation)
	float dist = length(_light.position - mFragPos);
	float attenuationFactor = min(1.0, 1.0 / ( 0.5 + 
    0.001 * dist));
  if (attenuationFactor > 0.01) { LightsInRange += 1; }

  // Result / Addition
  return (ambientTotal + (diffuseTotal + specularTotal) * ShadowFactor) * attenuationFactor;
}

vec3 ComputeLight(Light _light, MaterialClass _material)
{
  switch (_light.light_type)
  {
    case POINT_LIGHT:
      return PointLight(_light, _material);
      break;
  }
}


out vec4 FragColor;
void main()
{
  // Early out = No albedo alpha (should always be = 1)
  if (texture(gbuffer_albedo, TextureUV).a == 0) 
    discard;

  // Extract the material info
  int matID = int(256 * texture(gbuffer_normals, TextureUV).a);

  // Compute with the different lights
  vec3 TotalLight = vec3(0.2,0.2,0.2);
  LightsInRange = 0;
  for (int i = 0; i < numLights; ++i)
  {
    TotalLight += ComputeLight(mLights[i], mMaterials[matID]);
  }

  LightsInRange = max(1, LightsInRange);
  TotalLight /= LightsInRange;

  // Get the color from the albedo texture and then apply lighting
  vec4 color = FragColor = texture(gbuffer_albedo, TextureUV);
  FragColor = vec4(TotalLight * FragColor.rgb, FragColor.a);
//  FragColor = texture(gbuffer_normals, TextureUV);
}
