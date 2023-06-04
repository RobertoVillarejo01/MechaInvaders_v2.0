#include "InteractionText.h"
#include "InteractionComp.h"
#include "Player/Player.h"
#include "Graphics/TextRendering/TextRender.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR

#include "Utilities/Utils.h"

void InteractionText::Initialize()
{
	auto player = Scene.get_base_player();
	if (player)
		mPlayer = player->GetComponentType<Player>();
	mText = mOwner->GetComponentType<TextComponent>();

}

void InteractionText::Update()
{
	if (!mbInteracting && mPlayer)
	{
		glm::vec3 player_pos = mPlayer->mOwner->mTransform.mPosition;
		chosen_object = nullptr;
		float min_dist = std::numeric_limits<float>::max();

		for (auto it : objects)
		{
			float temp = glm::distance2(it.first->mOwner->mTransform.mPosition, player_pos);
			if (temp < min_dist)
			{
				min_dist = temp;
				chosen_object = it.first;
			}
		}

		if (chosen_object)
		{
			glm::vec3 direction = player_pos - chosen_object->mOwner->mTransform.mPosition;
			direction = glm::normalize(direction);
			mOwner->mTransform.mPosition = chosen_object->mOwner->mTransform.mPosition + direction * distance;
			mText->SetText(objects[chosen_object]);
			mText->SetVisible(true);
		}
		else
			mText->SetVisible(false);
	}
	else
		mText->SetVisible(false);
}

void InteractionText::Interact()
{
	if (chosen_object)
	{
		mbInteracting = true;
		chosen_object->mbInteracting = true;
	}
}

bool InteractionText::CheckInteract()
{
	if (chosen_object && !mbInteracting)
	{
		if (chosen_object->CheckInteract())
			return true;
	}
	return false;
}

void InteractionText::StopInteracting(bool keepactive)
{
	if (chosen_object && mbInteracting)
	{
		chosen_object->StopInteracting(keepactive);
		mbInteracting = false;
	}
}

void InteractionText::Shutdown()
{
	objects.clear();
}

void InteractionText::AttachObj(InteractionComp* obj, std::string str)
{
	objects[obj] = str;
}

void InteractionText::DeatachObj(InteractionComp* obj)
{
	if (obj == nullptr) return;
	if (obj == chosen_object)
	{
		mbInteracting = false;
		mPlayer->interacting = false;
	}
		
	objects.erase(obj);
}

#ifdef EDITOR
bool InteractionText::Edit()
{
	bool changed = false;

	ImGui::DragFloat("Text Distance", &distance, 0.1f);
	return changed;
}
#endif

void InteractionText::ToJson(nlohmann::json& j) const
{
	j["distance"] << distance;
}

void InteractionText::FromJson(nlohmann::json& j)
{
	if (j.find("distance") != j.end())
		j["distance"] >> distance;
}

IComp* InteractionText::Clone()
{
	return Scene.CreateComp<InteractionText>(mOwner->GetSpace(), this);
}