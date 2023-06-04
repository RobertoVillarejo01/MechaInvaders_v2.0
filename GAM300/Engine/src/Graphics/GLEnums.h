#pragma once

namespace GFX
{
	// General enums
	enum class PolygonMode_t {
		Solid, Wireframe, PointCloud
	};

	enum class Shape_t {
		Triangles, Line, Point
	};

	enum class Shader_t {
		Basic, Depth, CubeDepth, Cubemap, Lighting, ToGBuffer, DeferredLighting, 
		ScreenQuad, Blur, Blend, Particle, Text, None
	};

	// Texture units
	enum class TextureUnit {
		None, Diffuse, Normal, Specular, Shadow
	};

	// Lighting
	constexpr unsigned MAX_LIGHTS = 40;
	constexpr unsigned MAX_MATERIALS = 300;
	constexpr unsigned MAX_SHADOW_LIGHTS = 3;
	constexpr unsigned CUBE_FACES = 6;
	enum class LConfig {
		Diffuse, LightComponent, LightComponentShadows
	};

	// General uniforms
	enum class UniformLoc : unsigned {
		Model2ProjMtx, 
		Color, 
		UseTextures, 
		DiffuseSampler,
		Model2WorldMtx,
		Model2ViewMtx,
		M2VNormalsMtx,
		ShadowMapSampler,
		ShadowCubeSampler = ShadowMapSampler + MAX_SHADOW_LIGHTS,
		bEmitter = ShadowCubeSampler + MAX_SHADOW_LIGHTS
	};

	// Depthcube uniforms
	enum class DepthCubeLoc {
		Model2WorldMtx,
		LightPosition,
		FarPlane,
		ShadowMatrices
	};

	// Cubemap uniforms
	enum class CubemapLoc {
		Model2WorldMtx,
		CubemapShader
	};

	// Blur uniforms
	enum class BlurLoc {
		TextureSampler,
		Horizontal,
		Iterations
	};

	// UBOs
	enum class UBOBindPoint {
		Lights = 0,
		LightTransforms = 1,
		Materials = 2
	};
}
