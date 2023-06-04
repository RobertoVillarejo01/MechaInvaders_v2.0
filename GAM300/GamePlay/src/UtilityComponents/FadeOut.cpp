#include "FadeOut.h"
#include "Graphics/Renderable/Renderable.h"
#include "Utilities/FrameRateController/FrameRateController.h"
#include "Graphics/ParticleSystem/Particle.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR

void FadeOut::Initialize()
{
	render = mOwner->GetComponentType<renderable>();
	color = render->GetColor();
}

void FadeOut::Update()
{
	if (trigger || initialize_triggering)
	{
		timer += FRC.GetFrameTime();
		if (timer >= delay)
		{
			color.a -= FRC.GetFrameTime() * fade_speed;
			render->SetColor(color);
		}
	}
	if (use_particles)
	{
		particles_timer += FRC.GetFrameTime();
		if(particles_timer >= time_to_stop_particles)
			mOwner->GetComponentType<ParticleSystem>()->SetEmitRate(0.0f);
	}
}

void FadeOut::Reset()
{
	timer = 0.0f;
	color.a = 1.0f;
	trigger = true;
	render->SetColor(color);
}

#ifdef EDITOR
bool FadeOut::Edit()
{
	bool changed = false;

	ImGui::Checkbox("Trigger when Spawning", &initialize_triggering);
	ImGui::DragFloat("Delay", &delay, 0.01f, 0.0f, 50.0f);
	ImGui::DragFloat("FadeSpeed", &fade_speed, 0.01f, 0.0f, 50.0f);

	ImGui::Checkbox("Destroy Object", &destroy_object);
	ImGui::Checkbox("Use Particles", &use_particles);
	if(use_particles)
		ImGui::DragFloat("Particles Timer To Stop", &time_to_stop_particles, 0.01f, 0.0f, 100.0f);

	return changed;
}
#endif

void FadeOut::ToJson(nlohmann::json& j) const
{
	j["Delay"] << delay;
	j["FadeSpeed"] << fade_speed;
	j["initialize_triggering"] << initialize_triggering;
	j["use_particles"] << use_particles;
	j["time_to_stop_particles"] << time_to_stop_particles;
	j["destroy_object"] << destroy_object;
}

void FadeOut::FromJson(nlohmann::json& j)
{
	if (j.find("Delay") != j.end())
		j["Delay"] >> delay;
	if (j.find("FadeSpeed") != j.end())
		j["FadeSpeed"] >> fade_speed;
	if (j.find("initialize_triggering") != j.end())
		j["initialize_triggering"] >> initialize_triggering;
	if (j.find("use_particles") != j.end())
		j["use_particles"] >> use_particles;
	if (j.find("time_to_stop_particles") != j.end())
		j["time_to_stop_particles"] >> time_to_stop_particles;
	if (j.find("destroy_object") != j.end())
		j["destroy_object"] >> destroy_object;
}

IComp* FadeOut::Clone()
{
	return Scene.CreateComp<FadeOut>(mOwner->GetSpace(), this);
}