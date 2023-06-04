#include "MenuManager.h"
#include "Utilities/Input/Input.h"

#include "Window/Window.h"
#include "Utilities/Input/Input.h"
#include "Graphics/RenderManager/RenderManager.h"
#include "Graphics/TextRendering/TextRender.h"
#include "Graphics/DebugDraw/DebugDrawing.h"
#include "Graphics/Camera/Camera.h"
#include "System/Scene/SceneSystem.h"
#include "GameStateManager/GameStateManager.h"
#include "AudioManager/Audio.h"
#include "Networking/Networking.h"
#include "../GamePlay/src/UtilityComponents/FadeInOut.h"

#include "../Editor/src/BasicFunctionalities/ObjectSelection/MousePicker/MousePicker.h"

#undef min
#undef max
#undef GetMessage


void ShowMenu(std::string const& _name)
{
	/// I KNOW; THIS IS HORRIBLE; IN MY DEFENSE, IT ONLY HAPPENS RARELY AND IT IS NOT EVENT THAT BAD
	if (_name != "HowToPlay")
	{
		auto how = Scene.FindObject("HowToPlay", Scene.GetSpace("MainArea"));
		if (how) how->GetComponentType<renderable>()->SetVisible(false);
	}

	auto& menus = Scene.GetComponentsType<IRenderable>(Scene.GetSpace("MainArea"));
	for (auto& m : menus) {
		if (dynamic_cast<MenuComp*>(m) && m->mOwner->GetName() == _name) {
			MenuMgr.GetActiveMenu()->SetVisible(false);
			MenuMgr.SetActiveMenu(dynamic_cast<MenuComp*>(m));
			MenuMgr.GetActiveMenu()->SetVisible(true);
			MenuMgr.GetActiveMenu()->Reset();
			return;
		}
	}
	auto& menus2 = Scene.GetComponentsType<IRenderable>(Scene.GetSpace("HUD"));
	for (auto& m : menus2) {
		if (dynamic_cast<MenuComp*>(m) && m->mOwner->GetName() == _name) {
			MenuMgr.GetActiveMenu()->SetVisible(false);
			MenuMgr.SetActiveMenu(dynamic_cast<MenuComp*>(m));
			MenuMgr.GetActiveMenu()->SetVisible(true);
			MenuMgr.GetActiveMenu()->Reset();
			return;
		}
	}
}

void RunGameSinglePlayer() {
	NetworkingMrg.InitializeAsSinglePlayer();
	Scene.FindObject("FaderMainMenu")->GetComponentType<FadeInOut>()->trigger = true;
	MenuMgr.SetInMenu(false);
	SDL_ShowCursor(SDL_FALSE);
	SDL_SetRelativeMouseMode(SDL_TRUE);
}
void RunGameSinglePlayerFirstPlayable() {
	NetworkingMrg.InitializeAsSinglePlayer();
	Scene.FindObject("FaderMainMenu")->GetComponentType<FadeInOut>()->trigger = true;
	MenuMgr.SetInMenu(false);
	SDL_ShowCursor(SDL_FALSE);
	SDL_SetRelativeMouseMode(SDL_TRUE);
}
void RunGameMultiplePlayerServer() {
	auto fade = Scene.FindObject("FaderMainMenu")->GetComponentType<FadeInOut>();
	fade->SetNextLevel("LobbyLevel");
	fade->trigger = true;

	NetworkingMrg.InitializeAsServer();
	MenuMgr.SetInMenu(false);
	SDL_ShowCursor(SDL_FALSE);
	SDL_SetRelativeMouseMode(SDL_TRUE);
}
void RunGameMultiplePlayerClient() {
	auto fade = Scene.FindObject("FaderMainMenu")->GetComponentType<FadeInOut>();
	fade->SetNextLevel("LobbyLevel");
	fade->trigger = true;

	NetworkingMrg.InitializeAsClient();
	MenuMgr.SetInMenu(false);
	SDL_ShowCursor(SDL_FALSE);
	SDL_SetRelativeMouseMode(SDL_TRUE);
}
void RunGameSinglePlayerPlaytesting() {
	NetworkingMrg.InitializeAsSinglePlayer();
	GSM.SetNextLevel("Playtest");
	MenuMgr.SetInMenu(false);
	SDL_ShowCursor(SDL_FALSE);
	SDL_SetRelativeMouseMode(SDL_TRUE);
}

void CloseApp() {
	GSM.TerminateExecution();
}
void CloseMenu() {
	MenuMgr.SetInMenu(false);
	SDL_ShowCursor(SDL_FALSE);
	SDL_SetRelativeMouseMode(SDL_TRUE);
}


void GoToGfxMenu() {
	ShowMenu("Graphics");
}
void GoToAudioMenu() {
	ShowMenu("Audio");
}
void GoToLanguageMenu() {
	ShowMenu("Language");
}
void GoToMainMenu() {
	auto fade = Scene.FindObject("GameOverFader");
	if (fade)
	{
		auto f_comp = fade->GetComponentType<FadeInOut>();
		f_comp->mOutTime = 1.5f;
	//	f_comp->SetDelay(2.0f);
	//	f_comp->SetFadeSpeed(1.5f);
		f_comp->trigger = true;
	}

	ShowMenu("MainMenu");
}
void GoToSettings() {
	ShowMenu("Settings");
}
void GoToControls() {
	ShowMenu("Controls");
}
void GoToHowToPlay() {

	renderable* rend = Scene.FindObject("HowToPlay", Scene.GetSpace("MainArea"))->GetComponentType<renderable>();
	rend->SetVisible(true);
	ShowMenu("HowToPlay");
}

void SetFullscreen() {
	WindowMgr.ToggleFullScreen();
}
void ChangeResolution960() {
	WindowMgr.ChangeResolution({ 960,  540 });
}
void ChangeResolution1280() {
	WindowMgr.ChangeResolution({ 1280,  720 });
}
void ChangeResolution1600() {
	WindowMgr.ChangeResolution({ 1600,  900 });
}
void ChangeResolution1920() {
	WindowMgr.ChangeResolution({ 1920,  1080 });
}

void ChangeToEnglish() {
	TranslationMgr.ChangeLanguage(TranslationManager::Languages::ENGLISH);
}
void ChangeToSpanish() {
	TranslationMgr.ChangeLanguage(TranslationManager::Languages::SPANISH);
}




bool MenuManager::InMenu() const
{
	return mInMenu;
}
void MenuManager::SetInMenu(bool set)
{
	mInMenu = set;
	if (!mInMenu)
	{
		if (mActiveMenu)
			mActiveMenu->SetVisible(false);
	}
	else
	{
		if (mActiveMenu)
			mActiveMenu->SetVisible(true);
	}
}

void MenuManager::Initialize()
{
	// Avoid duplicates
	mFunctions.clear();


	// Register functions

	// Main options
	mFunctions["SinglePlayer"] = &RunGameSinglePlayer;
	mFunctions["SinglePlayer(FirstPlayable)"] = &RunGameSinglePlayerFirstPlayable;
	mFunctions["SinglePlayer(Playtest)"] = &RunGameSinglePlayerPlaytesting;

	mFunctions["MultiPlayer(Server)"] = &RunGameMultiplePlayerServer;
	mFunctions["MultiPlayer(Client)"] = &RunGameMultiplePlayerClient;
	mFunctions["Settings"] = &GoToSettings;
	mFunctions["CloseApp"] = &CloseApp;

	mFunctions["GraphicsSettings"] = &GoToGfxMenu;
	mFunctions["AudioSettings"] = &GoToAudioMenu;
	mFunctions["LanguageSettings"] = &GoToLanguageMenu;
	mFunctions["ControlSettings"] = &GoToControls;

	mFunctions["GoBackToMain"] = &GoToMainMenu;
	mFunctions["CloseMenu"] = &CloseMenu;

	// GFX Options
	mCheckboxes["FullscreenBool"] = &WindowMgr.mbFullscreen;
	mFunctions["Fullscreen"] = &SetFullscreen;
	mFunctions["Resolution960x540"] = &ChangeResolution960;
	mFunctions["Resolution1280x720"] = &ChangeResolution1280;
	mFunctions["Resolution1600x900"] = &ChangeResolution1600;
	mFunctions["Resolution1920x1080"] = &ChangeResolution1920;

	// Audio Options
	mSliders["SFXVolume"] = &gAudioMgr.SFXVolume;
	mSliders["MusicVolume"] = &gAudioMgr.MusicVolume;
	mSliders["Sensitivity"] = &GSM.mConfig.mSensitivity;

	// Language Options
	mFunctions["English"] = &ChangeToEnglish;
	mFunctions["Spanish"] = &ChangeToSpanish;

	//How to play 
	mFunctions["HowToPlay"] = &GoToHowToPlay;

	// Close confirmation

}
void MenuManager::CheckInMenu()
{
#ifdef EDITOR
	if (InputManager.KeyIsTriggered(Key::P))
#else
	if (InputManager.KeyIsTriggered(Key::P) || InputManager.KeyIsTriggered(Key::Esc))
#endif
	{
		if (mInLobby)
		{
			mInMenu = !mInMenu;
			SetInMenu(mInMenu);

			if (!mInMenu) {
				SDL_ShowCursor(SDL_FALSE);
				SDL_SetRelativeMouseMode(SDL_TRUE);
			}
			else {
				SDL_ShowCursor(SDL_TRUE);
				SDL_SetRelativeMouseMode(SDL_FALSE);
			}
		}
		else if (NetworkingMrg.AmIServer())
		{
			mInMenu = !mInMenu;
			SetInMenu(mInMenu);

			if (!mInMenu) {
				SDL_ShowCursor(SDL_FALSE);
				SDL_SetRelativeMouseMode(SDL_TRUE);
			}
			else {
				SDL_ShowCursor(SDL_TRUE);
				SDL_SetRelativeMouseMode(SDL_FALSE);
			}
		}

	}
}
void MenuManager::Update()
{
	SDL_ShowCursor(SDL_TRUE);
	SDL_SetRelativeMouseMode(SDL_FALSE);
	if (mActiveMenu)
		mActiveMenu->Update();
}



bool MenuComp::mLevelShouldStartPaused = false;

void MenuComp::Slider::React(float t)
{
	// Mouse
	if (mProgress && MouseTriggered(MouseKey::LEFT)) {
		*mProgress = t;
		*mProgress = glm::max(glm::min(*mProgress, 1.0f), 0.0f);
	}

	// Keyboard
	if (mProgress && KeyDown(Key::Right)) {
		*mProgress += 0.01f;
		*mProgress = glm::min(*mProgress, 1.0f);
	}
	if (mProgress && KeyDown(Key::Left)) {
		*mProgress -= 0.01f;
		*mProgress = glm::max(*mProgress, 0.0f);
	}
}
#ifdef EDITOR
void MenuComp::Slider::Edit()
{
	if (ImGui::BeginCombo("FloatPtr", mItemName.c_str())) {

		for (auto& f : MenuMgr.mSliders) {
			if (ImGui::Selectable(f.first.data())) {
				mItemName = f.first;
				mProgress = f.second;
			}
		}

		ImGui::EndCombo();
	}
}
#endif
void MenuComp::Slider::Render(Transform const& tr)
{
	// Basic menu
	auto TotalTransform = tr;
	TotalTransform.mPosition = tr.mPosition + glm::vec3{ mOffset.x *
		tr.mScale.x, mOffset.y * tr.mScale.y, tr.mScale.z };
	TotalTransform.mScale = { tr.mScale.x * mScale.x, tr.mScale.y * mScale.y,
		tr.mScale.z };

	RenderMgr.RenderModel(&GFX::Model::Quad, glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f },
		TotalTransform.ModelToWorld(), RenderMgr.GetPolygonMode());


	// Slider
	if (!mProgress) return;

	TotalTransform = tr;
	TotalTransform.mPosition = tr.mPosition +
		glm::vec3{ mOffset.x * tr.mScale.x, mOffset.y * tr.mScale.y, tr.mScale.z } -
		glm::vec3{ tr.mScale.x * mScale.x, 0, 0 }
	+ glm::vec3{ tr.mScale.x * mScale.x * 2.0f * *mProgress, 0, 0.1 };
	TotalTransform.mScale = { tr.mScale.x * mScale.x / 30.0f, tr.mScale.y * mScale.y * 1.1f,
		tr.mScale.z };

	RenderMgr.RenderModel(&GFX::Model::Quad, glm::vec4{ 0.6f, 0.6f, 0.6f, 1.0f },
		TotalTransform.ModelToWorld(), RenderMgr.GetPolygonMode());

	// Text
	RenderMgr.RenderText(nullptr, glm::vec3{ mScale.y * 0.01f },
		glm::vec3{ mOffset.x - mScale.x + mScale.y * 0.5f, mOffset.y - mScale.y * 0.25f, 0.2f },
		glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }, tr, false,
		false, TextComponent::Alignment::Left, TranslationMgr.GetMessage(mTextIdx));
}

void MenuComp::CheckBox::React(float t)
{
	// Sanity check
	if (!mStatePtr) return;

	// Keyboard & Mouse (assuming mActivate change the value of mStatePtr)
	if (mbUseFnPtrs) {
		if (MouseTriggered(MouseKey::LEFT) || KeyTriggered(Key::Space) || KeyTriggered(Key::Enter)) {
			if (!*mStatePtr && mActivate)	mActivate();
			else if (*mStatePtr && mDeactivate) mDeactivate();
		}
	}
	else {
		*mStatePtr = !*mStatePtr;
	}
}
#ifdef EDITOR
void MenuComp::CheckBox::Edit()
{
	if (ImGui::BeginCombo("BoolPtr", mItemName.c_str())) {

		for (auto& f : MenuMgr.mCheckboxes) {
			if (ImGui::Selectable(f.first.data())) {
				mItemName = f.first;
				mStatePtr = f.second;
			}
		}
		ImGui::EndCombo();
	}

	ImGui::Checkbox("Use Activate/Deactivate functions", &mbUseFnPtrs);
	if (mbUseFnPtrs) {
		if (ImGui::BeginCombo("Activate", mActivateName.c_str())) {

			for (auto& f : MenuMgr.mFunctions) {
				if (ImGui::Selectable(f.first.data())) {
					mActivateName = f.first;
					mActivate = f.second;
				}
			}
			ImGui::EndCombo();
		}
		if (ImGui::BeginCombo("Deactivate", mDeactivateName.c_str())) {

			for (auto& f : MenuMgr.mFunctions) {
				if (ImGui::Selectable(f.first.data())) {
					mDeactivateName = f.first;
					mDeactivate = f.second;
				}
			}
			ImGui::EndCombo();
		}
	}
}
#endif
void MenuComp::CheckBox::Render(Transform const& tr)
{
	// Basic menu
	auto TotalTransform = tr;
	TotalTransform.mPosition = tr.mPosition + glm::vec3{ mOffset.x *
		tr.mScale.x, mOffset.y * tr.mScale.y, tr.mScale.z };
	TotalTransform.mScale = { tr.mScale.x * mScale.x, tr.mScale.y * mScale.y,
		tr.mScale.z };

	RenderMgr.RenderModel(&GFX::Model::Quad, glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f },
		TotalTransform.ModelToWorld(), RenderMgr.GetPolygonMode());

	// Check box part
	if (!mStatePtr) return;

	TotalTransform = tr;
	TotalTransform.mPosition = tr.mPosition +
		glm::vec3{ mOffset.x * tr.mScale.x + mScale.x * tr.mScale.x
		- mScale.y * tr.mScale.y, mOffset.y * tr.mScale.y, tr.mScale.z + 1.2f };
	TotalTransform.mScale = { mScale.y * tr.mScale.y / 2.0f, mScale.y * tr.mScale.y / 2.0f,
		tr.mScale.z };

	auto color = *mStatePtr ? glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f }
	: glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f };
	RenderMgr.RenderModel(&GFX::Model::Quad, color,
		TotalTransform.ModelToWorld(), RenderMgr.GetPolygonMode());

	// Text
	RenderMgr.RenderText(nullptr, glm::vec3{ mScale.y * 0.01f },
		glm::vec3{ mOffset.x - mScale.x + mScale.y * 0.5f, mOffset.y - mScale.y * 0.25f, 0.1f },
		glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }, tr, false,
		false, TextComponent::Alignment::Left, TranslationMgr.GetMessage(mTextIdx));
}

void MenuComp::Button::React(float t)
{
	// Mouse
	if (MouseTriggered(MouseKey::LEFT))
		mFunction();

	// Keyboard
	if (mFunction && (KeyTriggered(Key::Space) || KeyTriggered(Key::Enter))) {
		mFunction();
	}
}
#ifdef EDITOR
void MenuComp::Button::Edit()
{
	if (ImGui::BeginCombo("Callback", mItemName.c_str())) {

		for (auto& f : MenuMgr.mFunctions) {
			if (ImGui::Selectable(f.first.data())) {
				mItemName = f.first;
				mFunction = f.second;
			}
		}
		ImGui::EndCombo();
	}
}
#endif
void MenuComp::Button::Render(Transform const& tr)
{
	// Basic box
	auto TotalTransform = tr;
	TotalTransform.mPosition = tr.mPosition + glm::vec3{ mOffset.x *
		tr.mScale.x, mOffset.y * tr.mScale.y, tr.mScale.z };
	TotalTransform.mScale = { tr.mScale.x * mScale.x, tr.mScale.y * mScale.y,
		tr.mScale.z };

	RenderMgr.RenderModel(&GFX::Model::Quad, glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f },
		TotalTransform.ModelToWorld(), RenderMgr.GetPolygonMode());

	// Text
	RenderMgr.RenderText(nullptr, glm::vec3{ mScale.y * 0.01f },
		glm::vec3{ mOffset.x - mScale.x + mScale.y * 0.5f, mOffset.y - mScale.y * 0.25f, 0.1f },
		glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }, tr, false,
		false, TextComponent::Alignment::Left, TranslationMgr.GetMessage(mTextIdx));
}

void MenuComp::Initialize()
{
	auto obj = Scene.FindObject("MenuEmitter");
	emitter = obj->GetComponentType<SoundEmitter>();

	//#serdada, sorry
	auto fader_main_menu = Scene.FindObject("FaderMainMenu");
	if (fader_main_menu) fader_main_menu->GetComponentType<FadeInOut>()->trigger = false;
}

void MenuComp::Update()
{
	// Checks with mouse
	auto trPos = mOwner->mTransform.mPosition;

	// Find the camera
	auto camera = Scene.GetSpace("HUD")->FindObject("Camera");
	if (!camera) {
		return;
	}

	MousePicker m{ dynamic_cast<GFX::Camera*>(camera->GetComponentType<CameraComp>()) };
	geometry::ray r;
	glm::vec2 mousePos = InputManager.RawMousePos();
	r.mDirection = glm::vec3(m.ScreenToWorld(mousePos));
	r.mOrigin = camera->mTransform.mPosition;

	int mTargetBox = -1;
	unsigned i = 0u;
	for (auto& it : mBoxes) {

		auto& BOX = it;
		auto& TR = mOwner->mTransform;
		auto TotalTransform = TR;
		TotalTransform.mPosition = TR.mPosition + glm::vec3{ it->mOffset.x *
			TR.mScale.x, it->mOffset.y * TR.mScale.y, TR.mScale.z };
		TotalTransform.mScale = { TR.mScale.x * it->mScale.x, TR.mScale.y * it->mScale.y,
			TR.mScale.z };

		geometry::aabb colliderBox;
		colliderBox.max = TotalTransform.mPosition + TotalTransform.mScale;
		colliderBox.min = TotalTransform.mPosition - TotalTransform.mScale;

		auto time = geometry::intersection_ray_aabb(r, colliderBox);
		if (time > 0) {

					// Compute, for the slider button, the position of the bar
			auto IntersectionPos = r.mOrigin + r.mDirection * time;
			auto t = (IntersectionPos.x - colliderBox.min.x) / (colliderBox.max.x - colliderBox.min.x);

			if (mCurrBox != i) {
				mCurrBox = i;
				if (emitter)
					emitter->PlayCue("./../Resources/Audio/Button_VendingMachine1.wav", 0.45f, false, false, false);
			}
			// Cast ray and check agains all same as in Editor
			if (MouseTriggered(MouseKey::LEFT))
				it->React(t);
			break;
		}

		i++;
	}
	// Checks with keyboard
	if (KeyTriggered(Key::Up)) {
		if (emitter)
			emitter->PlayCue("./../Resources/Audio/Button_VendingMachine1.wav", 0.45f, false, false, false);
		mCurrBox = (mCurrBox + mBoxes.size() - 1u) % (unsigned)mBoxes.size();
		return;
	}
	if (KeyTriggered(Key::Down)) {
		if (emitter) emitter->PlayCue("./../Resources/Audio/Button_VendingMachine1.wav", 0.45f, false, false, false);
		mCurrBox = (mCurrBox + 1u) % (unsigned)mBoxes.size();
		return;
	}

	if (KeyDown(Key::Left) || KeyDown(Key::Right) ||
		KeyTriggered(Key::Space) || KeyTriggered(Key::Enter))
	{
		if (emitter) emitter->PlayCue("./../Resources/Audio/Menu_Scifi_Enter01.wav", 0.45f, false, false, false);
		// Sanity check
		if (mCurrBox >= mBoxes.size()) {
			std::cerr << "mCurrBox out of range" << std::endl;
			return;
		}

		mBoxes[mCurrBox]->React();
	}
}
void MenuComp::Render()
{
	// Make sure this menu should be rendered
	if (!mbVisible) return;

	// Which box is currently active
	if (mCurrBox >= 0 && mCurrBox < mBoxes.size()) {

		auto BOX = mBoxes[mCurrBox];
		auto TR = mOwner->mTransform;
		auto color = glm::vec4{ 0.5f, 0.5f, 0.5f, 1.0f };

		TR.mPosition = TR.mPosition + glm::vec3{
			BOX->mOffset.x * TR.mScale.x - BOX->mScale.x * TR.mScale.x - BOX->mScale.y * TR.mScale.y * 0.0f,
			BOX->mOffset.y * TR.mScale.y, 1.2f };
		TR.mScale = {
			BOX->mScale.y * TR.mScale.y / 4.0f,
			BOX->mScale.y * TR.mScale.y / 4.0f,
			TR.mScale.z };

		RenderMgr.RenderModel(&GFX::Model::Quad, color,
			TR.ModelToWorld(), RenderMgr.GetPolygonMode());
	}

	// Render each of the other individual boxes
	for (auto& box : mBoxes) {
		box->Render(mOwner->mTransform);
	}
}
void MenuComp::Shutdown()
{
	//	for (auto& b : mBoxes)
	//	{
	//		delete b;
	//	}
	mBoxes.clear();
}
#ifdef EDITOR
bool MenuComp::Edit()
{
	IRenderable::Edit();
	ImGui::Separator();
	for (unsigned i = 0; i < mBoxes.size(); ++i)
	{
		auto b = mBoxes[i];

		ImGui::PushID(b);
		if (ImGui::TreeNode(TranslationMgr.GetMessage(b->mTextIdx).c_str()))
		{
			ImGui::DragFloat3("Offset", &b->mOffset[0], 0.0005f);
			ImGui::DragFloat2("Scale", &b->mScale[0], 0.005f);

			TranslationMgr.PickText(&b->mTextIdx);

			ImGui::Separator();
			b->Edit();
			ImGui::TreePop();
		}
		else {
			ImGui::SameLine(ImGui::GetWindowWidth() - 90);
			if (ImGui::Button("-") && i > 0) {
				std::swap(mBoxes[i], mBoxes[i - 1]);
			}
			ImGui::SameLine();
			if (ImGui::Button("+") && i < mBoxes.size() - 1) {
				std::swap(mBoxes[i], mBoxes[i + 1]);
			}
			ImGui::SameLine();
			if (ImGui::Button(" ")) {
				delete mBoxes[i];
				mBoxes.erase(mBoxes.begin() + i);
				ImGui::PopID();
				break;
			}
		}
		ImGui::PopID();
	}

	ImGui::Separator();
	if (ImGui::Selectable("Create Checkbox"))
		mBoxes.push_back(new CheckBox);
	if (ImGui::Selectable("Create Slider"))
		mBoxes.push_back(new Slider);
	if (ImGui::Selectable("Create FunctionCall"))
		mBoxes.push_back(new Button);

	ImGui::Separator();
	ImGui::Checkbox("FirstMenuOfLevel", &mFirstMenu);
	ImGui::Checkbox("LevelShouldStartPaused", &mLevelShouldStartPaused);

	return false;
}
#endif
void MenuComp::ToJson(nlohmann::json& j) const
{
	IRenderable::ToJson(j);

	j["mFirstMenu"] << mFirstMenu;
	j["mStartPaused"] << mLevelShouldStartPaused;

	for (auto& b : mBoxes) {
		nlohmann::json k;

		k["mItemName"] << b->mItemName;
		k["mTextIdx"] << b->mTextIdx;
		k["mOffset"] << b->mOffset;
		k["mScale"] << b->mScale;

		if (auto casted = dynamic_cast<CheckBox*>(b)) {
			k["mType"] = std::string("Checkbox");
			k["mbUseFnPtrs"] << casted->mbUseFnPtrs;
			if (casted->mbUseFnPtrs) {
				k["mActivateName"] << casted->mActivateName;
				k["mDeactivateName"] << casted->mDeactivateName;
			}
		}
		else if (dynamic_cast<Slider*>(b))
			k["mType"] = std::string("Slider");
		else
			k["mType"] = std::string("Button");

		j["Boxes"].push_back(k);
	}
}
void MenuComp::FromJson(nlohmann::json& j)
{
	IRenderable::FromJson(j);

	mCurrBox = 0;

	// Set the default menu to work on
	if (j.find("mFirstMenu") != j.end())
	{
		j["mFirstMenu"] >> mFirstMenu;
		if (mFirstMenu)
			MenuMgr.SetActiveMenu(this);

		// Set if the level should start paused, this variable may be set multpile times but 
		// all instances in each level should hold the same value
		j["mStartPaused"] >> mLevelShouldStartPaused;
		MenuMgr.SetInMenu(mLevelShouldStartPaused);
	}

	// The rest of boxes
	if (j.find("Boxes") != j.end()) {
		for (auto it = j["Boxes"].begin(); it != j["Boxes"].end(); ++it) {


			std::string itemName;
			(*it)["mItemName"] >> itemName;

			Box* nBox = nullptr;
			if ((*it)["mType"] == "Checkbox")
			{
				auto nCheck = new CheckBox;

				(*it)["mbUseFnPtrs"] >> nCheck->mbUseFnPtrs;
				if (nCheck->mbUseFnPtrs) {

					// Get the activate / deactivate functions
					(*it)["mActivateName"] >> nCheck->mActivateName;
					if (MenuMgr.mFunctions.find(nCheck->mActivateName) != MenuMgr.mFunctions.end())
						nCheck->mActivate = MenuMgr.mFunctions.at(nCheck->mActivateName);

					(*it)["mDeactivateName"] >> nCheck->mDeactivateName;
					if (MenuMgr.mFunctions.find(nCheck->mDeactivateName) != MenuMgr.mFunctions.end())
						nCheck->mDeactivate = MenuMgr.mFunctions.at(nCheck->mDeactivateName);
				}

				if (MenuMgr.mCheckboxes.find(itemName) != MenuMgr.mCheckboxes.end())
					nCheck->mStatePtr = MenuMgr.mCheckboxes.at(itemName);
				nBox = nCheck;
			}
			else if ((*it)["mType"] == "Slider")
			{
				auto nCheck = new Slider;
				if (MenuMgr.mSliders.find(itemName) != MenuMgr.mSliders.end())
					nCheck->mProgress = MenuMgr.mSliders.at(itemName);
				nBox = nCheck;
			}
			else
			{
				auto nCheck = new Button;
				if (MenuMgr.mFunctions.find(itemName) != MenuMgr.mFunctions.end())
					nCheck->mFunction = MenuMgr.mFunctions.at(itemName);
				nBox = nCheck;
			}

			// The general properties of the boxes
			nBox->mItemName = itemName;
			(*it)["mTextIdx"] >> nBox->mTextIdx;
			(*it)["mOffset"] >> nBox->mOffset;
			(*it)["mScale"] >> nBox->mScale;

			mBoxes.push_back(nBox);
		}
	}

}
void MenuComp::Reset()
{
	mCurrBox = 0;
}
IComp* MenuComp::Clone()
{
	return Scene.CreateComp<MenuComp>(mOwner->GetSpace(), this);
}
