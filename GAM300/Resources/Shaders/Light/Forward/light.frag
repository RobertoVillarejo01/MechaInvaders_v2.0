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
const vec3 CamPos = vec3(0, 0, 0);
const float bias = 0.02;
const float brightness = 1.0;

float ShadowFactor;

const int MAX_LIGHTS        = 40;
const int MAX_MATERIALS     = 256;
const int MAX_SHADOW_LIGHTS = 3;
const int POINT_LIGHT       = 0;
const int DIRECTIONAL_LIGHT = 1;
const int SPOT_LIGHT        = 2;

// Uniforms
layout(location = 1) uniform vec4          uniform_color;
layout(location = 2) uniform bool          use_textures;
layout(location = 3) uniform sampler2D     texture_diffuse;
layout(location = 7) uniform sampler2D     shadow_maps[MAX_SHADOW_LIGHTS];
layout(location = 10) uniform samplerCube  shadow_cubes[MAX_SHADOW_LIGHTS];
layout(location = 13) uniform bool         im_emitter;
layout(location = 14) uniform int          matID;

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
MaterialClass material;

in  vec2 mUVs;
in  vec3 mNormal;
in  vec3 mFragPos;
in  vec3 mFragPosWorld;
in  vec4 mLightSpaceFragPos[MAX_SHADOW_LIGHTS];

// Rendering the scene into 2 different targets so we can have the bloom effect
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BloomColor;

vec3 PointLight(Light _light)
{
  // Diffuse
  vec3 N = normalize(mNormal);
  vec3 L = normalize(_light.position - mFragPos);
	
  // Specular
  vec3 V = normalize(CamPos - mFragPos);
  vec3 R = 2 * dot(N, L) * N - L; 
  float specFactor = pow ( max( dot(R, V), 0 ), material.shininess);

  // Different light totals
  vec3 ambientTotal = material.ambient * _light.ambient;
  vec3 diffuseTotal = material.diffuse * _light.diffuse * brightness * max( dot(N, L), 0 );
  vec3 specularTotal = material.specular * _light.specular * specFactor;
	
	// Point Light's (Attenuation)
	float dist = length(_light.position - mFragPos);
	float attenuationFactor = min(1.0, 1.0 / ( _light.constAttenuation + 
    _light.linearAttenuation * dist + _light.quadrAttenuation * 0.2 * dist * dist));

  // Result / Addition
  return (ambientTotal + (diffuseTotal + specularTotal) * ShadowFactor) * attenuationFactor;
}

vec3 DirectionalLight(Light _light)
{
  // Diffuse
  vec3 N = normalize(mNormal);
  vec3 L = normalize(-_light.direction);					// Directional light
	
  // Specular
  vec3 V = normalize(CamPos - mFragPos);
  vec3 R = 2 * dot(N, L) * N - L; 
  float specFactor = pow ( max( dot(R, V), 0 ), material.shininess);

  // Different light totals
  vec3 ambientTotal = material.ambient * _light.ambient;
  vec3 diffuseTotal = material.diffuse * _light.diffuse * brightness * max( dot(N, L), 0 );
  vec3 specularTotal = material.specular * _light.specular * specFactor;

  // Result / Addition
  return ambientTotal + (diffuseTotal + specularTotal) * ShadowFactor;
}

vec3 SpotLight(Light _light)
{
  // Diffuse
  vec3 N = normalize(mNormal);
  vec3 L = normalize(_light.position - mFragPos);			// Point Light
  //vec3 L = normalize(-light.direction);					// Directional light
	
  // Specular
  vec3 V = normalize(CamPos - mFragPos);
  vec3 R = 2 * dot(N, L) * N - L; 
  float specFactor = pow ( max( dot(R, V), 0 ), material.shininess);
	
	// Point Light's (Attenuation)
	float dist = length(_light.position - mFragPos);
	float attenuationFactor = min(1.0, 1.0 / ( _light.constAttenuation + 
    _light.linearAttenuation * dist + _light.quadrAttenuation * dist * dist ));

	// Spot Light
	vec3 D = normalize(_light.direction);
	float SpotFactor;
	if 		  ( dot(-L, D) < cos(radians(_light.spotOuterAngle)) )  SpotFactor = 0.0f;
	else if ( dot(-L, D) > cos(radians(_light.spotInnerAngle)) ) 	SpotFactor = 1.0f;
	else {
		SpotFactor = pow( (dot(-L, D) - cos(radians(_light.spotOuterAngle))) / (cos(radians(_light.spotInnerAngle)) 
      - cos(radians(_light.spotOuterAngle))) , _light.spotExponent );	
		SpotFactor = clamp(SpotFactor, 0.0f, 1.0f);
	}

  // Different light totals
  vec3 ambientTotal = material.ambient * _light.ambient;
  vec3 diffuseTotal = material.diffuse * _light.diffuse * brightness * max( dot(N, L), 0 );
  vec3 specularTotal = material.specular * _light.specular * specFactor;

  // Result / Addition
  return (ambientTotal + (diffuseTotal + specularTotal) * SpotFactor * ShadowFactor) * attenuationFactor;
}

vec3 ComputeLight(Light _light)
{
  switch (_light.light_type)
  {
    case POINT_LIGHT:
      return PointLight(_light);
      break;
    case DIRECTIONAL_LIGHT:
      return DirectionalLight(_light);
      break;
    case SPOT_LIGHT:
      return SpotLight(_light);
      break;
  }
}

void ComputeShadowFactor(Light _light, int _Samples)
{
  // Check if the current fragment is in shadow or not, in order to check so, we need
  // the coordinates of the fragment in light space, already in NDC. We use the Z to 
  // check with the Light Depth Buffer (our shadow map) and the xy as uvs to extract 
  ShadowFactor = 0.0;
  
  // the actual value from the shadow map
  vec4 FragLightNDC = mLightSpaceFragPos[_light.texture_unit] / mLightSpaceFragPos[_light.texture_unit].w;
  
  // Change from range [-1, 1] to [0, 1]
  vec3 ShadowMapUVs = vec3(FragLightNDC) * 0.5 + 0.5; 
  
  // getPCFShadow
  vec2 texelOffset = 1.0 / textureSize(shadow_maps[_light.texture_unit], 0);
  int neighbor = _Samples; // For a 5x5 neighborhood
  
  int sampleCount = 0;
  float accumulatedVisibility = 0.0;
  
  for(int x = -neighbor; x <= neighbor; x++)
  {
    for(int y = -neighbor; y <= neighbor; y++)
    {
      float shadowDepth = texture(shadow_maps[_light.texture_unit], ShadowMapUVs.xy + texelOffset * vec2(x, y)).r;
      
      // Evaluate visibility for this sample
      if (ShadowMapUVs.z - bias < shadowDepth || ShadowMapUVs.z > 1.0)
      {
        accumulatedVisibility += 1.0f;
      }
      sampleCount++;
    }
  }
  
  ShadowFactor = accumulatedVisibility / sampleCount;
}

vec3 gridSamplingDisk[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
); 

void ComputeCubeShadowFactor(Light _light, int _Samples)
{
  ShadowFactor = 0.0; 
  int samples = 20;
  vec3 fragToLight = mFragPosWorld - _light.world_position;  
  float currentDepth = length(fragToLight);
  float viewDistance = length(mFragPos);
  float diskRadius = (1.0 + (viewDistance / _light.far_plane)) / 10.0;
  for(int i = 0; i < samples; ++i)
  {
    float closestDepth = texture(shadow_cubes[_light.texture_unit], fragToLight + gridSamplingDisk[i] * diskRadius).r;
    closestDepth *= _light.far_plane;   // undo mapping [0;1]
    if(currentDepth - bias > closestDepth)
        ShadowFactor += 1.0;
  }
  ShadowFactor /= float(samples);
  ShadowFactor = 1 - ShadowFactor;
}

subroutine void LightingType();

layout(index = 0) subroutine (LightingType)
void OnlyDiffuse()
{
  FragColor = vec4(FragColor.rgb * material.diffuse, FragColor.a);
}
layout(index = 1) subroutine (LightingType)
void UsingLights()
{
  ShadowFactor = 1.0;
  vec3 total = vec3(0,0,0);
  for (int i = 0; i < numLights; ++i)
  {
    total += FragColor.rgb * ComputeLight(mLights[i]);
  }
  FragColor = vec4(total / numLights, FragColor.a);
}
layout(index = 2) subroutine (LightingType)
void LightsAndShadows()
{
  vec3 total_lights_and_shadows = vec3(0,0,0);
  int  computed_lights = 0;
  for (int i = 0; i < numLights; ++i)
  {
    // Check if the light is using shadows
    if (mLights[i].texture_unit != -1) 
    {
      if (mLights[i].light_type == POINT_LIGHT) {
        ComputeCubeShadowFactor(mLights[i], 3);
      }
      else {
        ComputeShadowFactor(mLights[i], 3);
      }
    }
    else {
      ShadowFactor = 1.0f;
    } 

    total_lights_and_shadows += ComputeLight(mLights[i]);
    computed_lights++;
  }
  total_lights_and_shadows /=  computed_lights;
  FragColor = vec4(FragColor.rgb * total_lights_and_shadows, FragColor.a);
}

layout(location = 0) subroutine uniform LightingType LightingFunction;


//in  vec2 mUVs;
//in  vec3 mNormal;
//in  vec3 mFragPos;
//in  vec3 mFragPosWorld;

void main()
{
  if (use_textures) 
    FragColor = texture(texture_diffuse, mUVs);
  else
    FragColor = uniform_color;

  if (FragColor.a <= 0.0)
    discard;
  
  // Changes FragColor
  material = mMaterials[matID];
  LightingFunction();
  //FragColor = vec4(mNormal, 1);

  if (im_emitter) {
    FragColor = vec4(FragColor.rgb + 20.0, FragColor.a);
  }

  // Check if the "brightness" of the fragment surpasses a certain threshold
  //float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
  //if(brightness > 1.0)
  //  BloomColor = vec4(FragColor.rgb, 1.0);
  //else
    BloomColor = vec4(0.0, 0.0, 0.0, 0.0);
}
