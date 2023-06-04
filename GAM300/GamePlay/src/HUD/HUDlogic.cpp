#include "HUDlogic.h"
#include "Graphics/TextRendering/TextRender.h"
#include "Player/Player.h"
#include "Weapon/Weapon.h"
#include "WaveSystem/WaveSystem.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR

void HUDlogic::Initialize()
{
	auto player = Scene.get_base_player();
	if (player)
	{
		mPlayer = player->GetComponentType<Player>();
		auto weapon = mPlayer->mOwner->FindChild("Pistol");
		if (weapon) mWeapon = weapon->GetComponentType<Weapon>();
	}

	auto bullet = mOwner->GetSpace()->FindObject("BulletsCount");
	if(bullet) BulletsCount = bullet->GetComponentType<TextComponent>();

	auto points = mOwner->GetSpace()->FindObject("CurrentPoints");
	if (points) CurrentPoints = points->GetComponentType<TextComponent>();

	auto rounds = mOwner->GetSpace()->FindObject("RoundsCount");
	if (rounds) RoundsCount = rounds->GetComponentType<TextComponent>();
}

void HUDlogic::Update()
{
	if (!mWeapon || !mPlayer) return;

	//get the current bullets and bullet charger
	std::string bullets;

	char bullet_count[100];
	_itoa_s(static_cast<unsigned>(mPlayer->mWeaponComp->bullet_count), bullet_count, sizeof(bullet_count), 10);
	bullets += bullet_count;
	bullets += "/";
	char total_bullets[100] = "-";
	if (!mPlayer->mWeaponComp->infinity_ammo)
		_itoa_s(static_cast<unsigned>(mPlayer->mWeaponComp->total_bullets), total_bullets, sizeof(total_bullets), 10);
	bullets += total_bullets;
	BulletsCount->SetText(bullets);

	//set the current points
	std::string points;
	char money[100];
	_itoa_s(static_cast<unsigned>(mPlayer->money), money, sizeof(money), 10);
	points += money;
	points += " $";
	CurrentPoints->SetText(points);

	//set the round count
	std::string rounds("Round : ");
	char round[100];
	_itoa_s(static_cast<unsigned>(WaveSys.round_num), round, sizeof(round), 10);
	rounds += round;
	RoundsCount->SetText(rounds);
}


#ifdef EDITOR
bool HUDlogic::Edit()
{
	bool changed = false;
	return changed;
}
#endif

void HUDlogic::ToJson(nlohmann::json& j) const
{
}

void HUDlogic::FromJson(nlohmann::json& j)
{
}

IComp* HUDlogic::Clone()
{
	return Scene.CreateComp<HUDlogic>(mOwner->GetSpace(), this);
}