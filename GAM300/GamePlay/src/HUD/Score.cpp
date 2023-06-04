#include "Score.h"
#include "Graphics/TextRendering/TextRender.h"
#include "WaveSystem/WaveSystem.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR

void HighestScore::Initialize()
{
	auto score_object = Scene.FindObject("HighestScore");
	if (score_object)
		score = score_object->GetComponentType<TextComponent>();
}

void HighestScore::Update()
{
	if (!score) return;
	//get the current bullets and bullet charger
	std::string highest_score("Highest Round : ");

	if (WaveSys.highest_round > 0)
	{
		char sc[100];
		_itoa_s(static_cast<unsigned>(WaveSys.highest_round), sc, sizeof(sc), 10);
		highest_score += sc;
	}
	else
		highest_score += "-";

	score->SetText(highest_score);
}


#ifdef EDITOR
bool HighestScore::Edit()
{
	bool changed = false;
	return changed;
}
#endif

void HighestScore::ToJson(nlohmann::json& j) const
{
}

void HighestScore::FromJson(nlohmann::json& j)
{
}

IComp* HighestScore::Clone()
{
	return Scene.CreateComp<HighestScore>(mOwner->GetSpace(), this);
}