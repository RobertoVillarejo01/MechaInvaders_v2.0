#include "HUDBar.h"
#include "Graphics/Renderable/Renderable.h"
#include "Utilities/FrameRateController/FrameRateController.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR

void HUDBar::Initialize()
{
	render = mOwner->GetComponentType<renderable>();
	InitialPos = mOwner->mTransform.mPosition - (mOwner->mTransform.mScale);
	InitialScale = mOwner->mTransform.mScale;
	render->SetVisible(false);
}

void HUDBar::Update()
{

}

void HUDBar::SetBar(float percentage)
{
	render->SetVisible(true);
	current_var_val = 100.0f * percentage;

	if (percentage < 0.0f)
		percentage = 0;

	if (percentage > 1.0f)
		percentage = 1.0f;

	mOwner->mTransform.mScale.x = percentage * InitialScale.x;
	float offset = mOwner->mTransform.mScale.x;
	mOwner->mTransform.mPosition.x = InitialPos.x + offset;
}

void HUDBar::Reset()
{
	current_var_val = 0.0f;
	SetBar(current_var_val);
	render->SetVisible(false);
}

#ifdef EDITOR
bool HUDBar::Edit()
{
	bool changed = false;
	return changed;
}
#endif

void HUDBar::ToJson(nlohmann::json& j) const
{
}

void HUDBar::FromJson(nlohmann::json& j)
{
}

IComp* HUDBar::Clone()
{
	return Scene.CreateComp<HUDBar>(mOwner->GetSpace(), this);
}