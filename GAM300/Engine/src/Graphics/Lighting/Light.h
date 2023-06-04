#pragma once

#include "Utilities/Math.h"
#include "Objects/Components.h"
#include "Graphics/Framebuffer/Framebuffer.h"
#include "Graphics/RenderManager/RenderManager.h"

namespace GFX {

	class LightCamera;

	struct Light
	{
		// DO NOT TOUCH THE ORDER OF THESE VARIABLES (VERY IMPORTANT, UBO) //
		enum class Type {
			POINT, DIRECTIONAL, SPOT
		};

		// Basic properties 
		int   light_type;
		glm::vec3	 direction;
		glm::vec3  position;
		int texture_unit;
		glm::vec3  actual_dir;
		float far;

		// Light intensities
		glm::vec3  ambient;
		float near;
		glm::vec3  diffuse;
		float projection_size;
		glm::vec3  specular;

		// Spot Light / Flashlight
		float spotExponent;
		float spotOuterAngle;
		float spotInnerAngle;

		// Attenuation / Point Light
		float constAttenuation;
		float linearAttenuation;
		glm::vec3 world_position;
		float quadrAttenuation;
	};

}

class LightComponent : public IComp
{
public:
	// Basic component things
	void Initialize() override;
	void Shutdown() override;
#ifdef EDITOR
	bool   Edit() override;
#endif
	IComp* Clone();

	// Matrices when the light is used as camera 
	const glm::mat4& GetWorld2Cam();
	const glm::mat4& GetCam2Proj();

	// Gettors
	const GFX::Framebuffer& GetFramebuffer() const;

protected:
	// Serialization
	void ToJson(nlohmann::json& j) const override;
	void FromJson(nlohmann::json& j) override;

private:
	GFX::Light				mLight{};
	GFX::Framebuffer  mShadowMap{};

	glm::mat4  mWorld2Cam{};
	glm::mat4  mCam2Proj{};

	bool mbShadowMapComputed = false;
	bool mbEmitShadow = false;

	friend class GFX::LightCamera;
	friend class GFX::RenderManager;
};