#include "FadeInOut.h"
#include "Graphics/Renderable/Renderable.h"
#include "Utilities/FrameRateController/FrameRateController.h"
#include "Serializer/Factory.h"
#include "Utilities/Input/Input.h"
#include "GameStateManager/GameStateManager.h"
#include "Networking\Networking.h"
#include "GameStateManager\MenuManager\MenuManager.h"
#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
std::vector<std::string> levels;
#endif // EDITOR


void FadeInOut::Initialize()
{
	render = mOwner->GetComponentType<renderable>();
	color = render->GetColor();
	timer = just_out ? 0.0F : mInTime;//based on if we just want the fade out initialize the value differently
#ifdef EDITOR
	serializer.GetLevels(levels);
#endif // EDITOR
}

void FadeInOut::Update()
{
	if (trigger)
	{
		if (transition && !just_out)//check to guarro por parte de zapata para evitar que en el menu salte directo
		{
			//if any enter or click skip the screen
			if (InputManager.KeyIsTriggered(Key::Enter) || InputManager.MouseIsDown(MouseKey::LEFT))
			{
				GSM.SetNextLevel(next_level);
				if (!MenuMgr.InLobby())
					NetworkingMrg.ShutDownMenu();
			}
		}

		//if we want both fades
		if (!just_out)
		{
			//if the fade in has not been completed and the timer is not 0
			if (!faded_in && timer > 0.0F)
			{
				//fade in
				timer -= FRC.GetFrameTime();//decrease the counter
				color.a = inMenu ? timer / mInTime : ((mInTime - timer) / mInTime);//get the new alpha value
				render->SetColor(color);//set the new color
			}
			else if (!faded_in) { faded_in = true; timer = 0.0F; }//once completed set as faded in and the timer to 0

			if (faded_in && !displayed && timer < mDisplayTime) timer += FRC.GetFrameTime();//if has faded in, the displayed time has not been completed increase the counter
			else if (faded_in && !displayed) { timer = 0.0F; displayed = true; }//if the display time has been completed reset the timer and set the flag to true
					
		}
		else { faded_in = true; displayed = true; }//if we just want the fade out the flags are set to true

		//fade out
		if (faded_in && displayed && timer < mOutTime)//if it has completed the fade in and the display time and the fade out time is not completed
		{
			timer += FRC.GetFrameTime();//increase the timer
			color.a = inMenu ? timer / mOutTime : ((mOutTime - timer) / mOutTime);//compute the new alpha value
			render->SetColor(color);//setting the new color
		}
		else if(faded_in && displayed) faded_out = true;//setting the flag to true once that the timer ha been completed

		if (transition && faded_in && displayed && faded_out)//once all the fades have been done move to next level
		{
			GSM.SetNextLevel(next_level);
			if (!MenuMgr.InLobby())
				NetworkingMrg.ShutDownMenu();
		}
	}
}

void FadeInOut::Reset()
{
	timer = 0.0f;
	color.a = 1.0f;
	trigger = true;
	render->SetColor(color);
}

#ifdef EDITOR
bool FadeInOut::Edit()
{
	bool changed = false;

	ImGui::DragFloat("Time In Display", &mDisplayTime, 0.01f, 0.0f, 50.0f);
	ImGui::DragFloat("Time fade in", &mInTime, 0.01f, 0.0f, 50.0f);
	ImGui::DragFloat("Time fade out", &mOutTime, 0.01f, 0.0f, 50.0f);
	ImGui::Checkbox("Transition", &transition);
	ImGui::Checkbox("InMenu", &inMenu);
	if (transition)
	{
		ImGui::Checkbox("Just Fade Out", &just_out);
		if (ImGui::BeginCombo("Level to load", next_level.c_str()))
		{
			for (auto level : levels)
			{
				if (ImGui::Selectable(level.c_str(), next_level == level))
					next_level = level;
			}
			ImGui::EndCombo();
		}

	}

	return changed;
}
#endif

void FadeInOut::ToJson(nlohmann::json& j) const
{
	j["Display"] << mDisplayTime;
	j["In"] << mInTime;
	j["Out"] << mOutTime;
	j["transition"] << transition;
	j["JustFadeOut"] << just_out;
	j["InMenu"] << inMenu;

	if (transition)
		j["Next level"] << next_level;
}

void FadeInOut::FromJson(nlohmann::json& j)
{
	if (j.find("Display") != j.end())
		j["Display"] >> mDisplayTime;
	if (j.find("In") != j.end())
		j["In"] >> mInTime;
	if (j.find("Out") != j.end())
		j["Out"] >> mOutTime;
	if (j.find("transition") != j.end())
		j["transition"] >> transition;
	if (j.find("JustFadeOut") != j.end())
		j["JustFadeOut"] >> just_out;
	if (j.find("InMenu") != j.end())
		j["InMenu"] >> inMenu;
	if(transition)
		if (j.find("Next level") != j.end())
			j["Next level"] >> next_level;
	trigger = transition;
}

IComp* FadeInOut::Clone()
{
	return Scene.CreateComp<FadeInOut>(mOwner->GetSpace(), this);
}