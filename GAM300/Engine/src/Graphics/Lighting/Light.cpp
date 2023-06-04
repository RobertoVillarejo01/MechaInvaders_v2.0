#include "Light.h"
#include "System/Scene/SceneSystem.h"
#include "Graphics/Graphics.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR


static constexpr unsigned SHADOW_MAP_HEIGHT = 1024, SHADOW_MAP_WIDTH = 1024;

/**
 * Generate the shadow map that will be used for this light
 */
void LightComponent::Initialize()
{
	GFX::Framebuffer::TextureType fb_type = GFX::Framebuffer::TextureType::Texture2D;
	if (mLight.light_type == (uint32_t)GFX::Light::Type::POINT)
		fb_type = GFX::Framebuffer::TextureType::CubeMap;
	mShadowMap.GenerateDepthBuffer(fb_type,	{SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT});
}

/**
 * Clean the resources the shadow map needed
 */
void LightComponent::Shutdown()
{
	mShadowMap.Shutdown();
}

#ifdef EDITOR
bool LightComponent::Edit()
{
	// TODO: Properly set the type of light with a Combo or smth (maybe renderable mesh 
	// choosing should be fixed too)
	if (ImGui::SliderInt("Type", &mLight.light_type, 0, 2)) {
		if (mLight.light_type == (uint32_t)GFX::Light::Type::POINT) {
			mShadowMap.GenerateDepthBuffer(GFX::Framebuffer::TextureType::CubeMap,
				{ SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT });
		}
		else {
			mShadowMap.GenerateDepthBuffer(GFX::Framebuffer::TextureType::Texture2D,
				{ SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT });
		}
	}

	// Set the direction of the light.
	// At least for now the position of the light will be the same as the owner's
	// (The information about the position will be updated when uploading the light info)
	// Maybe later we want to have an offset
	ImGui::DragFloat3("Direction", &mLight.direction[0]);

	// Render the direction of the light
	Debug::DrawLine({ mOwner->mTransform.mPosition, mOwner->mTransform.mPosition + mLight.direction },
		{0.0f, 1.0f, 1.0f, 1.0f});

	Debug::DrawSphere({ mOwner->mTransform.mPosition, 3.0f }, { 1.0f, 1.0f, 0.0f, 1.0f });
	Debug::DrawLine({ mOwner->mTransform.mPosition, mOwner->mTransform.mPosition +
		glm::normalize(mLight.direction) * 4.0f }, { 0.0f, 0.0f, 0.0f, 1.0f });

	ImGui::DragFloat("Near", &mLight.near);
	ImGui::DragFloat("Far", &mLight.far);
	ImGui::DragFloat("Projection size", &mLight.projection_size);

	ImGui::ColorEdit3("Ambient", &mLight.ambient[0]);
	ImGui::ColorEdit3("Diffuse", &mLight.diffuse[0]);
	ImGui::ColorEdit3("Specular", &mLight.specular[0]);

	ImGui::DragFloat("spotExponent", &mLight.spotExponent, 0.5f);
	ImGui::SliderFloat("spotOuterAngle", &mLight.spotOuterAngle, 0.0f, 90.0f);
	ImGui::SliderFloat("spotInnerAngle", &mLight.spotInnerAngle, 0.0f, mLight.spotOuterAngle);
	mLight.spotInnerAngle = glm::min(mLight.spotInnerAngle, mLight.spotOuterAngle);

	ImGui::SliderFloat("constAttenuation", &mLight.constAttenuation, 0.0f, 1.0f);
	ImGui::SliderFloat("linearAttenuation", &mLight.linearAttenuation, 0.0f, 0.04f);
	ImGui::SliderFloat("quadrAttenuation", &mLight.quadrAttenuation, 0.0f, 0.02f);

	ImGui::Separator();
	ImGui::Checkbox("EmitShadow", &mbEmitShadow);

	return false;
}
#endif

IComp* LightComponent::Clone()
{
	return Scene.CreateComp<LightComponent>(mOwner->GetSpace(), this);
}

const glm::mat4& LightComponent::GetWorld2Cam()
{
	if (mLight.light_type != (u32)GFX::Light::Type::POINT)
	{
		mWorld2Cam = glm::lookAt(mOwner->mTransform.mPosition, mOwner->mTransform.mPosition
			+ mLight.direction, { 0.0f, 1.0f, 0.0f });
	}
	else
	{
		mWorld2Cam = glm::mat4{1.0f};
	}
	return mWorld2Cam;
}

const glm::mat4& LightComponent::GetCam2Proj()
{
	// Point light casts shadows in all 6 planes surrounding it
	if (mLight.light_type == (u32)GFX::Light::Type::POINT) 
	{
		mCam2Proj = glm::mat4{ 1.0f };
	}

	// Directional ligths cast shadows directly (Orthognal)
	else if (mLight.light_type == (u32)GFX::Light::Type::DIRECTIONAL) 
	{
		mCam2Proj = glm::ortho(-mLight.projection_size, mLight.projection_size, 
			-mLight.projection_size, mLight.projection_size, mLight.near, mLight.far);
	}

	// Spot light, are pretty much the same as a camera, use the typical perspective mtx
	// and have a single direction
	else if (mLight.light_type == (u32)GFX::Light::Type::SPOT) 
	{
		mCam2Proj = glm::perspective(glm::radians(70.0f), 1.0f, mLight.near, mLight.far);
	}

	return mCam2Proj;
}

const GFX::Framebuffer& LightComponent::GetFramebuffer() const
{
	return mShadowMap;
}

void LightComponent::ToJson(nlohmann::json& j) const
{
	j["LightType"] << mLight.light_type;
	j["Direction"] << mLight.direction;

	j["Ambient"]  << mLight.ambient;
	j["Diffuse"]  << mLight.diffuse;
	j["Specular"] << mLight.specular;

	j["Near"]            << mLight.near;
	j["Far"]             << mLight.far;
	j["Projection Size"] << mLight.projection_size;

	j["spotExponent"]   << mLight.spotExponent;
	j["spotOuterAngle"] << mLight.spotOuterAngle;
	j["spotInnerAngle"] << mLight.spotInnerAngle;

	j["constAttenuation"]  << mLight.constAttenuation;
	j["linearAttenuation"] << mLight.linearAttenuation;
	j["quadrAttenuation"]  << mLight.quadrAttenuation;

	j["mbEmitShadow"] << mbEmitShadow;
}

void LightComponent::FromJson(nlohmann::json& j)
{
	j["LightType"] >> mLight.light_type;
	j["Direction"] >> mLight.direction;

	j["Ambient"]  >> mLight.ambient;
	j["Diffuse"]  >> mLight.diffuse;
	j["Specular"] >> mLight.specular;
	
	if (j.find("Near") != j.end()) {
		j["Near"] >> mLight.near;
		j["Far"] >> mLight.far;
		j["Projection Size"] >> mLight.projection_size;
	}

	j["spotExponent"]   >> mLight.spotExponent;
	j["spotOuterAngle"] >> mLight.spotOuterAngle;
	j["spotInnerAngle"] >> mLight.spotInnerAngle;

	j["constAttenuation"]  >> mLight.constAttenuation;
	j["linearAttenuation"] >> mLight.linearAttenuation;
	j["quadrAttenuation"]  >> mLight.quadrAttenuation;

	if (j.find("mbEmitShadow") != j.end())
		j["mbEmitShadow"] >> mbEmitShadow;
}
