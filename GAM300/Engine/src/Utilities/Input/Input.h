#pragma once
#include <map>

#include <glm/glm.hpp>
#include <SDL2/SDL.h>

#include "InputEvent.h"
#include "Utilities/Singleton.h"

namespace glm
{
	std::ostream& operator<<(std::ostream& os, const ivec2& v);
	std::ostream& operator<<(std::ostream& os, const vec2& v);
}

static const int KEYBOARD_KEY_AMOUNT = SDL_NUM_SCANCODES;
static const int MOUSE_KEY_AMOUNT = 3;

enum class MouseKey {
	LEFT, MID, RIGHT
};

enum class Key : unsigned
{
	A = SDL_SCANCODE_A, B, C, D, E, F, G, H, I, J, K, L, M,
	N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

	Num1 = SDL_SCANCODE_1, Num2, Num3, Num4, Num5, Num6, Num7, 
	Num8, Num9, Num0,

	Tab				= SDL_SCANCODE_TAB,
	Control		= SDL_SCANCODE_LCTRL,
	LShift		= SDL_SCANCODE_LSHIFT,
	LAlt			= SDL_SCANCODE_LALT,
	Enter			= SDL_SCANCODE_RETURN,
	Delete		= SDL_SCANCODE_DELETE,
	Esc				= SDL_SCANCODE_ESCAPE,
	Space			= SDL_SCANCODE_SPACE,

	Plus			= SDL_SCANCODE_KP_PLUS,
	Minus			= SDL_SCANCODE_KP_MINUS,

	Supr			= SDL_SCANCODE_BACKSPACE,

	F1 = SDL_SCANCODE_F1,
	F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

	Right		= SDL_SCANCODE_RIGHT,
	Left		= SDL_SCANCODE_LEFT,
	Down		= SDL_SCANCODE_DOWN,
	Up			= SDL_SCANCODE_UP,
};

class InputHandler
{
	MAKE_SINGLETON(InputHandler)

public:
	bool Initialize();
	void StartFrame();

	void HandleEnvents(bool* _quit);

	bool MouseIsDown(MouseKey index);
	bool MouseIsUp(MouseKey index);
	bool MouseIsTriggered(MouseKey index);
	bool MouseIsReleased(MouseKey index);
	int  GetWheelScroll() const;

	bool KeyIsDown(Key index);
	bool KeyIsUp(Key index);
	bool KeyIsTriggered(Key index);
	bool KeyIsReleased(Key index);

	bool ButtonIsDown(unsigned index);
	bool ButtonIsUp(unsigned index);
	bool ButtonIsTriggered(unsigned index);
	bool ButtonIsReleased(unsigned index);
	bool CheckOther(unsigned button);
	bool WheelTriggered();

	const glm::ivec2& RawMousePos() const;
	const glm::vec2& WindowMousePos() const;
	void  SetMousePos(glm::vec2 pos);

	std::map<unsigned, unsigned> buttonDown;
	std::map<unsigned, unsigned> buttonUp;
	std::map<unsigned, bool> mCurrentButton;
	std::map<unsigned, bool> mPreviousButton;

private:
	void GetRawMouse();
	void HandleMouseEvent(SDL_Event event);
	void HandleKeyEvent(SDL_Event event);
	void HandleAxisEvent(SDL_Event event);
	void HandleGamePadEvent(SDL_Event gamepad_event);
	void HandleMouseWheel(SDL_Event event);
	void ResetPad(SDL_Event gamepad_event);

	/* Mouse */
	glm::ivec2 mRawMouse = {};
	glm::ivec2 mRelMouse = {};
	glm::vec2  mWindowMouse = {};
	int  mWheelScroll = 0;
	bool mMouseCurrent[MOUSE_KEY_AMOUNT] = {false};
	bool mMousePrevious[MOUSE_KEY_AMOUNT] = {false};

	/* KeyboardKeys (256) */
	bool mKeyCurrent[KEYBOARD_KEY_AMOUNT] = {false};
	bool mKeyPrevious[KEYBOARD_KEY_AMOUNT] = {false};

	bool mWheelCurrent = false;
	//Gamepad
	glm::vec2 mCurrentJoyStickValue = {};
	float offset = 0.2f;
	float lapse = 0.0f;
};

#define InputManager (InputHandler::Instance())

#define KeyDown(i)				InputManager.KeyIsDown(i)
#define KeyUp(i)					InputManager.KeyIsUp(i)
#define KeyTriggered(i)		InputManager.KeyIsTriggered(i)
#define KeyReleased(i)		InputManager.KeyIsReleased(i)

#define MouseWheel()			InputManager.WheelTriggered()
#define MouseDown(i)			InputManager.MouseIsDown(i)
#define MouseUp(i)				InputManager.MouseIsUp(i)
#define MouseTriggered(i) InputManager.MouseIsTriggered(i)
#define MouseReleased(i)	InputManager.MouseIsReleased(i)
